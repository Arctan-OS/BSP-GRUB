/**
 * @file pmm.c
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
#include <inttypes.h>

struct ARC_FreelistMeta *pmm_meta = NULL;

void *Arc_AllocPMM() {
	return freelist_alloc(pmm_meta);
}

void *Arc_FreePMM(void *address) {
	return freelist_free(pmm_meta, address);
}

void *Arc_ContiguousAllocPMM(int objects) {
	return freelist_contig_alloc(pmm_meta, objects);
}

void *Arc_ContiguousFreePMM(void *address, int objects) {
	return freelist_contig_free(pmm_meta, address, objects);
}

int init_pmm() {
	struct ARC_MMap *mmap = (struct ARC_MMap *)_boot_meta.arc_mmap;

	if (mmap == NULL || _boot_meta.arc_mmap_len <= 0) {
		ARC_DEBUG(ERR, "MMap (%p) is NULL or contains 0 (%d) entries, failed to initialize PMM\n", mmap, _boot_meta.arc_mmap_len);
		return -1;
	}

	ARC_DEBUG(INFO, "Initializing base freelist page frame allocator (MMap: %p)\n", mmap);

	uint64_t total_free = 0;

	for (int i = 0; i < _boot_meta.arc_mmap_len; i++) {
		if (mmap[i].type != ARC_MEMORY_AVAILABLE) {
			continue;
		}

		if (((mmap[i].base >> 32) & UINT32_MAX) > 0) {
			ARC_DEBUG(WARN, "Entry %d is outside of 32-bit address space, cannot initialize into freelist\n", i);
			continue;
		}

		uint64_t base = mmap[i].base;
		uint64_t ceil = mmap[i].base + mmap[i].len;

		ARC_DEBUG(INFO, "\tFound available entry %d (0x%"PRIx64" -> 0x%"PRIx64"), initializing a freelist\n", i, base, ceil);

		// hhdm_vaddr can be added here as it only effects the upper 32-bits
		// of each 64-bit value. This saves the kernel the step of converting
		// the freelists that are initialized here to use HHDM addresses and
		// still let's this code use the list as only the lower 32-bits are used
		// by it
		void *list = init_freelist(base + _boot_meta.hhdm_vaddr, ceil + _boot_meta.hhdm_vaddr, PAGE_SIZE);

		int ret = 0;

		if (pmm_meta == NULL) {
			// Create primary list
			ARC_DEBUG(INFO, "\t\tEstablished primary list\n");
			pmm_meta = list;
		} else {
			ARC_DEBUG(INFO, "\t\tLinking newly made list into primary\n")
			// Link lists
			ret = link_freelists(pmm_meta, list);
		}

		if (ret != 0) {
			ARC_DEBUG(ERR, "\t\tFailed to link lists (%p, %p), code: %d\n", pmm_meta, list, ret);
		}

		total_free += mmap[i].len;
	}

	_boot_meta.pmm_state = (uint64_t)pmm_meta;

	ARC_DEBUG(INFO, "Successfully initialized PMM, 0x%"PRIx64" bytes free\n", total_free);

	return 0;
}
