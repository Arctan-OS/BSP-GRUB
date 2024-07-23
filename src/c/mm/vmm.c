/**
 * @file vmm.c
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
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <global.h>
#include <cpuid.h>
#include <arch/x86/ctrl_regs.h>
#include <inttypes.h>

#define ADDRESS_MASK 0x000FFFFFFFFFF000

// Bitmap of supported features, the flags parameter
// of Arc_MapPageVMM, for instance are logically ANDed
// with this. If a particular feature is not supported, that
// flag will be turned off, ensuring that code complies with
// hardware
uint32_t feature_set = 0;

uint64_t *pml4 = NULL;

int vmm_map(uint64_t physical, uint64_t virtual, uint32_t flags) {
	int offset = virtual & 0xFFF; // Offset into the page
	int pt_e = (virtual >> 12) & 0x1FF; // Index of the page in the page table
	int pd_e = (virtual >> 21) & 0x1FF; // Index of the page table in the page directory
	int pdp_e = (virtual >> 30) & 0x1FF; // Index of the page directory in the page directory pointer table
	int pml4_e = (virtual >> 39) & 0x1FF; // Index of the page directory pointer table in the PML4

	uint64_t *pdp = (uint64_t *)(pml4[pml4_e] & ADDRESS_MASK);
	if ((pml4[pml4_e] & 0b11) == 0) {
		// No such entry
		pdp = Arc_AllocPMM();

		if (pdp == NULL) {
			ARC_DEBUG(ERR, "Failed to allocate page directory pointer for 0x%"PRIx64" -> 0x%"PRIx64"\n", physical, virtual);
			ARC_HANG;
		}

		memset(pdp, 0, PAGE_SIZE);
		pml4[pml4_e] = (((uint64_t)pdp) & ADDRESS_MASK) | 0b11;
	}

	uint64_t *pd = (uint64_t *)(pdp[pdp_e] & ADDRESS_MASK);
	if ((pdp[pdp_e] & 0b11) == 0) {
		pd = Arc_AllocPMM();

		if (pd == NULL) {
			ARC_DEBUG(ERR, "Failed to allocate page directory for 0x%"PRIx64" -> 0x%"PRIx64"\n", physical, virtual);
			ARC_HANG;
		}

		memset(pd, 0, PAGE_SIZE);
		pdp[pdp_e] = (((uint64_t)pd) & ADDRESS_MASK) | 0b11;

		if ((flags & ARC_VMM_NO_EXEC) != 0) {
			// Set NX bit
			pdp[pdp_e] |= (uint64_t)1 << 63;
		}
	
		if ((flags & ARC_VMM_1GIB) != 0) {
			// Set PS to 1 to enable 1 GiB pages
			pdp[pdp_e] |= 1 << 7;

			return 0;
		}
	}

	uint64_t *pt = (uint64_t *)(pd[pd_e] & ADDRESS_MASK);
	if ((pd[pd_e] & 0b11) == 0) {
		pt = Arc_AllocPMM();

		if (pt == NULL) {
			ARC_DEBUG(ERR, "Failed to allocate page table for 0x%"PRIx64" -> 0x%"PRIx64"\n", physical, virtual);
			ARC_HANG;
		}

		memset(pt, 0, PAGE_SIZE);
		pd[pd_e] = (((uint64_t)pt) & ADDRESS_MASK) | 0b11;

		if ((flags & ARC_VMM_NO_EXEC) != 0) {
			// Set NX bit
			pd[pd_e] |= (uint64_t)1 << 63;
		}

		if ((flags & ARC_VMM_2MIB) != 0) {
			pd[pd_e] |= 1 << 7;
			return 0;
		}
	}

	uint64_t *page = (uint64_t *)(pd[pd_e] & ADDRESS_MASK);
	if ((pt[pt_e] & 0b11) == 0) {
		pt[pt_e] = (physical & ADDRESS_MASK) | 0b11;

		if ((flags & ARC_VMM_NO_EXEC) != 0) {
			pt[pt_e] |= (uint64_t)1 << 63;
		}
	}

	return 0;
}

int init_vmm() {
	ARC_DEBUG(INFO, "Initializing VMM\n");

	pml4 = Arc_AllocPMM();

	if (pml4 == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate a page for PML4\n");
		return -1;
	}

	memset(pml4, 0, PAGE_SIZE);

	uint32_t eax, ebx, ecx, edx;
	__cpuid(0x80000001, eax, ebx, ecx, edx);

	// Check for 1 GiB pages
	if (((edx >> 26) & 1) == 1) {
		ARC_DEBUG(INFO, "1 GiB pages supported\n");
		feature_set |= ARC_VMM_1GIB;
	}

	// Check for NX
 	if (((edx >> 20) & 1) == 1) {
		ARC_DEBUG(INFO, "NX bit supported\n");
		feature_set |= ARC_VMM_NO_EXEC;
		// Enable in IA32_EFER
		uint64_t efer = _x86_RDMSR(0xC0000080);
		efer |= 1 << 11;
		_x86_WRMSR(0xC0000080, efer);
	}

	_boot_meta.paging_features = feature_set;

	ARC_DEBUG(INFO, "Initialized VMM\n");

	return 0;
}
