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
 * @brief       xipfs errno implementation header
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#ifndef FS_XIPFS_ERRNO_H
#define FS_XIPFS_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A enumration of all xipfs error numbers
 */
enum xipfs_errno_e {
    /**
     * No error
     */
    XIPFS_OK,
    /**
     * Path is null
     */
    XIPFS_ENULLP,
    /**
     * Path is empty
     */
    XIPFS_EEMPTY,
    /**
     * Invalid character
     */
    XIPFS_EINVAL,
    /**
     * Path is not null-terminated
     */
    XIPFS_ENULTER,
    /**
     * File pointer is null
     */
    XIPFS_ENULLF,
    /**
     * File is not page-aligned
     */
    XIPFS_EALIGN,
    /**
     * File is outside NVM space
     */
    XIPFS_EOUTNVM,
    /**
     * File improperly linked to others
     */
    XIPFS_ELINK,
    /**
     * Offset exceeds max position
     */
    XIPFS_EMAXOFF,
    /**
     * NVMC error
     */
    XIPFS_ENVMC,
    /**
     * Mount point is null
     */
    XIPFS_ENULLM,
    /**
     * Bad magic number
     */
    XIPFS_EMAGIC,
    /**
     * Bad page number
     */
    XIPFS_EPAGNUM,
    /**
     * File system full
     */
    XIPFS_EFULL,
    /**
     * File already exists
     */
    XIPFS_EEXIST,
    /**
     * File has wrong permissions
     */
    XIPFS_EPERM,
    /**
     * Insufficient space to create the file
     */
    XIPFS_ENOSPACE,
    /**
     * Error number - must be the last element
     */
    XIPFS_ENUM,
};

extern int xipfs_errno;

const char *xipfs_strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif /* FS_XIPFS_ERRNO_H */
