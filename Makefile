#/**
# * @file Makefile
# *
# * @author awewsomegamer <awewsomegamer@gmail.com>
# *
# * @LICENSE
# * Arctan-MB2BSP - Multiboot2 Bootstrapper for Arctan Kernel
# * Copyright (C) 2023-2024 awewsomegamer
# *
# * This file is part of Arctan-MB2BSP
# *
# * Arctan is free software; you can redistribute it and/or
# * modify it under the terms of the GNU General Public License
# * as published by the Free Software Foundation; version 2
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# *
# * @DESCRIPTION
#*/
CC ?= gcc
LD ?= ld

ifeq (,$(ARC_ROOT))
	$(ARC_ROOT) := .
endif

PRODUCT := bootstrap.elf

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")
OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS := $(CPPFLAG_DEBUG) $(CPPFLAG_E9HACK) -I src/c/include $(CPP_DEBUG_FLAG) $(ARC_DEF_COM) $(ARC_DEF_DEBUG)
CFLAGS := -m32 -c -fno-stack-protector -mno-sse -mno-sse2 -masm=intel -nostdlib -nodefaultlibs -fno-builtin -mabi=sysv -mno-sse -mno-sse2
LDFLAGS := -Tlinker.ld -melf_i386 -z max-page-size=0x1000 -o $(PRODUCT)
NASMFLAGS := -f elf32

.PHONY: all
all: $(ARC_PRODUCT)

.PHONY: pre-build
pre-build: clean

$(ARC_PRODUCT): pre-build
	$(MAKE) $(OFILES)
	$(LD) $(LDFLAGS) $(OFILES)

	rm -rf iso
	mkdir -p iso/boot/grub

# Copy various important things to grub directory
	cp -r $(ARC_VOLATILE)/* iso/boot
	cp bootstrap.elf iso/boot
	cp $(ARC_BUILD_SUPPORT)/grub.cfg iso/boot/grub

# Create ISO
	grub-mkrescue -o $(ARC_PRODUCT) iso

src/c/%.o: src/c/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf iso
	rm -f $(RODUCT)
	find -type f -name "*.o" -delete