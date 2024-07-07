/**
 * @file mbparse.c
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
#include <arctan.h>
#include <multiboot/mbparse.h>
#include <multiboot/multiboot2.h>
#include <global.h>
#include <inttypes.h>
#include <interface/terminal.h>
#include <stdint.h>

struct ARC_MB2BootInfo {
	uint64_t mbi_phys;
	uint64_t fb;
}__attribute__((packed));
struct ARC_MB2BootInfo mb2_bootinfo = { 0 };

struct mb2_base_tag {
	uint32_t type;
	uint32_t size;
}__attribute__((packed));

struct ARC_MMap arc_mmap[512]; // This is a lot of memory map entries, if you have
			       // a system that has more than this then good luck I guess
/**
 * Converts a MB2 memory type to ARC type
 */
uint32_t mb2_MMType2Type(uint32_t mb2_type) {
	switch (mb2_type) {
		case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE: {
			return ARC_MEMORY_ACPI_RECLAIMABLE;
		}
		case MULTIBOOT_MEMORY_AVAILABLE: {
			return ARC_MEMORY_AVAILABLE;
		}
		case MULTIBOOT_MEMORY_BADRAM: {
			return ARC_MEMORY_BADRAM;
		}
		case MULTIBOOT_MEMORY_NVS: {
			return ARC_MEMORY_NVS;
		}
		case MULTIBOOT_MEMORY_RESERVED: {
			return ARC_MEMORY_RESERVED;
		}
	}

	ARC_DEBUG(ERR, "Unknown MB2 memory type: %d\n", mb2_type);

	return -1;
}

int Arc_ParseMB2I(uint8_t *mb2i) {
	// Get information from the fixed tag
	uint32_t total_size = *(uint32_t *)mb2i;

	if (total_size == 0) {
		ARC_DEBUG(ERR, "Given MB2I's size is 0\n");
		return -1;
	}

	mb2_bootinfo.mbi_phys = (uint64_t)mb2i;

	// Start at first 8-byte aligned tag
	uint32_t offset = 8;

	ARC_DEBUG(INFO, "Parsing Multiboot2 tags\n");

	// First look for information like the framebuffer and modules
	// so that there is a way to display erros and such to the user
	// and look for modules / address related things which would
	// help calculate bootstrap_end_phys
	struct multiboot_tag_framebuffer *fb_tag = NULL;
	uint32_t bootstrap_end_phys = 0x0;

	while (offset < total_size) {
		struct mb2_base_tag *tag = (struct mb2_base_tag *)(mb2i + offset);

		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			fb_tag = (struct multiboot_tag_framebuffer *)tag;

			// Set framebuffer
			struct multiboot_tag_framebuffer_common common = (struct multiboot_tag_framebuffer_common)fb_tag->common;
			Arc_SetTerm((void *)common.framebuffer_addr, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp);
			ARC_DEBUG(INFO, "Framebuffer 0x%"PRIx64" (%d) %dx%dx%d\n", common.framebuffer_addr, common.framebuffer_type, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp);

			mb2_bootinfo.fb = (uint64_t)tag;

			break;
		}

		case MULTIBOOT_TAG_TYPE_MODULE: {
			struct multiboot_tag_module *info = (struct multiboot_tag_module *)tag;

			uint32_t last_address = ALIGN(info->mod_end, PAGE_SIZE);

			ARC_DEBUG(INFO, "Module:\n");
			ARC_DEBUG(INFO, "\tStart: 0x%"PRIx32"\n", info->mod_start);
			ARC_DEBUG(INFO, "\tEnd: 0x%"PRIx32" (0x%"PRIx32")\n", info->mod_end, last_address);
			ARC_DEBUG(INFO, "\tCommand: %s\n", info->cmdline);

			if (last_address > bootstrap_end_phys) {
				bootstrap_end_phys = last_address;
			}

			if (strcmp(info->cmdline, "arctan-module.kernel.elf") == 0) {
				ARC_DEBUG(INFO, "\tFound kernel!\n");
				_boot_meta.kernel_elf = (uint64_t)info->mod_start;
			} else if (strcmp(info->cmdline, "arctan-module.initramfs.cpio") == 0) {
				ARC_DEBUG(INFO, "\tFound initramfs!\n");
				_boot_meta.initramfs = (uint64_t)info->mod_start;
				_boot_meta.initramfs_size = (uint64_t)(info->mod_end - info->mod_start);
			}


			break;
		}

		case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
			// TODO: Could probably use this to get a more accurate bootstrap_end_phys

			break;
		}

		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
			// TODO: Could be used to get more accurate bootstrap_end_phys
			struct multiboot_tag_load_base_addr *info = (struct multiboot_tag_load_base_addr *)tag;


			bsp_image_base = (uint64_t)info->load_base_addr;
			bsp_image_ciel = (uint64_t)bsp_image_base + ((uint64_t)&__BOOTSTRAP_END__ - (uint64_t)&__BOOTSTRAP_START__);

			ARC_DEBUG(INFO, "Base address:\n");
			ARC_DEBUG(INFO, "\tLoaded at: 0x%"PRIx32"\n", info->load_base_addr);
			ARC_DEBUG(INFO, "\tLast address: 0x%"PRIx32" (0x%"PRIx32")\n", bsp_image_ciel, ALIGN(bsp_image_ciel, PAGE_SIZE));

			if (ALIGN(bsp_image_ciel, PAGE_SIZE) > bootstrap_end_phys) {
				bootstrap_end_phys = ALIGN(bsp_image_ciel, PAGE_SIZE);
			}

			break;
		}
		}

		offset += ALIGN(tag->size, 8);
	}

	offset = 8;

	ARC_DEBUG(INFO, "Bootstrap physical end: 0x%"PRIx32"\n", bootstrap_end_phys);

	// Go through the rest of the tags and initialize the meta
	//    If a memory map is found, parse it into a ARC_MMap
	while (offset < total_size) {
		struct mb2_base_tag *tag = (struct mb2_base_tag *)(mb2i + offset);

		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
			struct multiboot_tag_string *info = (struct multiboot_tag_string *)tag;

			ARC_DEBUG(INFO, "Bootloader: %s\n", info->string);

			break;
		}
		
		case MULTIBOOT_TAG_TYPE_CMDLINE: {
			struct multiboot_tag_string *info = (struct multiboot_tag_string *)tag;

			ARC_DEBUG(INFO, "Command line arguments: %s\n", info->string);

			break;
		}

		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			ARC_DEBUG(INFO, "Framebuffer already found\n");
			break;
		}

		case MULTIBOOT_TAG_TYPE_MMAP: {
			struct multiboot_tag_mmap *info = (struct multiboot_tag_mmap *)tag;
			uint32_t entries = (info->size - sizeof(struct multiboot_tag_mmap)) / info->entry_size;

			ARC_DEBUG(INFO, "Found MMap (%d, %d entries)\n", info->entry_version, entries);

                        const char *names[] = {
                                [MULTIBOOT_MEMORY_AVAILABLE] = "Available",
                                [MULTIBOOT_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
                                [MULTIBOOT_MEMORY_BADRAM] = "Bad",
                                [MULTIBOOT_MEMORY_NVS] = "NVS",
                                [MULTIBOOT_MEMORY_RESERVED] = "Reserved"
                        };

			uint32_t arc_mmap_entry = 0;

			for (uint32_t i = 0; i < entries; i++) {
				struct multiboot_mmap_entry entry = info->entries[i];
				ARC_DEBUG(INFO, "\t%d : 0x%"PRIx64" -> 0x%"PRIx64" (0x%"PRIx64" bytes) | %s (%d)\n", i, entry.addr, entry.addr + entry.len, entry.len, names[entry.type], entry.type);

				int overlap = 0;

				// Check if this entry is contained within another
				for (uint32_t j = 0; j < i; j++) {
					struct multiboot_mmap_entry cmp = info->entries[j];

					// Check if entry is in cmp
					//     cmp_low < low - The comparison's low address is lower than current
					//     high < cmp_high - The comparison's high address is higher than current
					if (cmp.addr < entry.addr && (entry.addr + entry.len) < (cmp.addr + cmp.len)) {
						ARC_DEBUG(INFO, "\tOverlapping entries! (%d overlaps %d)", i, j);
						overlap = 1;
					}
				}

				if (overlap) {
					// Overlapping sections, skip to the next one
					continue;
				}

				uint64_t base = entry.addr;
				uint64_t len = entry.len;
				uint32_t type = mb2_MMType2Type(entry.type);

				if (base < bootstrap_end_phys && type == ARC_MEMORY_AVAILABLE) {
					// Entry is below bootstrap end
					uint32_t _type = type;
					type = ARC_MEMORY_BOOTSTRAP;

					ARC_DEBUG(INFO, "\t\tEntry is below bootstrap end, marking as BOOTSTRAP\n");

					if (bootstrap_end_phys < base + entry.len)  {
						// Entry contains bootstrap end, split it
						ARC_DEBUG(INFO, "\t\tEntry contains bootstrap end, splitting\n");

						uint64_t delta = bootstrap_end_phys - base;

						arc_mmap[arc_mmap_entry].base = base;
						base += delta;
						arc_mmap[arc_mmap_entry].len = delta;
						len -= delta;
						arc_mmap[arc_mmap_entry].type = ARC_MEMORY_BOOTSTRAP;
						arc_mmap_entry++;

						type = _type;
					}
				}

				arc_mmap[arc_mmap_entry].base = base;
				arc_mmap[arc_mmap_entry].len = len;
				arc_mmap[arc_mmap_entry].type = type;
				arc_mmap_entry++;

				// Update highest physical address
				if (entry.addr > _boot_meta.highest_address) {
					_boot_meta.highest_address = (uint64_t)(entry.addr + entry.len);
				}
			}

			_boot_meta.arc_mmap = (uint64_t)&arc_mmap;
			_boot_meta.arc_mmap_len = arc_mmap_entry;

			ARC_DEBUG(INFO, "Highest physical address: 0x%"PRIx64"\n", _boot_meta.highest_address);
			ARC_DEBUG(INFO, "Arctan MMap: 0x%"PRIx64" (%d entries)\n", _boot_meta.arc_mmap, _boot_meta.arc_mmap_len);

			break;
		}

		case MULTIBOOT_TAG_TYPE_EFI_MMAP: {
			ARC_DEBUG(INFO, "Found EFI MMap\n");

			break;
		}

		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
			struct multiboot_tag_basic_meminfo *info = (struct multiboot_tag_basic_meminfo *)tag;

			ARC_DEBUG(INFO, "Basic memory:\n");
			ARC_DEBUG(INFO, "\tLow: %d KiB\n", info->mem_lower);
			ARC_DEBUG(INFO, "\tHigh: %d KiB\n", info->mem_upper);

			break;
		}

		case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
			struct multiboot_tag_new_acpi *acpi = (struct multiboot_tag_new_acpi *)tag;
			_boot_meta.rsdp = (uint64_t)&acpi->rsdp;
			ARC_DEBUG(INFO, "Found new ACPI (0x%"PRIx64")\n", acpi->rsdp);

			break;
		}

		case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
			struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
			_boot_meta.rsdp = (uint64_t)&acpi->rsdp;
			ARC_DEBUG(INFO, "Found old ACPI (0x%"PRIx32")\n", acpi->rsdp);

			break;
		}
		
		case MULTIBOOT_TAG_TYPE_MODULE: { break; }
		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: { break; }

		case MULTIBOOT_TAG_TYPE_END: {
			ARC_DEBUG(INFO, "Parsed all tags\n");

			break;
		}
		default: {
			ARC_DEBUG(ERR, "Unhandled tag (%d) at %p (:%d)\n", tag->type, tag, offset);

			break;
		}
		}

		offset += ALIGN(tag->size, 8);
	}

	_boot_meta.boot_info = (uint64_t)&mb2_bootinfo;

	return 0;
}
