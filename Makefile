#/**
# * @file Makefile
# *
# * @author awewsomegamer <awewsomegamer@gmail.com>
# *
# * @LICENSE
# * Arctan - Operating System Kernel
# * Copyright (C) 2023-2024 awewsomegamer
# *
# * This file is part of Arctan.
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

PRODUCT := bootstrap.elf

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS := $(CPPFLAG_DEBUG) $(CPPFLAG_E9HACK) -I src/c/include -I $(ARC_ROOT)/initramfs/include
CFLAGS := -m32 -c -fno-stack-protector -mno-sse -mno-sse2 -masm=intel -nostdlib -nodefaultlibs -fno-builtin

LDFLAGS := -Tlinker.ld -melf_i386 -z max-page-size=0x1000 -o $(PRODUCT)

NASMFLAGS := -f elf32

.PHONY: all
all: $(OFILES)
	$(LD) $(LDFLAGS) $(OFILES)

	mkdir -p iso/boot/grub
	
	# Copy various important things to grub directory
	cp $(BASE_DIR)/initramfs.cpio iso/boot
	cp kernel.elf iso/boot
	cp bootstrap.elf iso/boot
	cp $(BASE_DIR)/build-support/grub.cfg iso/boot/grub
	
	# Create ISO
	grub-mkrescue -o Arctan.iso iso

	cp Arctan.iso $(BASE_DIR)

src/c/%.o: src/c/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@