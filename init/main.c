#include "printk.h"
#include "sbi.h"
#include "defs.h"
#include "string.h"
#include "proc.h"

extern void test();

int start_kernel(){
	printk("[S-MODE] Hello RISCV!\n");
	schedule();
    test(); // DO NOT DELETE !!!

	return 0;
}
