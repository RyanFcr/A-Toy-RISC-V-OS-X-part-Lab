#ifndef _MM_H
#define _MM_H
#include "types.h"
#include "defs.h"
#include "string.h"
#include "printk.h"

struct run
{
    struct run *next;
};

void mm_init();

uint64 kalloc();
void kfree(uint64);
#endif
