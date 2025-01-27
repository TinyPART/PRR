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
 * @brief       Low-level NVM management implementation header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_FLASH_H
#define FS_XIPFS_FLASH_H

/**
 * @def XIPFS_FLASH_ERASE_STATE
 *
 * @brief The erase state of the NVM as a 32-bit value
 */
#define XIPFS_FLASH_ERASE_STATE \
    ((FLASHPAGE_ERASE_STATE << 24) | \
     (FLASHPAGE_ERASE_STATE << 16) | \
     (FLASHPAGE_ERASE_STATE <<  8) | \
     (FLASHPAGE_ERASE_STATE      ))

#ifdef __cplusplus
extern "C" {
#endif

unsigned xipfs_flash_base_addr(void);
unsigned xipfs_flash_end_addr(void);
int xipfs_flash_erase_page(unsigned page);
int xipfs_flash_in(const void *addr);
int xipfs_flash_is_erased_page(unsigned page);
int xipfs_flash_overflow(const void *addr, size_t size);
int xipfs_flash_page_aligned(const void *addr);
int xipfs_flash_page_overflow(const void *addr, size_t size);
int xipfs_flash_write_32(void *dest, uint32_t src);
int xipfs_flash_write_8(void *dest, uint8_t src);
int xipfs_flash_write_unaligned(void *dest, const void *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_FLASH_H*/
