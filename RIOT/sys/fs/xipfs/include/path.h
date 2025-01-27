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
 * @brief       xipfs path implementation header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_PATH_H
#define FS_XIPFS_PATH_H

/*
 * RIOT include
 */
#include "fs/xipfs.h"

/**
 * @def XIPFS_PATH_UNDEFINED
 *
 * @brief The xipfs path is undefined
 */
#define XIPFS_PATH_UNDEFINED (0)

/**
 * @def XIPFS_PATH_CREATABLE
 *
 * @brief The xipfs path is creatable as file or as empty
 * directory
 */
#define XIPFS_PATH_CREATABLE (1)

/**
 * @def XIPFS_PATH_EXISTS_AS_FILE
 *
 * @brief The xipfs path exists as file
 */
#define XIPFS_PATH_EXISTS_AS_FILE (2)

/**
 * @def XIPFS_PATH_EXISTS_AS_EMPTY_DIR
 *
 * @brief The xipfs path exists as empty directory
 */
#define XIPFS_PATH_EXISTS_AS_EMPTY_DIR (3)

/**
 * @def XIPFS_PATH_EXISTS_AS_NONEMPTY_DIR
 *
 * @brief The xipfs path exists as non-empty directory
 */
#define XIPFS_PATH_EXISTS_AS_NONEMPTY_DIR (4)

/**
 * @def XIPFS_PATH_INVALID_BECAUSE_NOT_DIRS
 *
 * @brief The xipfs path is invalid because one of the parent in
 * the path is not a directory
 */
#define XIPFS_PATH_INVALID_BECAUSE_NOT_DIRS (5)

/**
 * @def XIPFS_PATH_INVALID_BECAUSE_NOT_FOUND
 *
 * @brief The xipfs path is invalid because one of the parent in
 * the path does not exist
 */
#define XIPFS_PATH_INVALID_BECAUSE_NOT_FOUND (6)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A structure representing an xipfs path
 */
typedef struct xipfs_path_s {
    /**
     * The xipfs path
     */
    char path[XIPFS_PATH_MAX];
    /**
     * The dirname components of xipfs path
     */
    char dirname[XIPFS_PATH_MAX];
    /**
     * The basename component of xipfs path
     */
    char basename[XIPFS_PATH_MAX];
    /**
     * The length of the xipfs path
     */
    size_t len;
    /**
     * The index of the last slash in the xipfs path
     */
    size_t last_slash;
    /**
     * The number of xipfs file structures in an xipfs file
     * system that tracks the path of the parent directory
     */
    size_t parent;
    /**
     * The xipfs file structure that enables the identification
     * of the type of the xipfs path
     */
    xipfs_file_t *witness;
    /**
     * The type of the xipfs path
     */
    unsigned char info;
} xipfs_path_t;

int xipfs_path_new(xipfs_mount_t *vfs_mp, xipfs_path_t *xipath, const char *path);
int xipfs_path_new_n(xipfs_mount_t *mp, xipfs_path_t *xipath, const char **path, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_PATH_H*/
