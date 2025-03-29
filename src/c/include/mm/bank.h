/**
 * @file bank.h
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
#ifndef ARC_MM_BANK_H
#define ARC_MM_BANK_H

#include <mm/freelist.h>
#include <stddef.h>

#define ARC_MM_BANK_TYPE_NONE 0
#define ARC_MM_BANK_TYPE_PFREELIST 1
#define ARC_MM_BANK_TYPE_PSLAB 2
#define ARC_MM_BANK_TYPE_VBUDDY 3

struct ARC_BankNode {
        void *allocator_meta __attribute__((aligned(8)));
        struct ARC_BankNode *next __attribute__((aligned(8)));
};

struct ARC_BankMeta {
        struct ARC_BankNode *first __attribute__((aligned(8)));
	void *(*alloc)(size_t size) __attribute__((aligned(8)));
	size_t (*free)(void *address) __attribute__((aligned(8)));
        int type;
};

int bank_add_pfreelist(struct ARC_BankMeta *meta, struct ARC_FreelistMeta *allocator);
int bank_remove_pfreelist(struct ARC_BankMeta *meta, struct ARC_FreelistMeta *allocator);

int init_static_bank(struct ARC_BankMeta *meta, int type);


#endif