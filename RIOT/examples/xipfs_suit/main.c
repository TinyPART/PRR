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
 * @brief       An application demonstrating xipfs
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#include "fs/xipfs.h"
#include "periph/flashpage.h"
#include "shell.h"
#include "thread.h"
#include "net/nanocoap_sock.h"

#define COAP_INBUF_SIZE (256U)

/* Extend stacksize of nanocoap server thread */
static char _nanocoap_server_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];
#define NANOCOAP_SERVER_QUEUE_SIZE     (8)
static msg_t _nanocoap_server_msg_queue[NANOCOAP_SERVER_QUEUE_SIZE];

/**
 * @def PANIC
 *
 * @brief This macro handles fatal errors
 */
#define PANIC() for (;;);

/**
 * @def NVME0P0_PAGE_NUM
 *
 * @brief The number of flash page for the nvme0p0 file system
 */
#define NVME0P0_PAGE_NUM 10

/**
 * @def NVME0P1_PAGE_NUM
 *
 * @brief The number of flash page for the nvme0p1 file system
 */
#define NVME0P1_PAGE_NUM 15

/*
 * Allocate a new contiguous space for the nvme0p0 file system
 */
XIPFS_NEW_PARTITION(nvme0p0, "/dev/nvme0p0", NVME0P0_PAGE_NUM);

/*
 * Allocate a new contiguous space for the nvme0p1 file system
 */
XIPFS_NEW_PARTITION(nvme0p1, "/dev/nvme0p1", NVME0P1_PAGE_NUM);

/**
 * @internal
 *
 * @brief Mount a partition, or if it is corrupted, format and
 * remount it
 *
 * @param xipfs_mp A pointer to a memory region containing an
 * xipfs mount point structure
 */
static void mount_or_format(xipfs_mount_t *xipfs_mp)
{
    if (vfs_mount(&xipfs_mp->vfs) < 0) {
        printf("vfs_mount: \"%s\": file system has not been "
            "initialized or is corrupted\n", xipfs_mp->vfs.mount_point);
        printf("vfs_format: \"%s\": try initializing it\n",
            xipfs_mp->vfs.mount_point);
        vfs_format(&xipfs_mp->vfs);
        printf("vfs_format: \"%s\": OK\n", xipfs_mp->vfs.mount_point);
        if (vfs_mount(&xipfs_mp->vfs) < 0) {
            printf("vfs_mount: \"%s\": file system is corrupted!\n",
                xipfs_mp->vfs.mount_point);
            PANIC();
        }
    }
    printf("vfs_mount: \"%s\": OK\n", xipfs_mp->vfs.mount_point);
}

static void *_nanocoap_server_thread(void *arg)
{
    (void)arg;

    /* nanocoap_server uses gnrc sock which uses gnrc which needs a msg queue */
    msg_init_queue(_nanocoap_server_msg_queue, NANOCOAP_SERVER_QUEUE_SIZE);

    /* initialize nanocoap server instance */
    uint8_t buf[COAP_INBUF_SIZE];
    sock_udp_ep_t local = { .port=COAP_PORT, .family=AF_INET6 };
    nanocoap_server(&local, buf, sizeof(buf));

    return NULL;
}

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    mount_or_format(&nvme0p0);
    mount_or_format(&nvme0p1);

        /* start nanocoap server thread */
    thread_create(_nanocoap_server_stack, sizeof(_nanocoap_server_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  _nanocoap_server_thread, NULL, "nanocoap server");


    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
