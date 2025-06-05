/**
 * @file watermark.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/BSP-GRUB - GRUB bootstrapper for Arctan-OS/Kernel
 * Copyright (C) 2025 awewsomegamer
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
 * Header file defining functions to manage and use a basic watermark allocator
*/
#include <mm/watermark.h>
#include <global.h>
#include <inttypes.h>

static int current_entry = -1;
static size_t watermark_off = 0;

static int watermark_get_new_base() {
        struct ARC_MMap *mmap = (struct ARC_MMap *)Arc_KernelMeta.arc_mmap.base;

        int largest = -1;
        for (int i = 0; i < Arc_KernelMeta.arc_mmap.len; i++) {
                if (mmap[i].type != ARC_MEMORY_AVAILABLE || 
                        mmap[i].base < 0x100000 || mmap[i].base >= UINT32_MAX) {
                        continue;
                }
                
                if (largest == - 1 || mmap[largest].len < mmap[i].len) {
                        largest = i;
                }
        }
                
        
        if (current_entry != -1) {
                mmap[current_entry].type = ARC_MEMORY_BOOTSTRAP_ALLOC;
        }

        if (largest == -1) {
                return -1;
        }

        current_entry = largest;
        watermark_off = 0;

        return 0;
}

int watermark_update_mmap() {
        if (current_entry == -1) {
                return watermark_get_new_base();
        }

        struct ARC_MMap *mmap = (struct ARC_MMap *)Arc_KernelMeta.arc_mmap.base;

        nmemcpy(&mmap[current_entry + 1], &mmap[current_entry], sizeof(struct ARC_MMap) * (Arc_KernelMeta.arc_mmap.len - 1));
        Arc_KernelMeta.arc_mmap.len++;

        mmap[current_entry + 1].base += watermark_off;
        mmap[current_entry + 1].len -= watermark_off;

        mmap[current_entry].len = watermark_off;
        mmap[current_entry].type = ARC_MEMORY_BOOTSTRAP_ALLOC;

        return 0;
}

void *alloc(size_t size) {
        struct ARC_MMap *mmap = (struct ARC_MMap *)Arc_KernelMeta.arc_mmap.base;

        size = ALIGN(size, PAGE_SIZE);

        if (watermark_off + size > mmap[current_entry].base + mmap[current_entry].len
                && watermark_get_new_base() != 0) {
                ARC_DEBUG(ERR, "Overflow, can't rebase\n");
                return NULL;
        }

        void *a = (void *)(mmap[current_entry].base + watermark_off);
        watermark_off += size;

        return a;
}