/**
 * @file mb2parse.c
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
 * Abstract freelist implementation.
*/
#include "util.h"
#include <arctan.h>
#include <boot/mb2parse.h>
#include <boot/multiboot2.h>
#include <global.h>
#include <inttypes.h>
#include <interface/terminal.h>
#include <stdint.h>

struct mb2_base_tag {
	uint32_t type;
	uint32_t size;
}__attribute__((packed));

struct ARC_MMap arc_mmap[512] = { 0 }; // This is a lot of memory map entries, if you have
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

int parse_mb2i(uint8_t *mb2i) {
	// Get information from the fixed tag
	uint32_t total_size = *(uint32_t *)mb2i;

	if (total_size == 0) {
		ARC_DEBUG(ERR, "Given MB2I's size is 0\n");
		return -1;
	}

	Arc_KernelMeta.boot.proc = ARC_BOOTPROC_MB2;
	Arc_KernelMeta.boot.info.grub_tags = (uint64_t)mb2i;

	// Start at first 8-byte aligned tag
	uint32_t offset = 8;

	ARC_DEBUG(INFO, "Parsing Multiboot2 tags\n");

	// First look for information like the framebuffer and modules
	// so that there is a way to display erros and such to the user
	// and look for modules / address related things which would
	// help calculate bootstrap_end_phys
	struct multiboot_tag_framebuffer *fb_tag = NULL;
	uintptr_t bootstrap_begin_phys = 0;
	uintptr_t bootstrap_end_phys = 0;

	while (offset < total_size) {
		struct mb2_base_tag *tag = (struct mb2_base_tag *)(mb2i + offset);

		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				fb_tag = (struct multiboot_tag_framebuffer *)tag;

				// Set framebuffer
				struct multiboot_tag_framebuffer_common common = (struct multiboot_tag_framebuffer_common)fb_tag->common;
				set_term((void *)(common.framebuffer_addr), common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp);
				ARC_DEBUG(INFO, "Framebuffer 0x%"PRIx64" (%d) %dx%dx%d\n", common.framebuffer_addr, common.framebuffer_type, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp);

				break;
			}

			case MULTIBOOT_TAG_TYPE_MODULE: {
				struct multiboot_tag_module *info = (struct multiboot_tag_module *)tag;

				uint32_t last_address = ALIGN(info->mod_end, PAGE_SIZE);

				ARC_DEBUG(INFO, "Module:\n");
				ARC_DEBUG(INFO, "\tStart: 0x%"PRIx32"\n", info->mod_start);
				ARC_DEBUG(INFO, "\tEnd: 0x%"PRIx32" (0x%"PRIx32")\n", info->mod_end, last_address);
				ARC_DEBUG(INFO, "\tCommand: %s\n", info->cmdline);

				// Check if the last address (page aligned) is higher
				// than the current end of the bootstrapper, so it can
				// be excluded from the ARC_MMap
				if (last_address > bootstrap_end_phys) {
					bootstrap_end_phys = last_address;
				}

				// Check for important modules (kernel + initramfs) and set
				// proper variables
				if (strcmp(info->cmdline, "arctan-module.kernel.elf") == 0) {
					ARC_DEBUG(INFO, "\tFound kernel!\n");
					Arc_KernelMeta.kernel_elf = (uint64_t)info->mod_start;
				} else if (strcmp(info->cmdline, "arctan-module.initramfs.cpio") == 0) {
					ARC_DEBUG(INFO, "\tFound initramfs!\n");
					Arc_KernelMeta.initramfs.base = (uint64_t)info->mod_start;
					Arc_KernelMeta.initramfs.size = (uint64_t)(info->mod_end - info->mod_start);
				}

				break;
			}

			case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
				// TODO: Could probably use this to get a more accurate bootstrap_end_phys
				struct multiboot_tag_elf_sections *info = (struct multiboot_tag_elf_sections *)tag;

				break;
			}

			case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
				// The image is relocatable, check for the load base
				// and calculate the last address used by bootstrap.elf
				struct multiboot_tag_load_base_addr *info = (struct multiboot_tag_load_base_addr *)tag;

				Arc_BootMeta.bsp_image.base = (uint64_t)info->load_base_addr;
				Arc_BootMeta.bsp_image.size = ALIGN((uint64_t)((uintptr_t)&__BOOTSTRAP_END__ - (uintptr_t)&__BOOTSTRAP_START__), PAGE_SIZE);

				bootstrap_begin_phys = info->load_base_addr;

				ARC_DEBUG(INFO, "Base address:\n");
				ARC_DEBUG(INFO, "\tLoaded at: 0x%"PRIx32"\n", info->load_base_addr);
				ARC_DEBUG(INFO, "\tSize: 0x%"PRIx64"\n", Arc_BootMeta.bsp_image.size);

				break;
			}

			case MULTIBOOT_TAG_TYPE_END: {
				ARC_DEBUG(INFO, "Parsed primary tags\n");

				break;
			}
		}

		offset += ALIGN(tag->size, 8);
	}

	// After parsing primary information, parse
	// secondary information
	offset = 8;

	// Go through the rest of the tags and initialize the meta
	//    If a memory map is found, parse it into a ARC_MMap
	while (offset < total_size) {
		struct mb2_base_tag *tag = (struct mb2_base_tag *)(mb2i + offset);

		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
				// Cosmetic
				struct multiboot_tag_string *info = (struct multiboot_tag_string *)tag;

				ARC_DEBUG(INFO, "Bootloader: %s\n", info->string);

				break;
			}

			case MULTIBOOT_TAG_TYPE_MMAP: {
				// BIOS memory map, grab it print it, and reconstruct it into
				// the ARC_MMap
				struct multiboot_tag_mmap *info = (struct multiboot_tag_mmap *)tag;
				uint32_t entries = (info->size - sizeof(struct multiboot_tag_mmap)) / info->entry_size;

				ARC_DEBUG(INFO, "Found MMap (%d, %d entries)\n", info->entry_version, entries);

				// Sort entries by ascending base address and place them into the list
				for (uint32_t i = 0, j = 0; i < entries; i++) {
					struct multiboot_mmap_entry entry = info->entries[i];

					uint32_t idx = j;

					for (uint32_t _j = 0; _j < j; _j++) {
						if (entry.addr <= arc_mmap[_j].base) {
							idx = _j;
							nmemcpy(&arc_mmap[_j + 1], &arc_mmap[_j], (j - _j) * sizeof(struct ARC_MMap));
							break;
						}
					}

					arc_mmap[idx].base = entry.addr;
					arc_mmap[idx].len = entry.len;
					arc_mmap[idx].type = mb2_MMType2Type(entry.type);
					j++;
				}

				for (uint32_t i = 0; i < entries - 1; i++) {
					uint64_t ceil = arc_mmap[i].base + arc_mmap[i].len;

					if (arc_mmap[i + 1].base < ceil) {
						size_t delta = min(ceil - arc_mmap[i + 1].base, arc_mmap[i + 1].len);

						if (arc_mmap[i].type == arc_mmap[i + 1].type) {
							// Merge
							arc_mmap[i].len += arc_mmap[i + 1].len;

							if (i != entries - 2) {
								memcpy(&arc_mmap[i + 1], &arc_mmap[i + 2], (entries - i - 2) * sizeof(struct ARC_MMap));
							} else {
								memset(&arc_mmap[i + 1], 0, sizeof(struct ARC_MMap));
							}

							entries--;
						}

						if (arc_mmap[i].type < arc_mmap[i + 1].type) {
							// [i] takes precendence over [i + 1]
							arc_mmap[i + 1].base += delta;
							arc_mmap[i + 1].len -= delta;

							if (arc_mmap[i + 1].len == 0) {
								if (i != entries - 2) {
									memcpy(&arc_mmap[i + 1], &arc_mmap[i + 2], (entries - i - 2) * sizeof(struct ARC_MMap));
								} else {
									memset(&arc_mmap[i + 1], 0, sizeof(struct ARC_MMap));
								}

								entries--;
							}
						} else {
							// [i + 1] takes precendence over [i]
							arc_mmap[i].len -= delta;
						}
					}

					if (arc_mmap[i].base >= bootstrap_begin_phys && arc_mmap[i].base < bootstrap_end_phys) {
						size_t delta = bootstrap_end_phys - arc_mmap[i].base;
						if (delta >= arc_mmap[i].len && arc_mmap[i].type > ARC_MEMORY_BOOTSTRAP) {
							// Reserve
							arc_mmap[i].type = ARC_MEMORY_BOOTSTRAP;
						} else if (delta < arc_mmap[i].len) {
							// Split
							nmemcpy(&arc_mmap[i + 1], &arc_mmap[i], (entries - i) * sizeof(struct ARC_MMap));
							arc_mmap[i + 1].len -= delta;
							arc_mmap[i + 1].base += delta;
							arc_mmap[i].len = delta;
							arc_mmap[i].type = ARC_MEMORY_BOOTSTRAP;

							entries++;
						}
					}

				}
			
				Arc_KernelMeta.arc_mmap.base = (uintptr_t)&arc_mmap;
				Arc_KernelMeta.arc_mmap.len = entries;

				const char *names[] = {
                                        [ARC_MEMORY_AVAILABLE] = "Available",
                                        [ARC_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
                                        [ARC_MEMORY_BADRAM] = "Bad",
                                        [ARC_MEMORY_NVS] = "NVS",
                                        [ARC_MEMORY_RESERVED] = "Reserved",
				        [ARC_MEMORY_BOOTSTRAP] = "Bootstrap"
			        };

				ARC_DEBUG(INFO, "Memory size: 0x%"PRIx64"\n", Arc_BootMeta.mem_size);
				ARC_DEBUG(INFO, "Arctan MMap: 0x%"PRIx64" (%d entries)\n", Arc_KernelMeta.arc_mmap.base, Arc_KernelMeta.arc_mmap.len);
				for (uint32_t i = 0; i < Arc_KernelMeta.arc_mmap.len; i++) {
					struct ARC_MMap entry = arc_mmap[i];
					ARC_DEBUG(INFO, "\t%3d : 0x%016"PRIx64" -> 0x%016"PRIx64" (0x%016"PRIx64" bytes) | %s (%d)\n", i, entry.base, entry.base + entry.len, entry.len, names[entry.type], entry.type);
				}

				break;
			}

			case MULTIBOOT_TAG_TYPE_EFI_MMAP: {
				ARC_DEBUG(WARN, "Found EFI MMap (implement)\n");
				// TODO: Implement EFI mmap parsing



				break;
			}

			case MULTIBOOT_TAG_TYPE_EFI32: {
				ARC_DEBUG(WARN, "EFI32\n");

				break;
			}

			case MULTIBOOT_TAG_TYPE_EFI64: {
				ARC_DEBUG(WARN, "EFI64\n");

				break;
			}

			case MULTIBOOT_TAG_TYPE_EFI_BS: {
				ARC_DEBUG(ERR, "EFI bootservices not terminated\n");

				// TODO: Terminate them

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
				Arc_KernelMeta.rsdp = (uintptr_t)&acpi->rsdp;
				ARC_DEBUG(INFO, "Found new ACPI (0x%"PRIx64")\n", acpi->rsdp);

				break;
			}

			case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
				struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
				Arc_KernelMeta.rsdp = (uintptr_t)&acpi->rsdp;
				ARC_DEBUG(INFO, "Found old ACPI (0x%"PRIx32")\n", acpi->rsdp);

				break;
			}

			case MULTIBOOT_TAG_TYPE_APM: {
				ARC_DEBUG(WARN, "Found APM (implement)\n");

				break;
			}

			case MULTIBOOT_TAG_TYPE_BOOTDEV: {
				ARC_DEBUG(WARN, "Found bootdev (implement)\n");

				break;
			}

			case MULTIBOOT_TAG_TYPE_VBE: {
				ARC_DEBUG(WARN, "Found VBE (implement)\n");

				break;
			}

				// Already handled
			case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: { break; }
			case MULTIBOOT_TAG_TYPE_MODULE: { break; }
			case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: { break; }
			case MULTIBOOT_TAG_TYPE_CMDLINE: { break; }
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: { break; }

			case MULTIBOOT_TAG_TYPE_END: {
				ARC_DEBUG(INFO, "Parsed secondary tags\n");

				break;
			}
			default: {
				ARC_DEBUG(ERR, "Unhandled tag (%d) at %p (:%d)\n", tag->type, tag, offset);

				break;
			}
		}

		offset += ALIGN(tag->size, 8);
	}

	return 0;
}
