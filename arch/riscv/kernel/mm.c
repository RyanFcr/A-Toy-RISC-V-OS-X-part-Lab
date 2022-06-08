
#include "mm.h"

extern char _ekernel[];

struct
{
    struct run *freelist;
} kmem;

uint64 kalloc()
{
    struct run *r;

    r = kmem.freelist;
    kmem.freelist = r->next;

    memset((void *)r, 0x0, PGSIZE);
    return (uint64)r;
}

void kfree(uint64 addr)
{
    struct run *r;

    // PGSIZE align
    addr = addr & ~(PGSIZE - 1);

    memset((void *)addr, 0x0, (uint64)PGSIZE);

    r = (struct run *)addr;
    r->next = kmem.freelist;
    kmem.freelist = r;

    return;
}

void kfreerange(char *start, char *end)
{
    char *addr = (char *)PGROUNDUP((uint64)start);
    for (; (uint64)(addr) + PGSIZE <= (uint64)end; addr += PGSIZE)
    {
        // uint64 *p = 0x84200000;
        // printk("%lx\n", *p);
        // if(*p==0xffffffe0041ff000)
        // {
        //     // printk("%lx\n", addr);
        //     break;
        // }    
        kfree((uint64)addr);

    }
}

void mm_init(void)
{
    printk("...mm_init begin!\n");
    kfreerange(_ekernel, (char *)(0xffffffe004200000));
    printk("...mm_init done!\n");
}
