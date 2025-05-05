/**
 * @file global.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-MB2BSP - Multiboot2 Bootstrapper for Arctan Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan-MB2BSP
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
#ifndef ARC_GLOBAL_H
#define ARC_GLOBAL_H

#include <arctan.h>
#include <util.h>

#define ARC_HANG for (;;) __asm__("hlt");

#define ASSERT(cond) if (!(cond)) {					\
			printf("Assertion %s failed (%s:%d)\n", #cond, __FILE__, __LINE__); \
			for (;;); \
		     }
#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define max(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

#define abs(a)					\
	(a < 0 ? -a : a)


#define MASKED_READ(__value, __shift, __mask) (((__value) >> (__shift)) & (__mask))
#define MASKED_WRITE(__to, __value, __shift, __mask) __to = (((__to) & ~((__mask) << (__shift))) | (((__value) & (__mask)) << (__shift)));

#define PAGE_SIZE 0x1000

extern struct ARC_BootMeta _boot_meta;
extern uint8_t __BOOTSTRAP_START__;
extern uint8_t __BOOTSTRAP_END__;

extern uint64_t bsp_image_base;
extern uint64_t bsp_image_ceil;

extern void *term_address;
extern int term_w;
extern int term_h;
extern int term_bpp;

#endif
