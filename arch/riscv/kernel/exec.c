#include "exec.h"

int loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint filesz)
{
    // check whether va is aligned to PAGE_SIZE;
    // for (i = 0; i < filesz; i += PAGE_SIZE)
    // {
    //     walk the pagetable, get the corresponding pa of va;
    //     use readi() to read the content of segment to address pa;
    // }
    if(va & 0xFFF != 0){
        printk("VM is not aligned to PGSIZE!\n");
        return 0;
    }
    for(uint i=0; i < filesz; i=i+PGSIZE){ 
        uint64 VPN2 = (va >> 30) & 0x1FF;
        uint64 VPN1 = (va >> 21) & 0x1FF;
        uint64 VPN0 = (va >> 12) & 0x1FF;
        pagetable_t pgtbl1 = (pagetable_t)(((pagetable[VPN2] >> 10) << 12) + PA2VA_OFFSET);
        pagetable_t pgtbl0 = (pagetable_t)(((pgtbl1[VPN1] >> 10) << 12) + PA2VA_OFFSET);
        uint64 pa = (pgtbl0[VPN0] >> 10) << 12;
        int n;
        if(filesz - i < PGSIZE){
            n = filesz - i;
        }
        else n = PGSIZE;
        readi(ip, 0, (uint64)pa+PA2VA_OFFSET, offset + i, n);
    }
    return 0;
}