/**
 * @file terminal.c
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
*/
#include <arch/io/port.h>
#include <global.h>
#include <util.h>

// This is not at all cursed, don't worry about it
const uint8_t character_rom[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E, 0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E, 0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x38, 0x7C, 0x38, 0xFE, 0xFE, 0xD6, 0x10, 0x38, 0x10, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x10, 0x38, 0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00, 0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF, 0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00, 0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF, 0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18, 0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0, 0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0, 0x18, 0xDB, 0x3C, 0xE7, 0xE7, 0x3C, 0xDB, 0x18, 0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00, 0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00, 0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00, 0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0xCC, 0x78, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00, 0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00, 0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00, 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 0x30, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00, 0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00, 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00, 0x60, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00, 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00, 0x7C, 0xC6, 0xCE, 0xDE, 0xF6, 0xE6, 0x7C, 0x00, 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00, 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00, 0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00, 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00, 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00, 0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00, 0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60, 0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 0x30, 0x00, 0x7C, 0xC6, 0xDE, 0xDE, 0xDE, 0xC0, 0x78, 0x00, 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00, 0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00, 0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00, 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00, 0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00, 0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00, 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00, 0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00, 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00, 0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00, 0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00, 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00, 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00, 0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00, 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0xDC, 0x78, 0x1C, 0x00, 0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00, 0x78, 0xCC, 0x60, 0x30, 0x18, 0xCC, 0x78, 0x00, 0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x00, 0xC6, 0xC6, 0x6C, 0x38, 0x38, 0x6C, 0xC6, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00, 0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00, 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00, 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00, 0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00, 0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00, 0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 0x38, 0x6C, 0x60, 0xF0, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00, 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00, 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xC6, 0x00, 0x00, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, 0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0, 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E, 0x00, 0x00, 0xDC, 0x76, 0x66, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x00, 0x10, 0x30, 0x7C, 0x30, 0x30, 0x34, 0x18, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00, 0x00, 0x00, 0xC6, 0xD6, 0xFE, 0xFE, 0x6C, 0x00, 0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00, 0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00, 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x18, 0x0C, 0x78, 0x00, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00, 0x1C, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 0x7E, 0xC3, 0x3C, 0x06, 0x3E, 0x66, 0x3F, 0x00, 0xCC, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00, 0xE0, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00, 0x30, 0x30, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00, 0x00, 0x00, 0x78, 0xC0, 0xC0, 0x78, 0x0C, 0x38, 0x7E, 0xC3, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00, 0xCC, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 0xE0, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 0xCC, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 0x7C, 0xC6, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00, 0xE0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 0xC6, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0xC6, 0x00, 0x30, 0x30, 0x00, 0x78, 0xCC, 0xFC, 0xCC, 0x00, 0x1C, 0x00, 0xFC, 0x60, 0x78, 0x60, 0xFC, 0x00, 0x00, 0x00, 0x7F, 0x0C, 0x7F, 0xCC, 0x7F, 0x00, 0x3E, 0x6C, 0xCC, 0xFE, 0xCC, 0xCC, 0xCE, 0x00, 0x78, 0xCC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0xCC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0xE0, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x78, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00, 0x00, 0xE0, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00, 0x00, 0xCC, 0x00, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x18, 0x18, 0x7E, 0xC0, 0xC0, 0x7E, 0x18, 0x18, 0x38, 0x6C, 0x64, 0xF0, 0x60, 0xE6, 0xFC, 0x00, 0xCC, 0xCC, 0x78, 0xFC, 0x30, 0xFC, 0x30, 0x30, 0xF8, 0xCC, 0xCC, 0xFA, 0xC6, 0xCF, 0xC6, 0xC7, 0x0E, 0x1B, 0x18, 0x3C, 0x18, 0x18, 0xD8, 0x70, 0x1C, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00, 0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 0x00, 0x1C, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x1C, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00, 0x00, 0xF8, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0x00, 0xFC, 0x00, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00, 0x3C, 0x6C, 0x6C, 0x3E, 0x00, 0x7E, 0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x7C, 0x00, 0x00, 0x30, 0x00, 0x30, 0x60, 0xC0, 0xCC, 0x78, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x0C, 0x0C, 0x00, 0x00, 0xC3, 0xC6, 0xCC, 0xDE, 0x33, 0x66, 0xCC, 0x0F, 0xC3, 0xC6, 0xCC, 0xDB, 0x37, 0x6F, 0xCF, 0x03, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00, 0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0xDB, 0x77, 0xDB, 0xEE, 0xDB, 0x77, 0xDB, 0xEE, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x18, 0x18, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36, 0xF6, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x36, 0x36, 0x36, 0x00, 0x00, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x36, 0x36, 0xF6, 0x06, 0xF6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0xFE, 0x06, 0xF6, 0x36, 0x36, 0x36, 0x36, 0x36, 0xF6, 0x06, 0xFE, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0xFE, 0x00, 0x00, 0x00, 0x18, 0x18, 0xF8, 0x18, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36, 0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x30, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x30, 0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0xF7, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x36, 0x36, 0xF7, 0x00, 0xF7, 0x36, 0x36, 0x36, 0x18, 0x18, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x3F, 0x00, 0x00, 0x00, 0x18, 0x18, 0x1F, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xFF, 0x36, 0x36, 0x36, 0x18, 0x18, 0xFF, 0x18, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x18, 0x18, 0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xDC, 0xC8, 0xDC, 0x76, 0x00, 0x00, 0x78, 0xCC, 0xF8, 0xCC, 0xF8, 0xC0, 0xC0, 0x00, 0xFC, 0xCC, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0xFE, 0x6C, 0x6C, 0x6C, 0x6C, 0x6C, 0x00, 0xFC, 0xCC, 0x60, 0x30, 0x60, 0xCC, 0xFC, 0x00, 0x00, 0x00, 0x7E, 0xD8, 0xD8, 0xD8, 0x70, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x60, 0xC0, 0x00, 0x76, 0xDC, 0x18, 0x18, 0x18, 0x18, 0x00, 0xFC, 0x30, 0x78, 0xCC, 0xCC, 0x78, 0x30, 0xFC, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x6C, 0x38, 0x00, 0x38, 0x6C, 0xC6, 0xC6, 0x6C, 0x6C, 0xEE, 0x00, 0x1C, 0x30, 0x18, 0x7C, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0x7E, 0xDB, 0xDB, 0x7E, 0x00, 0x00, 0x06, 0x0C, 0x7E, 0xDB, 0xDB, 0x7E, 0x60, 0xC0, 0x38, 0x60, 0xC0, 0xF8, 0xC0, 0x60, 0x38, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, 0x00, 0xFC, 0x00, 0xFC, 0x00, 0xFC, 0x00, 0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0xFC, 0x00, 0x60, 0x30, 0x18, 0x30, 0x60, 0x00, 0xFC, 0x00, 0x18, 0x30, 0x60, 0x30, 0x18, 0x00, 0xFC, 0x00, 0x0E, 0x1B, 0x1B, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xD8, 0xD8, 0x70, 0x30, 0x30, 0x00, 0xFC, 0x00, 0x30, 0x30, 0x00, 0x00, 0x76, 0xDC, 0x00, 0x76, 0xDC, 0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x0F, 0x0C, 0x0C, 0x0C, 0xEC, 0x6C, 0x3C, 0x1C, 0x78, 0x6C, 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x70, 0x18, 0x30, 0x60, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static int term_x = 0;
static int term_y = 0;
static uint32_t fg = 0xFFFFFF;
#include <inttypes.h>

void set_term(void *address, int w, int h, int bpp) {
        Arc_BootMeta.term.base = (uint32_t)address;
        Arc_BootMeta.term.width = w;
        Arc_BootMeta.term.height = h;
        Arc_BootMeta.term.bpp = bpp;
        Arc_BootMeta.term.char_rom = (uint32_t)&character_rom;
}

void term_putchar(char c) {
#ifdef ARC_COM_PORT
        uint8_t lsr = inb(ARC_COM_PORT + 5);
        while (MASKED_READ(lsr, 5, 1) == 0) {
                lsr = inb(ARC_COM_PORT + 5);
        }

        outb(ARC_COM_PORT, c);
#endif

        if (Arc_BootMeta.term.base == 0) {
                return;
        }

        if (term_x >= Arc_BootMeta.term.width / 8) {
                term_y++;
                term_x = 0;
        }

        if (term_y >= Arc_BootMeta.term.height / 8 - 5) {
                term_y = 0;
                term_x = 0;

		memset((void *)Arc_BootMeta.term.base, 0, Arc_BootMeta.term.width * Arc_BootMeta.term.height * (Arc_BootMeta.term.bpp / 8));
        }

        int sx = term_x * 8;
        int sy = term_y * 8;

        switch (c) {
        case '\n': {
              term_x = 0;
              term_y++;
              break;
        }

        case '\t': {
            term_x += 8;
            break;
        }

        case 0: {
            break;
        }

        default: {

                for (int i = 0; i < 8; i++) {
                        int rx = 0;
                        for (int j = 8 - 1; j >= 0; j--) {
                                if (((character_rom[c * 8 + i] >> j) & 1) == 1 && c != 0) {
                                        ARC_FB_DRAW((void *)Arc_BootMeta.term.base, (sx + rx), (i + sy) * Arc_BootMeta.term.width, Arc_BootMeta.term.bpp, fg);
                                }
                                rx++;
                        }
                }
        
                term_x++;
        
                break;
        }
        }
}

void term_set_fg(uint32_t color) {
        fg = color;
}

void init_uart() {
	uint8_t lcr = inb(ARC_COM_PORT + 3);

	// No parity
	MASKED_WRITE(lcr, 0, 3, 0b111);
	// 1 stop bit
	MASKED_WRITE(lcr, 0, 2, 0b1);
	// 8 data bits
	MASKED_WRITE(lcr, 3, 0, 0b11);

	// Set divisor to 1 for fastest communications
	uint16_t divisor = 1;
	uint8_t dlab = inb(ARC_COM_PORT + 3);
	MASKED_WRITE(dlab, 1, 7, 1);
	outb(ARC_COM_PORT + 3, dlab);

	outb(ARC_COM_PORT, divisor & 0xFF);
	outb(ARC_COM_PORT + 1, (divisor >> 8) & 0xFF);

	MASKED_WRITE(dlab, 0, 7, 1);
	outb(ARC_COM_PORT + 3, dlab);

        uint8_t mcr = inb(ARC_COM_PORT + 4);
        MASKED_WRITE(mcr, 0, 4, 1);
        outb(ARC_COM_PORT + 4, mcr);
}