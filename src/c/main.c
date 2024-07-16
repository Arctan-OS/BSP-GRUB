/**
 * @file main.c
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
#include <stdint.h>
#include <global.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <multiboot/mbparse.h>
#include <inttypes.h>
#include <elf/elf.h>
#include <multiboot/multiboot2.h>
#include <mm/freelist.h>
#include <arch/x86/cpuid.h>

uint64_t kernel_entry = 0;
uint64_t bsp_image_base = 0;
uint64_t bsp_image_ceil = 0;

int helper(uint8_t *mb2i, uint32_t signature) {
	if (signature != MULTIBOOT2_BOOTLOADER_MAGIC) {
		ARC_DEBUG(ERR, "Not booted with Multiboot2!\n");
		ARC_HANG;
	}

	// Set the HHDM virtual address
	_boot_meta.hhdm_vaddr = 0xFFFFC00000000000;

	// Setup GDT
	Arc_InstallGDT();
	// Setup IDT
	Arc_InstallIDT();

	// Check requirements
	Arc_CheckCPUID();

	// Parse all multiboot2 boot information
	Arc_ParseMB2I(mb2i);

	// Initialize freelist page frame allocator
	//   Goal of this allocator is to make it so
	//   all page frames below UINT32_MAX are allocatable
	// TODO: Use a simpler, more coarse, allocator like a buddy
	//       allocator
	Arc_InitPMM();

	// Initialize VMM
	Arc_InitVMM();

	ARC_DEBUG(INFO, "Constructing HHDM at 0x%"PRIx64":\n", _boot_meta.hhdm_vaddr);

	struct ARC_MMap *mmap = (struct ARC_MMap *)_boot_meta.arc_mmap;

	// Put together HHDM so kernel can access all physical memory
	// later on
	// TODO: Try to detect large pages (2 MiB or 1 GiB)
	//       in order to save space within the allocator
	for (uint32_t i = 0; i < _boot_meta.arc_mmap_len; i++) {
		uint64_t phys_base = mmap[i].base;
		uint64_t virt_base = phys_base + _boot_meta.hhdm_vaddr;

		ARC_DEBUG(INFO, "\tMapping MMap entry %d 0x%"PRIx64" -> 0x%"PRIx64"\n", i, phys_base, virt_base)

		for (uint64_t page = 0; page < mmap[i].len; page += PAGE_SIZE) {
			if (Arc_MapPageVMM(phys_base + page, virt_base + page, ARC_VMM_NO_EXEC) != 0) {
				ARC_DEBUG(ERR, "\tFailed to construct HHDM!\n");
				ARC_HANG;
			}
		}
	}

	ARC_DEBUG(INFO, "Identity mapping bootstrapper:\n")

	// Map bootstrapper image into memory so a page fault is not immediately
	// thrown after enabling paging
	for (uint64_t phys = bsp_image_base; phys <= bsp_image_ceil; phys += PAGE_SIZE) {
		ARC_DEBUG(INFO, "\tIdentity mapping 4 KiB page: 0x%"PRIx64"\n", phys);

		if (Arc_MapPageVMM(phys, phys, 0) != 0) {
			ARC_DEBUG(ERR, "\tFailed to identity map!\n");
			ARC_HANG;
		}
	}

	// Parse the Kernel ELF file and map it into memory using 4 KiB pages
	// and set kernel_entry to the virtual address of where the kernel is
	kernel_entry = Arc_LoadELF((uint8_t *)_boot_meta.kernel_elf);

	uint64_t term_addr = ((uint64_t)term_address) & UINT32_MAX;

	for (uint64_t page = 0; page < term_w * term_h * (term_bpp / 4); page += PAGE_SIZE) {
		if (Arc_MapPageVMM(term_addr + page, term_addr + _boot_meta.hhdm_vaddr + page, ARC_VMM_NO_EXEC) != 0) {
			ARC_DEBUG(ERR, "\tFailed to map framebuffer\n");
		}
	}


	ARC_DEBUG(INFO, "Finished bootstrapping, returning to assembly to setup long mode and jump to kernel (0x%"PRIx64")\n", kernel_entry);

	// Jump back to the assembly phase, which will enable paging and put the
	// system into long mode before jumping to the kernel
	return 0;
}
