/**
 * @file pager.c
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
#if defined(ARC_TARGET_ARCH_X86_64) || defined(ARC_TARGET_ARCH_X86)

#include <arctan.h>
#include <arch/pager.h>
#include <arch/x86/ctrl_regs.h>
#include <global.h>
#include <util.h>
#include <stdbool.h>
#include <inttypes.h>
#include <mm/watermark.h>

void *pmm_alloc_page() {
	return alloc(PAGE_SIZE);
}

size_t pmm_free_page(void *addr) {
	return 1;
}


// NOTE: The pager does not overwrite the priveleges of a directory table.
//       So if a directory table is kernel only, and a userspace page is mapped
//       into it, the page will not be accessible by userspace as it is within
//       a privelleged table.

// BUG:  When the clone function is called with ARC_PAGER_OVW set, it will just fail.
//       Find out if other functions do this too, or if it just this one and fix it.

#define ADDRESS_MASK 0x000FFFFFFFFFF000

#define ONE_GIB 0x40000000
#define TWO_MIB 0x200000

struct pager_traverse_info {
	uint64_t *src_table; // Source page tables
	uint64_t *dest_table; // Destination page tables
	uint64_t *cur_table; // Currently loaded page tables
	uint64_t virtual;
	uint64_t physical;
	uint64_t size;
	uint32_t attributes;
	uint32_t pml4e;
	uint32_t pml3e;
	uint32_t pml2e;
	uint32_t pml1e;
};

uint64_t Arc_KernelPageTables = 0;

uint64_t get_entry_bits(uint32_t level, uint32_t attributes) {
	// Level 0: Page
	// Level 1: PT
	// Level 2: PD
	// Level 3: PDP
	// Level 4: PML4

	uint64_t bits = 0;
	
	bits |= ((attributes >> (ARC_PAGER_PAT)) & 1) << 2; // PWT
	bits |= ((attributes >> (ARC_PAGER_PAT + 1)) & 1) << 3; // PCD

	int us_rw_overwrite = (level > MASKED_READ(attributes, ARC_PAGER_AUTO_USRW_DISABLE, 0xF) + 1);

	switch (level) {
		case 1: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 7;

			break;
		}

		case 2: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 12;
			// 2MiB pages unless 4K specified
			if (((attributes >> ARC_PAGER_RESV1) & 1) == 1) {
				us_rw_overwrite = 0;
				bits |= 1 << 7;
			}

			break;
		}

		case 3: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 12;
			// 1GiB pages unless 4K specified
 			if (((attributes >> ARC_PAGER_RESV0) & 1) == 1) {
				us_rw_overwrite = 0;
				bits |= 1 << 7;
			}

			break;
		}

		// On 4, 5, bit 7 is reserved
	}

	// NOTE: The logic of us_rw_overwrite is such that all page directories
	//       (PT, PD, PDP, ...) have write enabled on them and are user accessible
	//       this allows for more granular mapping such that a fault isn't given
	//       on writeable page that just so happens to be located in a read only
	//       page table. This may prove to be a little less efficient when it comes
	//       to looking things up
	bits |= (((attributes >> ARC_PAGER_US) & 1) | us_rw_overwrite) << 2;
	bits |= (((attributes >> ARC_PAGER_RW) & 1) | us_rw_overwrite) << 1;
	bits |= (uint64_t)((attributes >> ARC_PAGER_NX) & ((Arc_KernelMeta.paging_features >> ARC_PAGER_FLAG_NO_EXEC) & 1)) << 63;
	bits |= 1; // Present

	return bits;
}

/**
 * Get the next page table.
 *
 * @param uint64_t *parent - The parent table.
 * @param int level - The level of the parent table (4 = PML4).
 * @param uint64_t virtual - The base virtual address.
 * @param uint32_t attributes - Specified attributes.
 * @parma int *index - The 64-bit entry in the parent corresponding to the returned table.
 * @return a non-NULL pointer to the next page table
 * */
static int get_page_table(uint64_t *parent, int level, uint64_t virtual, uint32_t attributes) {
	if (parent == NULL) {
		ARC_DEBUG(ERR, "NULL parent\n");
		return -1;
	}

	int shift = ((level - 1) * 9) + 12;
	int index = (virtual >> shift) & 0x1FF;
	uint64_t entry = parent[index];
	
	uint64_t *address = (uint64_t *)(entry & 0x0000FFFFFFFFF000);
	
	/*
		Level | Decsription
		4     | Parent: PML4, Entry: "PML4E" (no large pages here, always create new PML3)
		3     | Parent: PML3, Entry: PDPTE, "PML3E" (1 GiB page entries possible here)
		2     | Parent: PML2, Entry: PDE, "PML2E" (2 MiB page entries possible here)
		1     | Parent: PML1, Entry: PTE, "PML1E" (regular 4 KiB page entries here)
		0     | Page (can't really do anything with this level here)

		In the event that ARC_PAGER_4K is set, always create new lower page tables
	*/

	bool present = entry & 1;
	bool only_4k = MASKED_READ(attributes, ARC_PAGER_4K, 1);
	bool can_gib = MASKED_READ(attributes, ARC_PAGER_RESV0, 1);
	bool can_mib = MASKED_READ(attributes, ARC_PAGER_RESV1, 1);
	bool no_create = MASKED_READ(attributes, ARC_PAGER_RESV2, 1);

	if (!no_create && !present && level != 1 && ((level == 4) || (level == 3 && !can_gib) || (level == 2 && !can_mib) || only_4k)) {
		// Only make a new table if:
		//     The current entry is not present AND:
		//         - Mapping is only 4K, or
		//         - Can't make a GiB page, or
		//         - Can't make a MiB page, or
		//         - Parent is a level 2 page table
		//     AND creation of page tables is allowed
		//     AND not on page level
		
		address = (uint64_t *)pmm_alloc_page();
		
		if (address == NULL) {
			ARC_DEBUG(ERR, "Can't alloc\n");
			return -1;
		}

		memset(address, 0, PAGE_SIZE);
		parent[index] = (uint64_t)address | get_entry_bits(level, attributes);
	}

	return index;
}

/**
 * Standard function to traverse x86-64 page tables
 *
 * @param struct pager_traverse_info *info - Information to use for traversing and to pass to the callback.
 * @param int *(callback)(...) - The callback function.
 * @return zero on success.
 * */
static int pager_traverse(struct pager_traverse_info *info, int (*callback)(struct pager_traverse_info *info, uint64_t *table, int index, int level)) {
	if (info == NULL || info->dest_table == NULL) {
		return -1;
	}

	while (info->size) {
		uint8_t can_gib = ((Arc_KernelMeta.paging_features >> ARC_PAGER_FLAG_1_GIB) & 1) 
				   && (MASKED_READ(info->attributes, ARC_PAGER_4K, 1) == 0) && (info->size >= ONE_GIB); 
		uint8_t can_2mib = (info->size >= TWO_MIB) 
				   && (MASKED_READ(info->attributes, ARC_PAGER_4K, 1) == 0);

		MASKED_WRITE(info->attributes, can_gib, ARC_PAGER_RESV0, 1);
		MASKED_WRITE(info->attributes, can_2mib, ARC_PAGER_RESV1, 1);

		uint64_t *table = info->dest_table; // PML4
		int index = get_page_table(table, 4, info->virtual, info->attributes); // index in PML4
		
		if (index == -1) {
			return -2;
		}

		info->pml4e = index;

		table = (uint64_t *)(table[index] & ADDRESS_MASK); // PML4[index] -> PML3		
		index = get_page_table(table, 3, info->virtual, info->attributes); // index in PML3
		
		if (index == -1) {
			return -3;
		}

		info->pml3e = index;

		if (can_gib) {
			// Map 1 GiB page
			if (callback(info, table, index, 3) != 0) {
				return -4;
			}

			info->virtual += ONE_GIB;
			info->physical += ONE_GIB;	
			info->size -= ONE_GIB;

			continue;
		}

		table = (uint64_t *)(table[index] & ADDRESS_MASK);
		index = get_page_table(table, 2, info->virtual, info->attributes);

		if (index == -1) {
			return -5;
		}

		info->pml2e = index;

		if (can_2mib) {
			// Map 2 MiB page
			if (callback(info, table, index, 2) != 0) {
				return -6;
			}

			info->virtual += TWO_MIB;
			info->physical += TWO_MIB;
			info->size -= TWO_MIB;

			continue;
		}

		table = (uint64_t *)(table[index] & ADDRESS_MASK);
		index = get_page_table(table, 1, info->virtual, info->attributes);

		if (index == -1) {
			return -7;
		}

		info->pml1e = index;

		// Map 4K page
		if (callback(info, table, index, 1) != 0) {
			return -8;
		}

		info->virtual += PAGE_SIZE;
		info->physical += PAGE_SIZE;
		info->size -= PAGE_SIZE;
	}

	return 0;
}

void *pager_create_page_tables() {
	void *tables = pmm_alloc_page();

	if (tables == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate page tables\n");
		return NULL;
	}

	memset(tables, 0, PAGE_SIZE);

	return tables;
}

static int pager_map_callback(struct pager_traverse_info *info, uint64_t *table, int index, int level) {
	if (info == NULL || table == NULL || level == 0) {
		return -1;
	}
	
	table[index] = info->physical | get_entry_bits(level, info->attributes);

	//printf("Level %d Index %d Perms %"PRIx64"\n", level, index, table[index]);

	if (info->dest_table == info->cur_table) {
		__asm__("invlpg %0" : : "m"(info->virtual) : );
	}

	return 0;
}

int pager_map(void *page_tables, uint64_t virtual, uint64_t physical, uint64_t size, uint32_t attributes) {
	void *pml4 = (void *)(_x86_getCR3());
	struct pager_traverse_info info = { .virtual = virtual, .physical = physical,
	                                    .size = size, .attributes = attributes,
					    .dest_table = page_tables == NULL ? pml4 : page_tables,
					    .cur_table = pml4 };

	ARC_DEBUG(INFO, "Mapping %"PRIx64" -> %"PRIx64" for %"PRIx64" bytes\n", virtual, physical, size);

	int r = pager_traverse(&info, pager_map_callback);
	if (r != 0) {
		ARC_DEBUG(ERR, "Failed to map P0x%"PRIx64":V0x%"PRIx64" (0x%"PRIx64" B, 0x%x) %d\n", physical, virtual, size, attributes, r);
		return -1;
	}

	return 0;
}

#endif
