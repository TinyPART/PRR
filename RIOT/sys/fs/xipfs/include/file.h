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
 * @brief       xipfs file implementation header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_FILE_H
#define FS_XIPFS_FILE_H

#include "shell.h"

/**
 * @def EXEC_ARGC_MAX
 *
 * @brief The maximum number of arguments to pass to the binary
 */
#define EXEC_ARGC_MAX (SHELL_DEFAULT_BUFSIZE / 2)

#ifdef __cplusplus
extern "C" {
#endif

extern char *xipfs_infos_file;

int xipfs_file_erase(xipfs_file_t *filp);
int xipfs_file_exec(xipfs_file_t *filp, char *const argv[]);
int xipfs_file_filp_check(xipfs_file_t *filp);
off_t xipfs_file_get_max_pos(vfs_file_t *vfs_filp);
off_t xipfs_file_get_reserved(vfs_file_t *vfs_filp);
off_t xipfs_file_get_size(vfs_file_t *vfs_filp);
off_t xipfs_file_get_size_(xipfs_file_t *filp);
int xipfs_file_path_check(const char *path);
int xipfs_file_read_8(vfs_file_t *vfs_filp, char *byte);
int xipfs_file_rename(xipfs_file_t *filp, const char *to_path);
int xipfs_file_set_size(vfs_file_t *vfs_fp, off_t size);
int xipfs_file_write_8(vfs_file_t *vfs_filp, char byte);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_FILE_H */
