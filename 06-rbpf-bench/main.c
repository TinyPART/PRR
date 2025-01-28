/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2024)                */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the source code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

#include <assert.h>
#include <inttypes.h>
#include <stdalign.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>

#include "rbpf.h"
#include "shared.h"
#include "stdriot.h"

#define PROGNAME "rbpf-bench.bin"

#define RBPF_STACK_SIZE   (512)
#define BYTECODE_SIZE_MAX (600)
#define BUFFER_SIZE_MAX   (362)


static char buf[BUFFER_SIZE_MAX];
static uint8_t rbpf_stack[RBPF_STACK_SIZE];
static char bytecode[BYTECODE_SIZE_MAX];
static size_t bytecode_size;

typedef enum callee_interface_e {
    CALLEE_INTERFACE_INCR       = 0, // uint32_t
    CALLEE_INTERFACE_SQUARE     = 1, // uint32_t
    CALLEE_INTERFACE_FIB        = 2, // uint32_t
    CALLEE_INTERFACE_BITSWAP    = 3, // No param
    CALLEE_INTERFACE_SOCKBUF    = 4, // No param
    CALLEE_INTERFACE_MEMCPY     = 5, // uint32_t
    CALLEE_INTERFACE_FLETCHER32 = 6, // filename
    CALLEE_INTERFACE_BSORT      = 7, // No param

    CALLEE_INTERFACE_FIRST      = CALLEE_INTERFACE_INCR,
    CALLEE_INTERFACE_LAST       = CALLEE_INTERFACE_BSORT
} callee_interface_t;

static const char *callee_interface_param_names[CALLEE_INTERFACE_LAST + 1] = {
    [CALLEE_INTERFACE_INCR       ] = "incr.rbpf uint32_t",
    [CALLEE_INTERFACE_SQUARE     ] = "square.rbpf uint32_t",
    [CALLEE_INTERFACE_FIB        ] = "fib uint32_t",
    [CALLEE_INTERFACE_BITSWAP    ] = "bitswap <No param>",
    [CALLEE_INTERFACE_SOCKBUF    ] = "sockbuf <No param>",
    [CALLEE_INTERFACE_MEMCPY     ] = "memcpy uint32_t",
    [CALLEE_INTERFACE_FLETCHER32 ] = "fletcher32 filename",
    [CALLEE_INTERFACE_BSORT      ] = "bsort <No param>"
};

static void usage() {
    printf(PROGNAME": <n> <rbpf-file> <format> where format is :\n");

    for(unsigned int i = 0;
        i < (sizeof(callee_interface_param_names)/sizeof(callee_interface_param_names[0]));
        ++i
    ) {
        printf("%u %s\n", i, callee_interface_param_names[i]);
    }
}


static int
bpf_print_result(int64_t result, int status)
{
    switch (status) {
    case RBPF_OK:
        printf(PROGNAME": %lx\n", (uint32_t)result);
        return 0;
        break;
    case RBPF_ILLEGAL_MEM:
        printf(PROGNAME": illegal memory access\n");
        return 1;
        break;
    default:
        printf(PROGNAME": error\n");
        return 1;
    }
}


#define BPF_RUN_N(ctx, size, result, status)                \
    do {                                                    \
        unsigned int i;                                     \
        status = RBPF_OK;                                   \
        for (i = 0; i < n && status == RBPF_OK; i++) {      \
            status = rbpf_application_run_ctx(rbpf, ctx,    \
                size, &result);                             \
        } \
    } while (0)

static int
bpf_run_with_integer(rbpf_application_t *rbpf, unsigned n, uint64_t integer) {
    int64_t result;
    int status;

    BPF_RUN_N(&integer, sizeof(integer), result, status);

    return bpf_print_result(result, status);
}

static int bpf_run_with_context(rbpf_application_t *rbpf, unsigned n, void *context,
                                size_t context_size) {
    int64_t result = 0;
    int status;

    BPF_RUN_N(context, context_size, result, status);

    return bpf_print_result(result, status);
}


static int bpf_load_file_to_buffer(const char *filename) {
    int result = copy_file(filename, buf, BUFFER_SIZE_MAX);
    if (result < 0) {
        printf(PROGNAME ": failed to load data from file \"%s\".\n", filename);
    } else
        printf(PROGNAME": \"%s\" data loaded at address %p\n", filename, (void *)buf);

    return result;
}


#define ARRAY_LENGTH 40

uint32_t sockbuf_array[ARRAY_LENGTH];

typedef struct sockbuf_ctx_s
{
    uint32_t data_start;
    uint32_t data_end;
    uint32_t len;
    __bpf_shared_ptr(uint32_t *, array);
} sockbuf_ctx_t;

static int bpf_run_sockbuf(rbpf_application_t *rbpf, unsigned n) {
    rbpf_mem_region_t region;

    for(unsigned int i = 0; i < ARRAY_LENGTH; ++i) {
        sockbuf_array[i] = 0;
    }

    sockbuf_ctx_t ctx = {
        .data_start = 100,
        .data_end   = 200,
        .len        = 9,
        .array      = sockbuf_array
    };

    rbpf_memory_region_init(&region, &sockbuf_array, sizeof(sockbuf_array), RBPF_MEM_REGION_READ | RBPF_MEM_REGION_WRITE);
    rbpf_add_region(rbpf, &region);

    return bpf_run_with_context(rbpf, n, (void *)&ctx, sizeof(ctx));
}


typedef struct {
    __bpf_shared_ptr(const uint16_t *, data);
    uint32_t words;
} fletcher32_ctx_t;

static int bpf_run_fletcher32(rbpf_application_t *rbpf, unsigned n, int argc, char **argv) {

    rbpf_mem_region_t region;
    int buf_size;

    if (argc < 5) {
        usage();
        return 1;
    }

    buf_size = bpf_load_file_to_buffer(argv[4]);
    if (buf_size < 0)
        return 1;

    fletcher32_ctx_t ctx = {
        .data = (const uint16_t*)(uintptr_t)buf,
        .words = buf_size/2,
    };

    rbpf_memory_region_init(&region, buf, buf_size, RBPF_MEM_REGION_READ);
    rbpf_add_region(rbpf, &region);

    return bpf_run_with_context(rbpf, n, (void *)&ctx, sizeof(ctx));
}

#ifdef WIP
static char src_data[] =
        "AD3Awn4kb6FtcsyE0RU25U7f55Yncn3LP3oEx9Gl4qr7iDW7I8L6Pbw9jNnh0sE4DmCKuc"
        "d1J8I34vn31W924y5GMS74vUrZQc08805aj4Tf66HgL1cO94os10V2s2GDQ825yNh9Yuq3"
        /*"QHcA60xl31rdA7WskVtCXI7ruH1A4qaR6Uk454hm401lLmv2cGWt5KTJmr93d3JsGaRRPs"
        "4HqYi4mFGowo8fWv48IcA3N89Z99nf0A0H2R6P0uI4Tir682Of3Rk78DUB2dIGQRRpdqVT"
        "tLhgfET2gUGU65V3edSwADMqRttI9JPVz8JS37g5QZj4Ax56rU1u0m0K8YUs57UYG5645n"
        "byNy4yqxu7"*/;

static char dst_data[520];

typedef struct memcpy_ctx_s
{
    __bpf_shared_ptr(char*, src);
    __bpf_shared_ptr(char*, dst);
    uint32_t len;
} memcpy_ctx_t;

static int bpf_run_memcpy(rbpf_application_t *rbpf, unsigned n, int argc, char **argv) {

    rbpf_mem_region_t src_region, dst_region;
    const uint32_t src_data_size = sizeof(src_data) / sizeof(src_data[0]);

    if (argc < 5) {
        usage();
        return 1;
    }
    char *endptr;
    int64_t memcpy_size = (unsigned)strtol(argv[4], &endptr, 10);
    if (argv[4] == endptr ||  *endptr != '\0') {
        printf(PROGNAME": %s: failed to parse memcpy size\n", argv[4]);
        return -1;
    }

    memcpy_ctx_t memcpy_ctx = {
        .src = src_data,
        .dst = dst_data,
        .len = (memcpy_size > src_data_size) ? src_data_size : memcpy_size
    };


    rbpf_memory_region_init(&src_region, src_data, sizeof(src_data), RBPF_MEM_REGION_READ);
    rbpf_add_region(rbpf, &src_region);

    rbpf_memory_region_init(&dst_region, dst_data, sizeof(dst_data), RBPF_MEM_REGION_WRITE);
    rbpf_add_region(rbpf, &dst_region);


    return bpf_run_with_context(rbpf, n, (void *)&memcpy_ctx, sizeof(memcpy_ctx));
}


typedef struct bsort_context_s
{
    int size;
    int* arr;
} bsort_context_t;

static int unsort_list[] = {5923, 3314, 6281, 2408, 9997, 4393, 772, 3983, 4083, 3212, 9096, 1973, 7792, 1627, 1812, 1683, 4615, 8370, 7379, 1188, 2511, 1115, 9226, 9025, 1898, 5529, 3674, 7868, 750, 2393, 9372, 4370};

static int bpf_run_bsort(rbpf_application_t *rbpf, unsigned n) {
    bsort_context_t context = {
        .size = sizeof(unsort_list) / sizeof(unsort_list[0]),
        .arr  = unsort_list
    };

    rbpf_mem_region_t region;
    rbpf_memory_region_init(&region, unsort_list, sizeof(unsort_list), RBPF_MEM_REGION_READ | RBPF_MEM_REGION_WRITE);
    rbpf_add_region(rbpf, &region);

    return bpf_run_with_context(rbpf, n, (void *)&context, sizeof(context));
}
#endif // WIP


static int
bpf_run(rbpf_application_t *rbpf, unsigned n)
{
    int64_t result;
    int status;

    BPF_RUN_N(NULL, 0, result, status);

    return bpf_print_result(result, status);
}

int
main(int argc, char **argv)
{
    rbpf_application_t rbpf;
    rbpf_mem_region_t region;
    uint64_t integer;
    ssize_t result;
    callee_interface_t callee_interface;
    unsigned n;
    char *endptr;
/*
    for(int i = 0; i < argc; ++i)
        printf("arg %d = %s\n", i, argv[i]);

    printf("DEBUG 0\n");
*/
    if (argc < 4) {
        usage();
        return 1;
    }

    /*
    printf("DEBUG n\n");
    */
    n = (unsigned)strtol(argv[1], &endptr, 10);
    if (argv[1] == endptr ||  *endptr != '\0') {
        printf(PROGNAME": %s: failed to parse run number\n", argv[1]);
        return 1;
    }
    /*
    printf("DEBUG n=%d\n", n);
    */

    if ((result = copy_file(argv[2], bytecode, BYTECODE_SIZE_MAX)) < 0) {
        printf(PROGNAME": %s: failed to copy bytecode\n", argv[2]);
        return 1;
    }
    bytecode_size = (size_t)result;

    printf(PROGNAME": \"%s\" bytecode loaded at address %p\n", argv[2],
        (void *)bytecode);

    rbpf_application_setup(&rbpf, rbpf_stack, (void *)bytecode,
        bytecode_size);
    rbpf_memory_region_init(&region, bytecode, bytecode_size,
        RBPF_MEM_REGION_READ);
    rbpf_add_region(&rbpf, &region);

    /*
    printf("DEBUG callee interface\n");
    */
    callee_interface = (callee_interface_t)strtol(argv[3], &endptr, 10);
    if (argv[3] == endptr ||  *endptr != '\0') {
        printf(PROGNAME": %s: failed to parse format number\n", argv[1]);
        return 1;
    }

    /*
    printf("DEBUG callee interface = %d\n", callee_interface);
    */

    switch(callee_interface) {
        case CALLEE_INTERFACE_INCR :
            /* FALLTHROUGH */
        case CALLEE_INTERFACE_SQUARE :
            /* FALLTHROUGH */
        case CALLEE_INTERFACE_FIB : {
            if (argc < 4) {
                usage();
                return 1;
            }
            integer = (unsigned)strtol(argv[4], &endptr, 10);
            if (argv[4] == endptr ||  *endptr != '\0') {
                printf(PROGNAME": %s: failed to parse integer\n", argv[1]);
                return 1;
            }
            return bpf_run_with_integer(&rbpf, n, integer);
        }

        case CALLEE_INTERFACE_BITSWAP :
            return bpf_run(&rbpf, n);

        case CALLEE_INTERFACE_SOCKBUF :
            return bpf_run_sockbuf(&rbpf, n);

        case CALLEE_INTERFACE_FLETCHER32 :
            return bpf_run_fletcher32(&rbpf, n, argc, argv);
#ifdef WIP
        case CALLEE_INTERFACE_MEMCPY :
            return bpf_run_memcpy(&rbpf, n, argc, argv);
        case CALLEE_INTERFACE_BSORT :
            return bpf_run_bsort(&rbpf, n);
#endif // WIP
        default :
            usage();
            return 1;
    }

    /* other argument types are not yet supported */

    return 1;
}
