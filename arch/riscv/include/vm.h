#ifndef _VM_H
#define _VM_H

#include "defs.h"
#include "../../../include/string.h"
#include "../../../include/stddef.h"
#include "mm.h"
#include "../../../include/printk.h"
#include "../../../include/types.h"

void setup_vm(void);
void setup_vm_final(void);
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);

#endif