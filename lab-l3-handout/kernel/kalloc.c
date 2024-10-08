// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

uint64 MAX_PAGES = 0;
uint64 FREE_PAGES = 0;

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
    struct run *next;
};

struct
{
    struct spinlock lock;
    struct run *freelist;
} kmem;

void kinit()
{
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
    MAX_PAGES = FREE_PAGES;
}

void freerange(void *pa_start, void *pa_end)
{
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    {
        kfree(p);
    }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
    if (MAX_PAGES != 0)
        assert(FREE_PAGES < MAX_PAGES);
    struct run *r;

    if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");
    
    
    //coundDOWN(pa);

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run *)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    FREE_PAGES++;
    release(&kmem.lock);

    
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
    assert(FREE_PAGES > 0);
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    release(&kmem.lock);

    if (r)
        memset((char *)r, 5, PGSIZE); // fill with junk
    FREE_PAGES--;

    // increment the refcount for r
    countUP((uint64)r);

    return (void *)r;
}

/* TODO */
// Implement a reference counter of the number of user page tables that refer to that page
// If the reference counter is 0, then the kfree() frees the page 

// metodene er en refcount som skal holde styr på antall user page table som refererer til en spesifikk page i minnet 
// skal frigjøre den fysiske siden når refcount er når (fordi da er den ingen som referer til den lengre)

// initiering av refc teller array
uint64 refc[(PHYSTOP - KERNBASE) / PGSIZE] = {0};


// øker teller for en gitt fysisk adresse pa
void countUP(uint64 pa){
    int i = (pa-KERNBASE) / PGSIZE;
    refc[i] += 1;
}

// senker teller for en gitt fysisk adresse pa
void countDOWN(void *pa){
    int i = ((uint64)(pa) - KERNBASE) / PGSIZE;

    if(refc[i] > 0){
        refc[i] -= 1;
    }
   
    //sjekker når refcount er 0 og frigjør den fysiske siden
    uint64 countRef = refc[i];
    if(countRef <= 0){
        kfree(pa);
    }
}