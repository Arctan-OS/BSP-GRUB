%if 0
/**
 * @file boot.asm
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
%endif
bits 32

MAGIC               equ 0xE85250D6
ARCH                equ 0
LENGTH              equ (boot_header - boot_header_end)
CHECKSUM            equ -(MAGIC + ARCH + LENGTH)
STACK_SZ            equ 0x1000                      ; 4 KiB of stack should be fine for now

section .mb2header

align 8
boot_header:
        dd MAGIC
        dd ARCH
        dd LENGTH
        dd CHECKSUM
align 8
        ;; Module Align tag
        dw 0x6
        dw 0x0
        dd 0x8
align 8
        ;; Framebuffer Request tag
        dw 0x5
        dw 0x0
        dd 0x20
        ;; Allow bootloader to pick width
        dd 0x0
        ;; Allow bootloader to pick height
        dd 0x0
        ;; Allow bootloader to pick bpp
        dd 0x0
align 8
        ;; Relocatable Image tag
        dw 0xA
        dw 0x0
        dd 24
        dd 0x100000
        dd 0x10000000
        dd 0x1000
        dd 1
align 8
        ;; End of Tags tag
        dw 0x0
        dw 0x0
        dd 0x8
boot_header_end:

section .text

extern bsp
extern pt_root
extern _kernel_station
global _entry
_entry:
        cli
        ;; Setup everything
        mov ebp, _BOOTSTRAP_STACK
        mov esp, ebp
        push eax
        push ebx
        call bsp

        ;; Switch to long mode and set paging
        mov eax, cr4
        or eax, 1 << 5
        mov cr4, eax

        mov eax, dword [pt_root]
        mov cr3, eax

        mov ecx, 0xC0000080
        rdmsr
        or eax, 1 << 8
        wrmsr

        mov eax, cr0
        or eax, 1 << 31
        mov cr0, eax

        ;; Jump to 64/entry.asm
        jmp 0x18:_kernel_station

section .bss
        ;; Stack
                    resb 0x2000
global _BOOTSTRAP_STACK
_BOOTSTRAP_STACK:
