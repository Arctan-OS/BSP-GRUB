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
#include "mm/bank.h"
#include <mm/freelist.h>
#include <arctan.h>
#include <mm/pmm.h>
#include <global.h>
#include <stdint.h>
#include <inttypes.h>

static struct ARC_BankMeta pmm_bank_low = { 0 };
static struct ARC_BankMeta pmm_bank_high = { 0 };

void *Arc_AllocPMM() {
	struct ARC_BankNode *node = pmm_bank_high.first;
	void *a = NULL;
	while (node != NULL && a == NULL) {
		a = freelist_alloc(node->allocator_meta);
		node = node->next;
	}

	return a;
}

void *Arc_FreePMM(void *address) {
	struct ARC_BankNode *node = pmm_bank_high.first;
	void *a = NULL;
	while (node != NULL && a != address) {
		a = freelist_free(node->allocator_meta, address);
		node = node->next;
	}

	return a;	
}

void *Arc_ContiguousAllocPMM(int objects) {
	struct ARC_BankNode *node = pmm_bank_high.first;
	void *a = NULL;
	while (node != NULL && a == NULL) {
		a = freelist_contig_alloc(node->allocator_meta, objects);
		node = node->next;
	}

	return a;
}

void *Arc_ContiguousFreePMM(void *address, int objects) {
	struct ARC_BankNode *node = pmm_bank_high.first;
	void *a = NULL;
	while (node != NULL && a != address) {
		a = freelist_contig_free(node->allocator_meta, address, objects);
		node = node->next;
	}

	return a;
}

int init_pmm() {
	struct ARC_MMap *mmap = (struct ARC_MMap *)((uintptr_t)_boot_meta.arc_mmap);

	if (mmap == NULL || _boot_meta.arc_mmap_len <= 0) {
		ARC_DEBUG(ERR, "MMap (%p) is NULL or contains 0 (%d) entries, failed to initialize PMM\n", mmap, _boot_meta.arc_mmap_len);
		return -1;
	}

	init_static_bank(&pmm_bank_low, ARC_MM_BANK_TYPE_PFREELIST);
	init_static_bank(&pmm_bank_high, ARC_MM_BANK_TYPE_PFREELIST);

	ARC_DEBUG(INFO, "Initializing base freelist page frame allocator (MMap: %p)\n", mmap);

	uint64_t total_free = 0;

	ARC_DEBUG(INFO, "Creating low allocator:\n");

	// Initialize low memory
	for (int i = 0; i < _boot_meta.arc_mmap_len; i++) {
		if (mmap[i].type != ARC_MEMORY_BOOTSTRAP || mmap[i].base >= 0x100000) {
			continue;
		}

		// Round up
		uint64_t base = ALIGN(mmap[i].base, PAGE_SIZE);
		// Round down
		uint64_t ceil = ((mmap[i].base + mmap[i].len) >> 12) << 12;

		if (base == 0) {
			base += PAGE_SIZE;
		}

		ARC_DEBUG(INFO, "\tFound available entry %d (0x%"PRIx64" -> 0x%"PRIx64"), initializing freelist\n", i, base, ceil);

		struct ARC_FreelistMeta *list = init_freelist(base + _boot_meta.hhdm_vaddr, ceil + _boot_meta.hhdm_vaddr, PAGE_SIZE);
		bank_add_pfreelist(&pmm_bank_low, list);

		total_free += mmap[i].len;
	}

	_boot_meta.pmm_low_bank = (uintptr_t)&pmm_bank_low;

	ARC_DEBUG(INFO, "Successfully initialized low PMM, 0x%"PRIx64" bytes free\n", total_free);

	ARC_DEBUG(INFO, "Creating high allocator:\n");
	for (int i = 0; i < _boot_meta.arc_mmap_len; i++) {
		if (mmap[i].type != ARC_MEMORY_AVAILABLE) {
			continue;
		}

		if (((mmap[i].base >> 32) & UINT32_MAX) > 0) {
			ARC_DEBUG(WARN, "Entry %d is outside of 32-bit address space, cannot initialize into freelist\n", i);
			continue;
		}

		// Round up
		uint64_t base = ALIGN(mmap[i].base, PAGE_SIZE);
		// Round down
		uint64_t ceil = ((mmap[i].base + mmap[i].len) >> 12) << 12;

		ARC_DEBUG(INFO, "\tFound available entry %d (0x%"PRIx64" -> 0x%"PRIx64"), initializing a freelist\n", i, base, ceil);

		// hhdm_vaddr can be added here as it only effects the upper 32-bits
		// of each 64-bit value. This saves the kernel the step of converting
		// the freelists that are initialized here to use HHDM addresses and
		// still let's this code use the list as only the lower 32-bits are used
		// by it
		void *list = init_freelist(base + _boot_meta.hhdm_vaddr, ceil + _boot_meta.hhdm_vaddr, PAGE_SIZE);
		bank_add_pfreelist(&pmm_bank_high, list);
		
		total_free += mmap[i].len;
	}

	_boot_meta.pmm_high_bank = (uintptr_t)&pmm_bank_high;

	ARC_DEBUG(INFO, "Successfully initialized high PMM, 0x%"PRIx64" bytes free (high + low)\n", total_free);

	return 0;
}
