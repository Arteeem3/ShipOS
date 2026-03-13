//
// Created by ShipOS developers on 28.10.23.
// Copyright (c) 2023 SHIPOS. All rights reserved.
//

#include "spinlock.h"
#include "../sched/percpu.h"

void init_spinlock(struct spinlock *lock, char *name) {
    lock->is_locked = 0;
    lock->name = name;
}

void acquire_spinlock(struct spinlock *lk) {
    // Disable interrupts to avoid deadlock
    pushcli(); 

    // The xchg is atomic.
    while (xchg(&lk->is_locked, 1) != 0) {
        // Spin and wait for the lock to be released
    }

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that the critical section's memory
    // references happen after the lock is acquire_spinlockd.
    __sync_synchronize();
}

void release_spinlock(struct spinlock *lk) {
    if (!holding_spinlock(lk)) {
        panic("release_spinlock: lock is not held");
    }

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that all the stores in the critical
    // section are visible to other cores before the lock is release_spinlockd.
    __sync_synchronize();

    // Release the lock, equivalent to lk->is_locked = 0.
    // We use inline assembly to guarantee that the compiler doesn't 
    // translate this into multiple non-atomic instructions.
    asm volatile("movl $0, %0" : "+m" (lk->is_locked) : );

    popcli();
}

int holding_spinlock(struct spinlock *lock) {
    int r;
    pushcli();
    r = lock->is_locked;
    popcli();
    return r;
}
