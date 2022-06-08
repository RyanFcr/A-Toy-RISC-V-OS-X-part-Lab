#ifndef _EXEC_H
#define _EXEC_H
#include "syscall.h"

int loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint filesz);
#endif