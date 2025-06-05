/**
 * @file mb2parse.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/BSP-GRUB - GRUB bootstrapper for Arctan-OS/Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/BSP-GRUB
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
 * Provides a function for reading the multiboot2 boot information tag
 * structure.
*/
#ifndef ARC_BOOT_MB2PARSE_H
#define ARC_BOOT_MB2PARSE_H

#include <stdint.h>

/**
 * Reads the tags provided by boothloader.
 *
 * Populates the _boot_meta, also initializes the
 * PMM.
 *
 * @param void *mb2i - Pointer to the base of the first tag.
 * @return Error code (0: success).
 * */
int parse_mb2i(uint8_t *mb2i);

#endif
