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
 * @brief       xipfs I/O buffer implementation header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_BUFFER_H
#define FS_XIPFS_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

int xipfs_buffer_flush(void);
int xipfs_buffer_read(void *dest, const void *src, size_t len);
int xipfs_buffer_read_32(unsigned *dest, void *src);
int xipfs_buffer_read_8(char *dest, void *src);
int xipfs_buffer_write(void *dest, const void *src, size_t len);
int xipfs_buffer_write_32(void *dest, unsigned src);
int xipfs_buffer_write_8(void *dest, char src);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_BUFFER_H*/
