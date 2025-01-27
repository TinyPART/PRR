/*
 * Copyright (C) 2024 Université de Lille
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_fs_xipfs
 * @{
 *
 * @file
 * @brief       xipfs I/O buffer implementation
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

/*
 * libc includes
 */
#include <stdint.h>
#include <string.h>

/*
 * RIOT include
 */
#include "periph/flashpage.h"

/*
 * xipfs includes
 */
#include "include/buffer.h"
#include "include/errno.h"
#include "include/flash.h"

/**
 * @internal
 *
 * @brief An enumeration that describes the state of the buffer
 */
typedef enum xipfs_buffer_state_e {
    /**
     * A valid buffer state
     */
    XIPFS_BUFFER_OK,
    /**
     *  An invalid buffer state
     */
    XIPFS_BUFFER_KO,
} xipfs_buffer_state_t;

/**
 * @internal
 *
 * @brief A structure that describes xipfs buffer
 */
typedef struct xipfs_buf_s {
    /**
     * The state of the buffer
     */
    xipfs_buffer_state_t state;
    /**
     * The I/O buffer
     */
    char buf[FLASHPAGE_SIZE];
    /**
     * The flash page number loaded into the I/O buffer
     */
    unsigned page_num;
    /**
     * The flash page address loaded into the I/O buffer
     */
    char *page_addr;
} xipfs_buf_t;

/**
 * @internal
 *
 * @brief The buffer used by xipfs
 */
static xipfs_buf_t xipfs_buf = {
    .state = XIPFS_BUFFER_KO,
};

/**
 * @internal
 *
 * @pre num must be a valid flash page number
 *
 * @brief Check whether the page passed as an argument is the
 * same as the one in the buffer
 *
 * @param num A flash page number
 *
 * @return Returns one if the page passed as an argument is the
 * same as the one in the buffer or a zero otherwise
 */
static int xipfs_buffer_page_changed(unsigned num)
{
    return xipfs_buf.page_num != num;
}

/**
 * @internal
 *
 * @brief Checks whether the I/O buffer requires flushing
 *
 * @return Returns one if the buffer requires flushing or a
 * zero otherwise
 */
static int xipfs_buffer_need_flush(void)
{
    size_t i;

    if (xipfs_buf.state == XIPFS_BUFFER_KO) {
        return 0;
    }

    for (i = 0; i < FLASHPAGE_SIZE; i++) {
        if (xipfs_buf.buf[i] != xipfs_buf.page_addr[i]) {
            return 1;
        }
    }

    return 0;
}

/**
 * @internal
 *
 * @brief Flushes the I/O buffer
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_flush(void)
{
    void *src, *dest;
    unsigned num;
    size_t len;

    if (xipfs_buffer_need_flush() == 0) {
        /* no need to flush the buffer */
        return 0;
    }

    num = xipfs_buf.page_num;

    if (xipfs_flash_erase_page(num) < 0) {
        /* xipfs_errno was set */
        return -1;
    }

    src = xipfs_buf.buf;
    dest = xipfs_buf.page_addr;
    len = FLASHPAGE_SIZE;

    if (xipfs_flash_write_unaligned(dest, src, len) < 0) {
        /* xipfs_errno was set */
        return -1;
    }

    (void)memset(&xipfs_buf, 0, sizeof(xipfs_buf));
    xipfs_buf.state = XIPFS_BUFFER_KO;

    return 0;
}

/**
 * @internal
 *
 * @pre num must be a valid flash page number
 *
 * @pre addr must be a valid flash page address
 *
 * @brief Loads a flash page into the I/O buffer
 *
 * @param num The number of the flash page to load into the I/O
 * buffer
 *
 * @param addr The address of the flash page to load into the
 * I/O buffer
 */
static void xipfs_buffer_load(unsigned num, void *addr)
{
    size_t i;

    for (i = 0; i < FLASHPAGE_SIZE; i++) {
        xipfs_buf.buf[i] = ((char *)addr)[i];
    }
    xipfs_buf.page_num = num;
    xipfs_buf.page_addr = addr;
    xipfs_buf.state = XIPFS_BUFFER_OK;
}

/**
 * @brief Buffered implementation of the read(2) function
 *
 * @param dest A pointer to an accessible memory region where to
 * store the read bytes
 *
 * @param src A pointer to an accessible memory region from
 * which to read bytes
 *
 * @param len The number of bytes to read
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_read(void *dest, const void *src, size_t len)
{
    void *addr, *ptr;
    size_t pos, i;
    unsigned num;

    for (i = 0; i < len; i++) {
        ptr = (char *)src + i;
        if (xipfs_flash_in(ptr) < 0) {
            /* xipfs_errno was set */
            return -1;
        }
        num = flashpage_page(ptr);
        addr = flashpage_addr(num);
        if (xipfs_buf.state == XIPFS_BUFFER_KO) {
            xipfs_buffer_load(num, addr);
        } else if (xipfs_buffer_page_changed(num) == 1) {
            if (xipfs_buffer_flush() < 0) {
                /* xipfs_errno was set */
                return -1;
            }
            xipfs_buffer_load(num, addr);
        }
        pos = (uintptr_t)ptr % FLASHPAGE_SIZE;
        ((char *)dest)[i] = xipfs_buf.buf[pos];
    }

    return 0;
}

/**
 * @brief Reads a byte
 *
 * @param dest A pointer to an accessible memory region where to
 * store the read byte
 *
 * @param src A pointer to an accessible memory region from
 * which to read byte
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_read_8(char *dest, void *src)
{
    return xipfs_buffer_read(dest, src, sizeof(*dest));
}

/**
 * @brief Read a word
 *
 * @param dest A pointer to an accessible memory region where to
 * store the read bytes
 *
 * @param src A pointer to an accessible memory region from
 * which to read bytes
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_read_32(unsigned *dest, void *src)
{
    return xipfs_buffer_read(dest, src, sizeof(*dest));
}

/**
 * @brief Buffered implementation of the write(2) function
 *
 * @param dest A pointer to an accessible memory region where to
 * store the bytes to write
 *
 * @param src A pointer to an accessible memory region from
 * which to write bytes
 *
 * @param len The number of bytes to write
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_write(void *dest, const void *src, size_t len)
{
    void *addr, *ptr;
    size_t pos, i;
    unsigned num;

    for (i = 0; i < len; i++) {
        ptr = (char *)dest + i;
        if (xipfs_flash_in(ptr) < 0) {
            /* xipfs_errno was set */
            return -1;
        }
        num = flashpage_page(ptr);
        addr = flashpage_addr(num);
        if (xipfs_buf.state == XIPFS_BUFFER_KO) {
            xipfs_buffer_load(num, addr);
        } else if (xipfs_buffer_page_changed(num) == 1) {
            if (xipfs_buffer_flush() < 0) {
                /* xipfs_errno was set */
                return -1;
            }
            xipfs_buffer_load(num, addr);
        }
        pos = (uintptr_t)ptr % FLASHPAGE_SIZE;
        xipfs_buf.buf[pos] = ((char *)src)[i];
    }

    return 0;
}

/**
 * @brief Write a byte
 *
 * @param dest A pointer to an accessible memory region where to
 * write byte
 *
 * @param src A pointer to an accessible memory region from
 * which to write byte
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_write_8(void *dest, char src)
{
    return xipfs_buffer_write(dest, &src, sizeof(src));
}

/**
 * @brief Write a word
 *
 * @param dest A pointer to an accessible memory region where to
 * write bytes
 *
 * @param src A pointer to an accessible memory region from
 * which to write bytes
 *
 * @return Returns zero if the function succeeds or a negative
 * value otherwise
 */
int xipfs_buffer_write_32(void *dest, unsigned src)
{
    return xipfs_buffer_write(dest, &src, sizeof(src));
}
