/**
 * @file atomics.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
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
#ifndef ARC_LIB_ATOMICS_H
#define ARC_LIB_ATOMICS_H

#include <stdatomic.h>
#include <stdbool.h>

/// Generic spinlock
typedef _Atomic int ARC_GenericSpinlock;

/// Generic mutex
typedef _Atomic int ARC_GenericMutex;

int Arc_MutexStaticInit(ARC_GenericMutex *mutex);
int Arc_MutexLock(ARC_GenericMutex *mutex);
int Arc_MutexUnlock(ARC_GenericMutex *mutex);

#endif
