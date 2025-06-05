/**
 * @file terminal.h
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
#ifndef ARC_INTERFACE_TERMINAL_H
#define ARC_INTERFACE_TERMINAL_H

#include <stdint.h>

typedef struct {
	uint32_t data : 24;
}__attribute__((packed)) uint24_s;

#define ARC_FB_DRAW(__addr, __x, __y, __bpp, __color)			\
	switch (__bpp) {						\
		case 32:						\
			*((uint32_t *)__addr + __x + __y) = __color;	\
			break;						\
		case 24:						\
			((uint24_s *)__addr + __x + __y)->data = __color; \
			break;						\
		case 16:						\
			*((uint16_t *)__addr + __x + __y) = __color;	\
			break;						\
	}

void set_term(void *address, int w, int h, int bpp);
void term_putchar(char c);
void term_set_fg(uint32_t color);
void init_uart();

#endif
