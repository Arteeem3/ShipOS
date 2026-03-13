#include "../kalloc/kalloc.h"
#include "../sync/spinlock.h"
#include "../memlayout.h"
#include "../lib/include/memset.h"
#include <inttypes.h>
#include <stddef.h>
#include "../lib/include/logging.h"

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    struct run *freelist;
} kmem;


void kinit(uint64_t start, uint64_t stop) {
    // Initialize the lock before any pages are pushed to the stack
    init_spinlock(&kmem.lock, "kmem");
    
    char *p;
    p = (char *) PGROUNDUP(start);
    for (; p + PGSIZE < stop; p += PGSIZE) {
        kfree(p);
    }
}

void kfree(void *pa) {
    struct run *r;

    if (((uint64_t) pa % PGSIZE) != 0 || (char *) pa < end || (uint64_t) pa >= PHYSTOP) {
        LOG("Panic while trying to free memory\nPA: %p END: %p PHYSTOP: %p", pa, end, PHYSTOP);
        panic("kfree");
    }

    // Fill with junk to catch dangling references
    memset(pa, 0, PGSIZE);

    r = (struct run *) pa;

    // Push the page onto the LIFO stack securely using a spinlock
    acquire_spinlock(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release_spinlock(&kmem.lock);
}

void *kalloc() {
    struct run *r;

    // Pop the top page from the LIFO stack securely
    acquire_spinlock(&kmem.lock);
    r = kmem.freelist;
    if (r) {
        kmem.freelist = r->next;
    }
    release_spinlock(&kmem.lock);

    if (r) {
        // Fill with junk to ensure the caller doesn't rely on previous content
        memset((char *) r, 5, PGSIZE); 
    }
    
    return (void *) r;
}

uint64_t count_pages() {
    struct run *r;
    uint64_t res = 0;

    // Traverse the stack safely
    acquire_spinlock(&kmem.lock);
    r = kmem.freelist;
    while (r != 0) {
        res++;
        r = r->next;
    }
    release_spinlock(&kmem.lock);

    LOG("%d pages available in allocator stack", res);
    return res;
}