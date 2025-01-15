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

CPP_DEBUG_FLAG := -DARC_DEBUG_ENABLE
CPP_E9HACK_FLAG := -DARC_E9HACK_ENABLE

ifeq (,$(ARC_ROOT))
	$(ARC_ROOT) := .
endif

ifeq (,$(wildcard ./e9hack.enable))
# Disable E9HACK
	CPP_E9HACK_FLAG :=
endif

ifeq (,$(wildcard ./debug.enable))
# Disable debugging
	CPP_DEBUG_FLAG :=
else
# Must set serial flag if debugging
	CPP_E9HACK_FLAG := -DARC_E9HACK_ENABLE
endif

ifneq (,$(wildcard ./hardware.enable))
# hardware.enable is present, disable E9
	CPP_E9HACK_FLAG :=
endif

PRODUCT := bootstrap.elf

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS := $(CPPFLAG_DEBUG) $(CPPFLAG_E9HACK) -I src/c/include $(CPP_DEBUG_FLAG) $(CPP_E9HACK_FLAG)
CFLAGS := -m32 -c -fno-stack-protector -mno-sse -mno-sse2 -masm=intel -nostdlib -nodefaultlibs -fno-builtin -mabi=sysv -mno-sse -mno-sse2

LDFLAGS := -Tlinker.ld -melf_i386 -z max-page-size=0x1000 -o $(PRODUCT)

NASMFLAGS := -f elf32

.PHONY: all
all: $(OFILES)
	$(LD) $(LDFLAGS) $(OFILES)

	rm -rf iso
	mkdir -p iso/boot/grub

# Copy various important things to grub directory
	cp -r $(ARC_ROOT)/volatile/* iso/boot
	cp bootstrap.elf iso/boot
	cp $(ARC_ROOT)/build-support/grub.cfg iso/boot/grub

# Create ISO
	grub-mkrescue -o $(ARC_ROOT)/Arctan.iso iso

src/c/%.o: src/c/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf iso
	rm -f $(RODUCT)
	find -type f -name "*.o" -delete

-include clean
