#include "vm.h"
/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void)
{
    /*
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
    uint64 VA = PHY_START + PA2VA_OFFSET;
    uint64 PA = PHY_START;
    uint64 VPN = (VA >> 30) & 0x1FF;
    uint64 PPN = PA >> 30;
    early_pgtbl[PPN] = (PPN << 28) | 0xF;
    early_pgtbl[VPN] = (PPN << 28) | 0xF;
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */

unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

void setup_vm_final(void)
{
    memset(swapper_pg_dir, 0x0, PGSIZE);

    // No OpenSBI mapping required
    uint64 pa = PHY_START + OPENSBI_SIZE;
    uint64 va = pa + PA2VA_OFFSET;
    uint64 text_sz = 0x4000;
    uint64 rodata_sz = 0x1000;
    uint64 other_sz = PHY_SIZE - text_sz - rodata_sz - OPENSBI_SIZE;
    uint64 initrd_start_pa = PHY_START + 0x04200000UL;
    uint64 initrd_start_va = VM_START + 0x04200000UL;

    // mapping kernel text X|-|R|V
    create_mapping(swapper_pg_dir, va, pa, text_sz, 11);
    pa += text_sz;
    va += text_sz;
    // mapping kernel rodata -|-|R|V
    create_mapping(swapper_pg_dir, va, pa, rodata_sz, 3);
    pa += rodata_sz;
    va += rodata_sz;
    // mapping other memory -|W|R|V
    create_mapping(swapper_pg_dir, va, pa, other_sz, 7);
    create_mapping(swapper_pg_dir, initrd_start_va-2*PGSIZE, initrd_start_pa-2*PGSIZE, 3*PGSIZE, 15);
    // set satp with swapper_pg_dir
    asm volatile("mv a1, x0");
    asm volatile("mv a2, x0");
    asm volatile("lui a1, 0xfffff");
    asm volatile("addi a1, a1, 2043");
    asm volatile("addi a1, a1, 2020");
    asm volatile("slli a1, a1, 32");
    asm volatile("li a2, 8");
    asm volatile("slli a2, a2, 28");
    asm volatile("add a1, a1, a2");
    asm volatile("la a2, swapper_pg_dir");
    asm volatile("sub a2, a2, a1");
    asm volatile("srli a2, a2, 12");
    asm volatile("li a1, 0");
    asm volatile("lui a1, 0x80000");
    asm volatile("slli a1, a1, 32");
    asm volatile("or a1, a1, a2");
    asm volatile("csrw satp, a1");

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    printk("...setup_vm_final done!\n");
    return;
}

/* 创建多级页表映射关系 */

void create_mapping(uint64 *pgtbl2, uint64 va, uint64 pa, uint64 sz, int perm)
{
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    int i;
    if (pgtbl2 == NULL)
        printk("ERROR!!!\n");
    uint64 VPN2, VPN1, VPN0, PPN2, PPN1, PPN0;
    unsigned long *pgtbl1 = NULL;
    unsigned long *pgtbl0 = NULL;
    for (i = 0; i < sz; i += 0x1000)
    {

        VPN2 = (va >> 30) & 0x1FF;
        VPN1 = (va >> 21) & 0x1FF;
        VPN0 = (va >> 12) & 0x1FF;
        if ((pgtbl2[VPN2] & 0x1) == 0)
        {
            pgtbl1 = (unsigned long *)kalloc();
            PPN2 = (((uint64)pgtbl1 - PA2VA_OFFSET) >> 12);
            pgtbl2[VPN2] = (PPN2 << 10) | 0x1;
        }
        else if (pgtbl1 == NULL)
        {
            pgtbl1 = (uint64 *)(((pgtbl2[VPN2] >> 10) << 12) + PA2VA_OFFSET);
        }
        if ((pgtbl1[VPN1] & 0x1) == 0)
        {
            pgtbl0 = (unsigned long *)kalloc();
            PPN1 = (((uint64)pgtbl0 - PA2VA_OFFSET) >> 12);
            pgtbl1[VPN1] = (PPN1 << 10) | 0x1;
        }
        else if (pgtbl0 == NULL)
        {
            pgtbl0 = (uint64 *)(((pgtbl1[VPN1] >> 10) << 12) + PA2VA_OFFSET);
        }
        PPN0 = pa >> 12;
        pgtbl0[VPN0] = (PPN0 << 10) | perm;
        va += 0x1000;
        pa += 0x1000;
    }
}
