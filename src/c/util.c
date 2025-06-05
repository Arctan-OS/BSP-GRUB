/**
 * @file util.c
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
 * Utility functions.
*/
#include <util.h>

int strcmp(char *a, char *b) {
	uint8_t ca = (uint8_t)*a;
	uint8_t cb = (uint8_t)*b;

	while (ca == cb && ca != 0 && cb != 0) {
		a++;
		b++;
		ca = (uint8_t)*a;
		cb = (uint8_t)*b;
	}

	return ca - cb;
}

int memcpy(void *a, void *b, size_t size) {
	size_t i = 0;
	while (i < size) {
		*(uint8_t *)(a + i) = *(uint8_t *)(b + i);
		i++;
	}

	return 0;
}

int nmemcpy(void *a, void *b, size_t size) {
	size_t i = size - 1;
	while (i > 0) {
		*(uint8_t *)(a + i) = *(uint8_t *)(b + i);
		i--;
	}
	*(uint8_t *)(a) = *(uint8_t *)(b);

	return 0;
}

// word_size in bytes
// size in bytes
int wordcpy(void *a, void *b, size_t size, int word_size) {
	switch (word_size) {
	case 0: {
		return 0;
	}

	case 1: {
		return memcpy(a, b, size);
	}

	case 2: {
		size_t i = 0;
		while (i < size) {
			*(uint16_t *)(a + i) = *(uint16_t *)(b + i);
			i++;
		}

		return 0;
	}

	case 4: {
		size_t i = 0;
		while (i < size) {
			*(uint32_t *)(a + i) = *(uint32_t *)(b + i);
			i++;
		}

		return 0;
	}

	case 8: {
		size_t i = 0;
		while (i < size) {
			*(uint64_t *)(a + i) = *(uint64_t *)(b + i);
			i++;
		}

		return 0;
	}
	}

	return 0;
}


void memset(void *mem, uint8_t value, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*(uint8_t *)(mem + i) = value;
	}
}
