/*
 * Copyright (C) 2024 Université de Lille
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example of a post-issuance software
 *
 * @author      Damien Amara <damien.amara@univ-lille.fr>
 *
 * @}
 */

#include <stdriot.h>

int main(int argc, char **argv)
{
    int i;

    printf("Hello World!\n");

    for (i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    return 0;
}
