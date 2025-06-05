/**
 * @file bsp.c
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
#include <stdint.h>
#include <util.h>
#include <inttypes.h>
#include <boot/multiboot2.h>
#include <interface/terminal.h>
#include <global.h>
#include <arch/init.h>
#include <mm/watermark.h>
#include <arch/pager.h>
#include <elf.h>
#include <config.h>
#include <boot/mb2parse.h>
#include <arch/cpuid.h>

// The kernel expects:
// 	ARC_COM_PORT must be configured for 8N1 transmission at the highest baudrate (done)
//	Clean memory Arc_MMap
//		Bootstrapper must reserve portions of memory it occupies for kernel fallback
//		Bootstrapper must leave available portions of memory uninitialized
//	Bootstrapper must unpack kernel image (done)
//	Bootstrapper must create HHDM at virtual base ARC_HHDM_VADDR (done)
//	Bootstrapper must handle exceptions
//	Bootstrapper must parse boot tags and pass them to the kernel in appropriate fields (i.e. initramfs_base, initramfs_length, ...)
// 	Bootstrapper must prepare environment for the kernel

uint64_t kernel_entry = 0;
uint64_t pt_root = 0;

struct ARC_KernelMeta Arc_KernelMeta = { 0 };
struct ARC_BootMeta Arc_BootMeta = { 0 };

uint64_t bsp(uint8_t *mb2i, uint32_t signature) {
	init_uart();

	if (signature != MULTIBOOT2_BOOTLOADER_MAGIC) {
		ARC_DEBUG(ERR, "Not booted with Multiboot2!\n");
		ARC_HANG;
	}

	if (check_cpuid() != 0) {
		ARC_HANG;
	}

	if (parse_mb2i(mb2i) != 0) {
		ARC_DEBUG(ERR, "Failed to read MB2 tags\n");
		ARC_HANG;
	}

	watermark_update_mmap();

	// Setup architecture
	init_arch();

	pt_root = (uint64_t)pager_create_page_tables();
		
	if (pt_root == 0) {
		ARC_DEBUG(ERR, "Failed to initialize pml4\n");
		ARC_HANG;
	}

	ARC_DEBUG(INFO, "Constructing HHDM at 0x%"PRIx64"\n", ARC_HHDM_VADDR);
	
	// Put together HHDM so kernel can access all physical memory
	pager_map((void *)pt_root, ARC_HHDM_VADDR, 0, Arc_BootMeta.mem_size, 1 << ARC_PAGER_RW);

	ARC_DEBUG(INFO, "Identity mapping bootstrapper\n")

	// Map bootstrapper image into memory so a page fault is not immediately
	// thrown after enabling paging and then another when trying to handle that fault
	if (pager_map((void *)pt_root, Arc_BootMeta.bsp_image.base, Arc_BootMeta.bsp_image.base, 
		      Arc_BootMeta.bsp_image.size, 1 << ARC_PAGER_4K) != 0) {
		ARC_DEBUG(ERR, "\tFailed to identity map!\n");
		ARC_HANG;
	}

	// Parse the Kernel ELF file and map it into memory and set 
	// kernel_entry to the virtual address of where the kernel is
	kernel_entry = load_elf((void *)pt_root, (uint8_t *)((uintptr_t)Arc_KernelMeta.kernel_elf));

	watermark_update_mmap();

	const char *names[] = {
		[ARC_MEMORY_AVAILABLE] = "Available",
		[ARC_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
		[ARC_MEMORY_BADRAM] = "Bad",
		[ARC_MEMORY_NVS] = "NVS",
		[ARC_MEMORY_RESERVED] = "Reserved",
		[ARC_MEMORY_BOOTSTRAP] = "Bootstrap",
		[ARC_MEMORY_BOOTSTRAP_ALLOC] = "Bootstrap Allocated",
	};

	struct ARC_MMap *mmap = (struct ARC_MMap *)Arc_KernelMeta.arc_mmap.base;
	ARC_DEBUG(INFO, "Arctan MMap: 0x%"PRIx64" (%d entries)\n", Arc_KernelMeta.arc_mmap.base, Arc_KernelMeta.arc_mmap.len);
	for (uint32_t i = 0; i < Arc_KernelMeta.arc_mmap.len; i++) {
		struct ARC_MMap entry = mmap[i];
		ARC_DEBUG(INFO, "\t%3d : 0x%016"PRIx64" -> 0x%016"PRIx64" (0x%016"PRIx64" bytes) | %s (%d)\n", i, entry.base, entry.base + entry.len, entry.len, names[entry.type], entry.type);
	}

	ARC_DEBUG(INFO, "Finished bootstrapping, returning to assembly to setup long mode and jump to kernel (0x%"PRIx64") %"PRIx64"\n", kernel_entry);

	// Jump back to the assembly phase, which will enable paging and put the
	// system into long mode before jumping to the kernel
	return kernel_entry;
}
