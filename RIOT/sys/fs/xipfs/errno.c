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
 * @brief       xipfs errno implementation
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

/*
 * libc include
 */
#include <errno.h>

/*
 * xipfs include
 */
#include "include/errno.h"

/*
 * Global variables
 */

/**
 * @brief The xipfs errno global variable
 */
int xipfs_errno = XIPFS_OK;

/**
 * @internal
 *
 * @brief Map xipfs errno number to the associated error string
 */
static const char *xipfs_errno_to_str[XIPFS_ENUM] = {
    [XIPFS_OK] = "",
    [XIPFS_ENULLP] = "path is null",
    [XIPFS_EEMPTY] = "path is empty",
    [XIPFS_EINVAL] = "invalid character",
    [XIPFS_ENULTER] = "path is not null-terminated",
    [XIPFS_ENULLF] = "file pointer is null",
    [XIPFS_EALIGN] = "file is not page-aligned",
    [XIPFS_EOUTNVM] = "file is outside NVM space",
    [XIPFS_ELINK] = "file improperly linked to others",
    [XIPFS_EMAXOFF] = "offset exceeds max position",
    [XIPFS_ENVMC] = "NVMC error",
    [XIPFS_ENULLM] = "mount point is null",
    [XIPFS_EMAGIC] = "bad magic number",
    [XIPFS_EPAGNUM] = "bad page number",
    [XIPFS_EFULL] = "file system full",
    [XIPFS_EEXIST] = "file already exists",
    [XIPFS_EPERM] = "file has wrong permissions",
    [XIPFS_ENOSPACE] = "insufficient space to create the file",
};

/**
 * @brief Maps xipfs errno number to the associated error string
 *
 * @param errnum The errno number to map
 *
 * @return The associated error string
 */
const char *xipfs_strerror(int errnum)
{
    unsigned num = errnum;

    if (num >= XIPFS_ENUM) {
        return "unknown xipfs errno";
    }

    return xipfs_errno_to_str[errnum];
}
