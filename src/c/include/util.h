/**
 * @file util.h
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
 * File containing utility functions / macros.
*/
#ifndef ARC_UTIL_H
#define ARC_UTIL_H

#include <stddef.h>
#include <stdint.h>

#include <interface/printf.h>
#include <interface/terminal.h>

#define ARC_DEBUG_STRINGIFY(val) #val
#define ARC_DEBUG_TOSTRING(val) ARC_DEBUG_STRINGIFY(val)
#define ARC_DEBUG_NAME_STR "[BOOTSTRAP "__FILE__":"ARC_DEBUG_TOSTRING(__LINE__)"]"
#define ARC_DEBUG_NAME_SEP_STR " : "
#define ARC_DEBUG_INFO_STR "[INFO]"
#define ARC_DEBUG_WARN_STR "[WARNING]"
#define ARC_DEBUG_ERR_STR  "[ERROR]"

#define ARC_DEBUG(__level__, ...) ARC_DEBUG_##__level__(__VA_ARGS__)
#define ARC_DEBUG_ERR(...)  term_set_fg(0x00FF0000); printf(ARC_DEBUG_ERR_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);

#ifdef ARC_DEBUG_ENABLE
#define ARC_DEBUG_INFO_STR "[INFO]"
#define ARC_DEBUG_WARN_STR "[WARNING]"
#define ARC_DEBUG_INFO(...) term_set_fg(0xFFFFFFFF); printf(ARC_DEBUG_INFO_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);
#define ARC_DEBUG_WARN(...) term_set_fg(0x00FFFF00); printf(ARC_DEBUG_WARN_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);
#else
#define ARC_DEBUG_INFO(...) ;
#define ARC_DEBUG_WARN(...) ;
#endif // ARC_DEBUG_ENABLE

#if defined(ARC_TARGET_ARCH_X86_64) || defined(ARC_TARGET_ARCH_X86)
#define ARC_HANG __asm__("1: hlt; jmp 1b");
#endif


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

int strcmp(char *a, char *b);
int memcpy(void *a, void *b, size_t size);
int nmemcpy(void *a, void *b, size_t size);
void memset(void *mem, uint8_t value, size_t size);

#endif
