//
// Created by ShipOS developers on 28.10.23.
// Copyright (c) 2023 SHIPOS. All rights reserved.
//

#ifndef UNTITLED_OS_SPINLOCK_H
#define UNTITLED_OS_SPINLOCK_H

#include <inttypes.h>
#include "../sched/proc.h"
#include "../lib/include/x86_64.h"

struct spinlock {
    // Changed to uint32_t to safely match the 'movl' assembly instruction and standard x86 32-bit atomic operations.
    uint32_t is_locked; 
    char *name;
};

void init_spinlock(struct spinlock *lock, char *name);

void acquire_spinlock(struct spinlock *lk);

void release_spinlock(struct spinlock *lk);

int holding_spinlock(struct spinlock *lock);

#endif //UNTITLED_OS_SPINLOCK_H
