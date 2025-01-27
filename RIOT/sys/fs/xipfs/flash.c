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
 * @brief       Low-level NVM management implementation
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

/*
 * RIOT include
 */
#include "periph/flashpage.h"

/*
 * xipfs include
 */
#include "include/errno.h"

#ifndef CPU_FLASH_BASE
#error "sys/fs/xipfs: the target MCU does not define CPU_FLASH_BASE"
#endif /* !CPU_FLASH_BASE */

#ifndef FLASHPAGE_SIZE
#error "sys/fs/xipfs: the target MCU does not define FLASHPAGE_SIZE"
#endif /* !FLASHPAGE_SIZE */

#ifndef FLASHPAGE_NUMOF
#error "sys/fs/xipfs: the target MCU does not define FLASHPAGE_NUMOF"
#endif /* !FLASHPAGE_NUMOF */

/**
 * @brief Returns the MCU flash memory base address
 *
 * @return The MCU flash memory base address
 */
unsigned xipfs_flash_base_addr(void)
{
    return CPU_FLASH_BASE;
}

/**
 * @brief Returns the MCU flash memory end address
 *
 * @return The MCU flash memory end address
 */
unsigned xipfs_flash_end_addr(void)
{
    return CPU_FLASH_BASE + FLASHPAGE_NUMOF * FLASHPAGE_SIZE;
}

/**
 * @brief Checks whether an address points to the MCU's
 * flash memory address space
 *
 * @param addr The address to check
 *
 * @return 1 if the address points to the MCU's flash
 * memory address space, 0 otherwise
 */
int xipfs_flash_in(const void *addr)
{
    uintptr_t val = (uintptr_t)addr;

    return
#if CPU_FLASH_BASE > 0
        (val >= xipfs_flash_base_addr()) &&
#endif
        (val < xipfs_flash_end_addr());
}

/**
 * @brief Checks whether an address is aligned with a
 * flash page
 *
 * @param addr The address to check
 *
 * @return 1 if the address is aligned with a flash
 * page, 0 otherwise
 */
int xipfs_flash_page_aligned(const void *addr)
{
    uintptr_t val = (uintptr_t)addr;

    return val % FLASHPAGE_SIZE == 0;
}

/**
 * @brief Checks whether the copy of n bytes from addr
 * overflows the MCU's flash memory address space
 *
 * @param addr The address from which to copy n bytes
 *
 * @param n The number of bytes to copy from addr
 *
 * @return 1 if the copy of n bytes from addr overflows
 * the MCU's flash memory address space
 */
int xipfs_flash_overflow(const void *addr, size_t n)
{
    return !xipfs_flash_in((char *)addr + n);
}

/**
 * @brief Checks whether the copy of n bytes from addr
 * overflows the flash page pointed to by addr
 *
 * @param addr The address from which to copy n bytes
 *
 * @param n The number of bytes to copy from addr
 *
 * @return 1 if the copy of n bytes from addr overflows
 * the flash page pointed to by addr, 0 otherwise
 */
int xipfs_flash_page_overflow(const void *addr, size_t n)
{
    uintptr_t val;

    val = (uintptr_t)addr;

    return !(val % FLASHPAGE_SIZE + n <= FLASHPAGE_SIZE);
}

/**
 * @brief Copy n bytes from the unaligned memory area
 * src to the unaligned memory area dest
 *
 * @param dest The address where to copy n bytes from
 * src
 *
 * @param src The address from which to copy n bytes to
 * dest
 *
 * @param n The number of bytes to copy from src to dest
 *
 * @return 0 if the n bytes were copied from src to
 * dest, -1 otherwise
 *
 * @warning src and dest addresses must be different
 *
 * @warning dest must point to the MCU's flash memory
 * address space
 *
 * @warning The copy must not overflow the flash page
 * pointed to by dest
 *
 * @warning The copy must no overflow the MCU's flash
 * memory address space
 */
int xipfs_flash_write_unaligned(void *dest, const void *src, size_t n)
{
    uint32_t mod, shift, addr, addr4, val4;
    uint8_t byte;
    size_t i;

    assert(dest != src);
    assert(xipfs_flash_in(dest) == 1);
    assert(xipfs_flash_overflow(dest, n) == 0);
    assert(xipfs_flash_page_overflow(dest, n) == 0);

    for (i = 0; i < n; i++) {
        /* retrieve the current byte to write */
        byte = ((uint8_t *)src)[i];

        /* cast the address to a 4-bytes integer */
        addr = (uint32_t)dest + i;

        /* calculate the modulus from the address */
        mod = addr & ((uint32_t)FLASHPAGE_WRITE_BLOCK_ALIGNMENT-1);

        /* align the address to the previous multiple of 4 */
        addr4 = addr & ~mod;

        /* read 4 bytes at the address aligned to a multiple of 4 */
        val4 = *(uint32_t *)addr4;

        /* calculate the byte shift value */
        shift = mod << ((uint32_t)FLASHPAGE_WRITE_BLOCK_SIZE-1);

        /* clear the byte corresponding to the shift */
        val4 &= ~((uint32_t)FLASHPAGE_ERASE_STATE << shift);

        /* set the byte corresponding to the shift */
        val4 |= (uint32_t)byte << shift;

        /* write bytes to flash memory */
        flashpage_write((void *)addr4, &val4, FLASHPAGE_WRITE_BLOCK_SIZE);

        /* checks the written byte against the expected byte */
        if (*(uint8_t *)addr != byte) {
            /* write failed */
            return -1;
        }
    }

    /* write succeeded */
    return 0;
}

/**
 * @brief Checks whether a flash page needs to be erased
 *
 * @param page The flash page to check
 *
 * @return 1 if the flash page needs to be erased, 0
 * otherwise
 */
int xipfs_flash_is_erased_page(unsigned page)
{
    char *ptr;
    size_t i;

    ptr = flashpage_addr(page);
    for (i = 0; i < FLASHPAGE_SIZE; i++) {
        if (ptr[i] != FLASHPAGE_ERASE_STATE) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Erases a flash page, if needed
 *
 * @param page The flash page to erase
 *
 * @return 0 if the flash page was erased or if the
 * flash page was already erased, -1 otherwise
 */
int xipfs_flash_erase_page(unsigned page)
{
    if (xipfs_flash_is_erased_page(page)) {
        return 0;
    }

    flashpage_erase(page);

    if (xipfs_flash_is_erased_page(page)) {
        return 0;
    }

    xipfs_errno = XIPFS_ENVMC;

    return -1;
}
