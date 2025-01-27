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
 * @brief       eXecute-In-place File System header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_H
#define FS_XIPFS_H

#include <stddef.h>
#include <stdint.h>

#include "vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def XIPFS_PATH_MAX
 *
 * @brief The maximum length of an xipfs path
 */
#define XIPFS_PATH_MAX (64)

/**
 * @def XIPFS_MAGIC
 *
 * @brief The magic number of an xipfs file system
 */
#define XIPFS_MAGIC 0xf9d3b6cb

/**
 * @def XIPFS_FILESIZE_SLOT_MAX
 *
 * @brief The maximum slot number for the list holding file
 * sizes
 */
#define XIPFS_FILESIZE_SLOT_MAX (86)

/*
 * @def EXEC_ARGC_MAX
 *
 * @brief The maximum number of arguments on the command line
 */
#define EXEC_ARGC_MAX (SHELL_DEFAULT_BUFSIZE / 2)

/**
 * @def XIPFS_NEW_PARTITION
 *
 * @brief Allocate a new contiguous space aligned to a page in
 * the non-volatile addressable memory of the MCU to serve as a
 * partition for an xipfs file system
 *
 * @param id Identifier name for the mount point used by
 * functions that manipulate xipfs file systems
 *
 * @param mp The mount point of the file system in the VFS tree
 *
 * @param npage The total number of pages allocated for the
 * partition
 */
#define XIPFS_NEW_PARTITION(id, mp, npage) \
    FLASH_WRITABLE_INIT(xipfs_desc_##id, npage); \
    static xipfs_mount_t id = { \
        .vfs = { \
            .fs = &xipfs_file_system, \
            .mount_point = mp, \
            .private_data = (void *)xipfs_desc_##id, \
        }, \
        .magic = XIPFS_MAGIC, \
        .nbpage = npage, \
    }

/**
 * @brief File data structure for xipfs
 */
typedef struct xipfs_file_s {
    /**
     * The address of the next file
     */
    struct xipfs_file_s *next;
    /**
     * The path of the file relative to the mount point
     */
    char path[XIPFS_PATH_MAX];
    /**
     * The actual size reserved for the file
     */
    size_t reserved;
    /**
     * The table lists the file sizes, with the last entry
     * reflecting the current size of the file. This method
     * helps to avoid flashing the flash page every time there
     * is a change in size
     */
    size_t size[XIPFS_FILESIZE_SLOT_MAX];
    /**
     * Execution right
     */
    uint32_t exec;
    /**
     * First byte of the file's data
     */
    unsigned char buf[0];
} xipfs_file_t;

/**
 * @brief A specialized mount point data structure for xipfs
 */
typedef struct xipfs_mount_s {
    /**
     * A VFS mount point data structure
     */
    vfs_mount_t vfs;
    /**
     * The magic number of the file system
     */
    uint32_t magic;
    /**
     * The number of pages reserved for the file system
     */
    size_t nbpage;
} xipfs_mount_t;

/**
 * @brief xipfs file system driver
 */
extern const vfs_file_system_t xipfs_file_system;

/*
 * xipfs-specific functions
 */

int xipfs_new_file(const char *path, uint32_t size, uint32_t exec);
int xipfs_execv(const char *pathname, char *const argv[]);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_H */

/** @} */
