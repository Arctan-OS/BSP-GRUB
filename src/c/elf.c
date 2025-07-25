/**
 * @file elf.c
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
 * Simple IDT to handle errors which may occur when getting to the kernel.
*/
#include <elf.h>
#include <global.h>
#include <inttypes.h>
#include <mm/watermark.h>
#include <arch/pager.h>

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11

#define EI_MAG0        0
#define EI_MAG1        1
#define EI_MAG2        2
#define EI_MAG3        3

#define EI_CLASS       4
#define CLASS_32       1
#define CLASS_64       2

#define EI_DATA        5
#define EI_VERSION     6

#define EI_OSABI       7
#define ABI_SYSV       0
#define ABI_HPUX       1
#define ABI_STANDALONE 2

#define EI_ABIVERSION  8
#define EI_PAD         9
#define EI_NIDENT      16

static const char *section_types[] = {
	[SHT_NULL] = "NULL",
	[SHT_PROGBITS] = "PROGBITS",
	[SHT_SYMTAB] = "SYMTAB",
	[SHT_STRTAB] = "STRTAB",
	[SHT_RELA] = "RELA",
	[SHT_HASH] = "HASH",
	[SHT_DYNAMIC] = "DYNAMIC",
	[SHT_NOTE] = "NOTE",
	[SHT_NOBITS] = "NOBITS",
	[SHT_REL] = "REL",
	[SHT_SHLIB] = "SHLIB",
	[SHT_DYNSYM] = "DYNSYM",
};

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef uint64_t Elf64_Sxword;

struct Elf64_Ehdr {
	unsigned char e_ident[16]; /* ELF identification */
	Elf64_Half e_type; /* Object file type */
	Elf64_Half e_machine; /* Machine type */
	Elf64_Word e_version; /* Object file version */
	Elf64_Addr e_entry; /* Entry point address */
	Elf64_Off e_phoff; /* Program header offset */
	Elf64_Off e_shoff; /* Section header offset */
	Elf64_Word e_flags; /* Processor-specific flags */
	Elf64_Half e_ehsize; /* ELF header size */
	Elf64_Half e_phentsize; /* Size of program header entry */
	Elf64_Half e_phnum; /* Number of program header entries */
	Elf64_Half e_shentsize; /* Size of section header entry */
	Elf64_Half e_shnum; /* Number of section header entries */
	Elf64_Half e_shstrndx; /* Section name string table index */
}__attribute__((packed));

struct Elf64_Shdr {
	Elf64_Word sh_name; /* Section name */
	Elf64_Word sh_type; /* Section type */
	Elf64_Xword sh_flags; /* Section attributes */
	Elf64_Addr sh_addr; /* Virtual address in memory */
	Elf64_Off sh_offset; /* Offset in file */
	Elf64_Xword sh_size; /* Size of section */
	Elf64_Word sh_link; /* Link to other section */
	Elf64_Word sh_info; /* Miscellaneous information */
	Elf64_Xword sh_addralign; /* Address alignment boundary */
	Elf64_Xword sh_entsize; /* Size of entries, if section has table */
}__attribute__((packed));

struct Elf64_Sym {
	Elf64_Word st_name; /* Symbol name */
	unsigned char st_info; /* Type and Binding attributes */
	unsigned char st_other; /* Reserved */
	Elf64_Half st_shndx; /* Section table index */
	Elf64_Addr st_value; /* Symbol value */
	Elf64_Xword st_size; /* Size of object (e.g., common) */
}__attribute__((packed));

struct Elf64_Rel {
	Elf64_Addr r_offset; /* Address of reference */
	Elf64_Xword r_info; /* Symbol index and type of relocation */
}__attribute__((packed));

struct Elf64_Rela {
	Elf64_Addr r_offset; /* Address of reference */
	Elf64_Xword r_info; /* Symbol index and type of relocation */
	Elf64_Sxword r_addend; /* Constant part of expression */
}__attribute__((packed));

struct Elf64_Phdr {
	Elf64_Word p_type; /* Type of segment */
	Elf64_Word p_flags; /* Segment attributes */
	Elf64_Off p_offset; /* Offset in file */
	Elf64_Addr p_vaddr; /* Virtual address in memory */
	Elf64_Addr p_paddr; /* Reserved */
	Elf64_Xword p_filesz; /* Size of segment in file */
	Elf64_Xword p_memsz; /* Size of segment in memory */
	Elf64_Xword p_align; /* Alignment of segment */
}__attribute__((packed));

uint64_t elf_load64(void *page_tables, uint8_t *data) {
	ARC_DEBUG(INFO, "Loading 64-bit ELF file (%p)\n", data);

	struct Elf64_Ehdr *header = (struct Elf64_Ehdr *)data;

	uint64_t entry_addr = header->e_entry;

	ARC_DEBUG(INFO, "Entry: %"PRIx64"\n", entry_addr);

	uint32_t section_count = header->e_shnum;
	ARC_DEBUG(INFO, "Mapping sections (%d sections):\n", section_count);

	struct Elf64_Shdr strings = ((struct Elf64_Shdr *)(data + header->e_shoff))[header->e_shstrndx];

	uint64_t phys_address = 0;
	for (uint32_t i = 0; i < section_count; i++) {
		struct Elf64_Shdr section_header = ((struct Elf64_Shdr *)(data + header->e_shoff))[i];

		uint64_t load_base = section_header.sh_addr;
		uint64_t load_size = ALIGN(section_header.sh_size, PAGE_SIZE);

		ARC_DEBUG(INFO, "\t%3d: 0x%016"PRIx64", 0x%016"PRIx64" B | Type: %d\n", i, load_base, load_size, section_header.sh_type);

		if (load_base == 0 || load_size == 0) {
			ARC_DEBUG(INFO, "\t\tSkipping as load_base or size is 0\n");
			continue;
		}

		uint64_t phys = (uint64_t)data + section_header.sh_offset;

		if (section_header.sh_type == SHT_NOBITS) {
			phys = (uint64_t)alloc(load_size);
			memset((void *)phys, 0, load_size);
		}

		if (pager_map(page_tables, load_base, phys, load_size, 1 << ARC_PAGER_RW) != 0) {
			ARC_DEBUG(ERR, "Failed to map %"PRIx64" -> %"PRIx64"\n", load_base, phys);
		}
	}

	return entry_addr;
}

uint64_t load_elf(void *page_tables, uint8_t *data) {
	struct Elf64_Ehdr *header = (struct Elf64_Ehdr *)data;

	if (header->e_ident[EI_CLASS] != CLASS_64) {
		ARC_DEBUG(ERR, "ELF file is not 64-bit\n");
		return -1;
	}

	return elf_load64(page_tables, data);
}
