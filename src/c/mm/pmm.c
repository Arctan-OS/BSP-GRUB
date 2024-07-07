/**
 * @file freelist.c
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
 * Abstract freelist implementation.
*/
#include <mm/freelist.h>
#include <arctan.h>
#include <mm/pmm.h>
#include <global.h>
#include <stdint.h>

struct ARC_FreelistMeta *pmm_meta = NULL;

void *Arc_AllocPMM() {
	return Arc_ListAlloc(pmm_meta);
}

void *Arc_FreePMM(void *address) {
	return Arc_ListFree(pmm_meta, address);
}

void *Arc_ContiguousAllocPMM(int objects) {
	return Arc_ListContiguousAlloc(pmm_meta, objects);
}

void *Arc_ContiguousFreePMM(void *address, int objects) {
	return Arc_ListContiguousFree(pmm_meta, address, objects);
}

int Arc_InitPMM() {
	struct ARC_MMap *mmap = (struct ARC_MMap *)_boot_meta.arc_mmap;

	if (mmap == NULL || _boot_meta.arc_mmap_len <= 0) {
		ARC_DEBUG(ERR, "MMap is NULL or contains 0 entries, failed to initialize PMM\n");
		return -1;
	}

	ARC_DEBUG(INFO, "Initializing base freelist page frame allocator (MMap: %p)\n", mmap);

	for (int i = 0; i < _boot_meta.arc_mmap_len; i++) {
		if (mmap[i].type != ARC_MEMORY_AVAILABLE) {
			continue;
		}

		if (((mmap[i].base >> 32) & UINT32_MAX) > 0) {
			ARC_DEBUG(WARN, "Entry %d is outside of 32-bit address space, cannot initialize into freelist\n", i);
			continue;
		}

		void *base = (void *)mmap[i].base;
		void *ciel = (void *)(mmap[i].base + mmap[i].len);

		ARC_DEBUG(INFO, "\tFound available entry %d, initializing a freelist\n", i);

		void *list = Arc_InitializeFreelist(base, ciel, PAGE_SIZE);

		int ret = 0;

		if (pmm_meta == NULL) {
			// Create primary list
			ARC_DEBUG(INFO, "Established primary list\n");
			pmm_meta = list;
		} else {
			ARC_DEBUG(INFO, "Linking newly made list into primary\n")
			// Link lists
			ret = Arc_ListLink(pmm_meta, list);
		}

		if (ret != 0) {
			ARC_DEBUG(INFO, "Failed to link lists (%p, %p), code: %d\n", pmm_meta, list, ret);
		}
	}

	_boot_meta.pmm_state = (uint64_t)pmm_meta;

	ARC_DEBUG(INFO, "Successfully initialized PMM\n");

	return 0;
}
