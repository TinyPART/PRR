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
 * @brief       xipfs file system implementation header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_FS_H
#define FS_XIPFS_FS_H

#ifdef __cplusplus
extern "C" {
#endif

int xipfs_fs_format(xipfs_mount_t *vfs_mp);
int xipfs_fs_free_pages(xipfs_mount_t *vfs_mp);
int xipfs_fs_get_page_number(xipfs_mount_t *vfs_mp);
xipfs_file_t *xipfs_fs_head(xipfs_mount_t *vfs_mp);
int xipfs_fs_mountp_check(xipfs_mount_t *mountp);
xipfs_file_t *xipfs_fs_new_file(xipfs_mount_t *vfs_mp, const char *path, size_t size, int exec);
xipfs_file_t *xipfs_fs_next(xipfs_file_t *filp);
int xipfs_fs_remove(void *dst);
int xipfs_fs_rename_all(xipfs_mount_t *vfs_mp, const char *from, const char *to);
xipfs_file_t *xipfs_fs_tail(xipfs_mount_t *vfs_mp);
xipfs_file_t *xipfs_fs_tail_next(xipfs_mount_t *vfs_mp);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_FS_H*/
