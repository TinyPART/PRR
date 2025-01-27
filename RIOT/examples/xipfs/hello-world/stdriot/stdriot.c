/*
 * Copyright (C) 2024 Université de Lille
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       stdriot implementation
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#include <stdarg.h>
#include <stddef.h>

#include "stdriot.h"

/*
 * Macro definitions
 */

/**
 * @internal
 *
 * @def SHELL_DEFAULT_BUFSIZE
 *
 * @brief Default shell buffer size (maximum line length shell
 * can handle)
 *
 * @see sys/include/shell.h
 */
#define SHELL_DEFAULT_BUFSIZE 128

/**
 * @internal
 *
 * @def XIPFS_FREE_RAM_SIZE
 *
 * @brief Amount of free RAM available for the relocatable
 * binary to use
 *
 * @see sys/fs/xipfs/file.c
 */
#define XIPFS_FREE_RAM_SIZE (512)

/**
 * @internal
 *
 * @def EXEC_STACKSIZE_DEFAULT
 *
 * @brief The default execution stack size of the binary
 *
 * @see sys/fs/xipfs/file.c
 */
#define EXEC_STACKSIZE_DEFAULT 1024

/**
 * @internal
 *
 * @def EXEC_ARGC_MAX
 *
 * @brief The maximum number of arguments to pass to the binary
 *
 * @see sys/fs/xipfs/include/file.h
 */
#define EXEC_ARGC_MAX (SHELL_DEFAULT_BUFSIZE / 2)

/**
 * @internal
 *
 * @def PANIC
 *
 * @brief This macro handles fatal errors
 */
#define PANIC() for (;;);

/*
 * Internal structures
 */

/**
 * @internal
 *
 * @brief Data structure that describes the memory layout
 * required by the CRT0 to execute the relocatable binary
 *
 * @see sys/fs/xipfs/file.c
 */
typedef struct crt0_ctx_s {
    /**
     * Start address of the binary in the NVM
     */
    void *bin_base;
    /**
     * Start address of the available free RAM
     */
    void *ram_start;
    /**
     * End address of the available free RAM
     */
    void *ram_end;
    /**
     * Start address of the free NVM
     */
    void *nvm_start;
    /**
     * End address of the free NVM
     */
    void *nvm_end;
} crt0_ctx_t;

/**
 * @internal
 *
 * @warning The order of the members in the enumeration must
 * remain synchronized with the order of the members of the same
 * enumeration declared in file sys/fs/xipfs/file.c
 *
 * @brief An enumeration describing the index of the functions
 * of the libc and RIOT in the system call table
 *
 * @see sys/fs/xipfs/file.c
 */
enum syscall_index_e {
    /**
     * Index of exit(3)
     */
    SYSCALL_EXIT,
    /**
     * Index of printf(3)
     */
    SYSCALL_PRINTF,
    /**
     * Maximum size of the syscall table used by the relocatable
     * binary. It must remain the final element of the
     * enumeration
     */
    SYSCALL_MAX,
};

/**
 * @internal
 *
 * @brief Data structure that describes the execution context of
 * a relocatable binary
 *
 * @see sys/fs/xipfs/file.c
 */
typedef struct exec_ctx_s {
    /**
     * Data structure required by the CRT0 to execute the
     * relocatable binary
     */
    crt0_ctx_t crt0_ctx;
    /**
     * Reserved memory space in RAM for the stack to be used by
     * the relocatable binary
     */
    char stkbot[EXEC_STACKSIZE_DEFAULT-4];
    /**
     * Last word of the stack indicating the top of the stack
     */
    char stktop[4];
    /**
     * Number of arguments passed to the relocatable binary
     */
    int argc;
    /**
     * Arguments passed to the relocatable binary
     */
    char *argv[EXEC_ARGC_MAX];
    /**
     * Table of function pointers for the libc and RIOT
     * functions used by the relocatable binary
     */
    void *syscall_table[SYSCALL_MAX];
    /**
     * Reserved memory space in RAM for the free RAM to be used
     * by the relocatable binary
     */
    char ram_start[XIPFS_FREE_RAM_SIZE-1];
    /**
     * Last byte of the free RAM
     */
    char ram_end;
} exec_ctx_t;

/*
 * Internal types
 */

/**
 * @internal
 *
 * @brief Pointer type for exit(3)
 */
typedef int (*exit_t)(int status);

/**
 * @internal
 *
 * @brief Pointer type for printf(3)
 */
typedef int (*vprintf_t)(const char *format, va_list ap);

/*
 * Global variable
 */

/**
 * @internal
 *
 * @brief A pointer to the system call table
 *
 * @see sys/fs/xipfs/file.c
 */
static void **syscall_table;

/**
 * @brief Wrapper that branches to the xipfs_exit(3) function
 *
 * @param status The exit status of the program
 *
 * @see sys/fs/xipfs/file.c
 */
extern void exit(int status)
{
    exit_t func;

    /* No need to save the R10 register, which holds the address
     * of the program's relocated GOT, since this register is
     * callee-saved according to the ARM Architecture Procedure
     * Call Standard, section 5.1.1 */
    func = syscall_table[SYSCALL_EXIT];
    (*func)(status);
}

/**
 * @brief Wrapper that branches to the RIOT's printf(3) function
 *
 * @param format The formatted string to print
 */
extern int printf(const char * format, ...)
{
    vprintf_t func;
    int res = 0;
    va_list ap;

    /* No need to save the R10 register, which holds the address
     * of the program's relocated GOT, since this register is
     * callee-saved according to the ARM Architecture Procedure
     * Call Standard, section 5.1.1 */
    func = syscall_table[SYSCALL_PRINTF];
    va_start(ap, format);
    res = (*func)(format, ap);
    va_end(ap);

    return res;
}

/**
 * @internal
 *
 * @brief The function to which CRT0 branches after the
 * executable has been relocated
 */
int start(exec_ctx_t *exec_ctx)
{
    int status, argc;
    char **argv;

    /* initialize the syscall table pointer */
    syscall_table = exec_ctx->syscall_table;

    /* initialize the arguments passed to the program */
    argc = exec_ctx->argc;
    argv = exec_ctx->argv;

    /* branch to the main() function of the program */
    extern int main(int argc, char **argv);
    status = main(argc, argv);

    /* exit the program */
    exit(status);

    /* should never be reached */
    PANIC();
}
