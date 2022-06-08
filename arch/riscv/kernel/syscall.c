#include "../include/syscall.h"

extern struct task_struct *current;
extern struct task_struct *task[NR_TASKS];
extern unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern void ret_from_fork(struct pt_regs *trapframe);
int pid = 2;

int sys_write(uint64 fd, const char *buf, size_t count)
{
    long syscall_ret = 0;

    for (int i = 0; i < count; i++)
    {
        syscall_ret += printk("%c", buf[i]);
    }
    return syscall_ret;
}

long sys_getpid()
{
    long ret;
    ret = (long)(current->pid);
    return ret;
}

uint64 clone(struct pt_regs *regs)
{
    return do_fork(regs);
}

uint64 do_fork(struct pt_regs *regs)
{

    struct task_struct *new = (struct task_struct *)kalloc();
    uint64 *ustack = (uint64 *)kalloc();
    uint64 *oldstack = (uint64 *)(PGROUNDDOWN(regs->sscratch));
    struct thread_info *info = (struct thread_info *)kalloc();
    info->user_sp = (uint64)ustack - PA2VA_OFFSET + PGSIZE;
    new->thread_info = info;

    for (int i = 0; i < 512; i++)
    {
        ustack[i] = oldstack[i];
    }
    new->state = TASK_RUNNING;
    new->counter = 0;
    new->priority = rand();
    new->pid = pid;
    new->thread.ra = (uint64)(&forkret);
    new->thread.sp = new->thread.sscratch = (uint64) new + PGSIZE;
    new->thread.sepc = regs->sepc;
    new->thread.sstatus = (uint64)0x40020;
    new->parent = current; // parent is the current task

    pagetable_t temppgd = (pagetable_t)kalloc();

    pagetable_t t = (pagetable_t)((uint64)(current->pgd) + PA2VA_OFFSET);
    for (int i = 0; i < 512; i++)
    {
        temppgd[i] = t[i];
    }
    new->pgd = (pagetable_t)((uint64)temppgd - PA2VA_OFFSET);

    new->mm = (struct mm_struct *)kalloc();
    new->mm->mmap = (struct vm_area_struct *)kalloc();
    new->mm->mmap->vm_mm = new->mm;
    new->mm->mmap->vm_start = new->mm->mmap->vm_end = 0;
    new->mm->mmap->vm_next = new->mm->mmap->vm_prev = NULL;
    struct vm_area_struct *p = current->mm->mmap->vm_next;
    struct vm_area_struct *tail = new->mm->mmap;

    while (p != NULL)
    {
        struct vm_area_struct *node = (struct vm_area_struct *)kalloc();
        node->vm_mm = new->mm;
        node->vm_prev = tail;
        node->vm_next = NULL;
        node->vm_start = p->vm_start;
        node->vm_end = p->vm_end;
        node->vm_flags = p->vm_flags;
        p = p->vm_next;
        tail->vm_next = node;
        tail = tail->vm_next;
    }

    new->trapframe = (struct pt_regs *)kalloc();
    for (int i = 0; i < 32; i++)
    {
        new->trapframe->reg[i] = regs->reg[i];
    }
    new->trapframe->reg[2] = regs->sscratch; // user-sp
    new->trapframe->reg[10] = 0;
    new->trapframe->sepc = regs->sepc + 4;
    new->trapframe->sscratch = regs->sscratch;
    new->trapframe->sstatus = regs->sstatus;

    regs->reg[10] = pid;
    regs->sepc = regs->sepc + 4;
    task[pid++] = new;
    return pid - 1;
}

void forkret()
{
    ret_from_fork(current->trapframe);
}

// 同学们可直接复制sys_exec()以及exec()
uint64 sys_exec(const char *path)
{
    int ret = exec(path);
    return ret;
}

int exec(const char *path)
{

    int ret = proc_exec(current, path);
    return ret;
}

// 需要同学们实现proc_exec函数
// 成功返回0，失败返回-1
// void *memset(void *s, int c, size_t n)
// {
//     const unsigned char uc = c;
//     unsigned char *su;
//     for (su = s; 0 < n; ++su, --n)
//         *su = uc;
//     return s;
// }
// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
// int uvmalloc(pagetable_t *pgtbl, uint oldsz, uint newsz)
// {
//     if (newsz < oldsz)
//         return oldsz;
//     uint64 a =  PGROUNDUP(oldsz);
//     for (; a < newsz; a += PGSIZE)
//     {
//         uint64 *mem = (uint64 *)kalloc();
//         if (mem == 0)
//         {
//             printk("uvmalloc out of memory");
//             return 0;
//         }
//         memset(mem, 0, PGSIZE);
//         create_mapping(pgtbl, a, a - PA2VA_OFFSET, PGSIZE, 0b10111);
//     }
//     return newsz;
// }
int proc_exec(struct task_struct *proc, const char *path)
{
    // printk("welcome to proc_exec\n");
    struct inode *inode_ = namei(path); // namei函数里已经kalloc()，初始化inode_
    // inode = namei(path);

    pagetable_t newpgtbl = (pagetable_t)kalloc();
    for (int i = 0; i < 512; i++)
    {
        newpgtbl[i] = swapper_pg_dir[i];
    }
    struct elfhdr *elf_header = (struct elfhdr *)kalloc();
    // create a new page table newpgtbl, copy kernel page table from kpgtbl(same as lab5);
    if (readi(inode_, 0, (char *)elf_header, 0, sizeof(*elf_header)) != sizeof(*elf_header)) // read the content of segment到elf_header
        return -1;
    // Attention：elf_header需不需要类型转换？转换成char* 还是uint64*
    // readi() read the elf header(denoted as elf_header) from elf file;
    if (elf_header->magic != ELF_MAGIC)
        return -1;
    // check whether elf_header.magic == ELF_MAGIC;
    struct proghdr *prog_header = (struct proghdr *)kalloc();
    uint64 off = elf_header->phoff;
    int sz = 0;
    for (int i = 0; i < elf_header->phnum; i++, off += sizeof(*prog_header))
    {
        readi(inode_, 0, (char *)prog_header, off, sizeof(*prog_header)); //读到prog_header
        if (prog_header->type != LOAD)
            return -1;
        int r = VM_READ & prog_header->flags;
        int w = VM_WRITE & prog_header->flags;
        int e = VM_EXEC & prog_header->flags;
        int flag = r | w | e;
        // create_mapping(newpgtbl,)

        do_mmap(proc->mm, prog_header->vaddr, PGROUNDUP(prog_header->memsz), flag);

        for (uint64 a = prog_header->vaddr; a < prog_header->vaddr + PGROUNDUP(prog_header->memsz); a += PGSIZE)
        {
            create_mapping(newpgtbl, a, (uint64)kalloc() - PA2VA_OFFSET, PGSIZE, 0b10001 | (flag << 1));
        }
        // create_mapping(newpgtbl, prog_header->vaddr, (uint64)kalloc() - PA2VA_OFFSET, PGROUNDUP(prog_header->memsz), 0b10001 | (flag << 1));
        // create_mapping(newpgtbl,prog_header->vaddr+PGSIZe,(uint64)kalloc()-PA2VA_OFFSET,)
        // sz = uvmalloc(newpgtbl, sz, prog_header->vaddr + prog_header->memsz);

        loadseg(newpgtbl, prog_header->vaddr, inode_, prog_header->off, prog_header->filesz);
    }
    // for (i = 0; i < elf_header.phnum; i++)
    // {
    //     readi() read the program header(denoted as prog_header) of each segment;
    //     check whether prog_header.type == LOAD;
    //     parse_ph_flags() parse the prog_header.flags to permissions;
    //     uvmalloc() allocate user pages for [prog_header.vaddr, prog_header.vaddr+prog_header.memsz] of this segment in newpgtbl, and set proper permissions;
    //     loadseg() copy the content of this segment from prog_header.off to its just allocated memory space;
    // }
    unsigned long *user_stack = (unsigned long *)kalloc(); //申请空页面作为user_stack
    do_mmap(proc->mm, USER_END - PGSIZE, PGSIZE, VM_READ | VM_WRITE);
    create_mapping(newpgtbl, USER_END - PGSIZE, (unsigned long)user_stack - PA2VA_OFFSET, PGSIZE, 0b10001 | (VM_READ << 1) | (VM_WRITE << 1));
    proc->pgd = (pagetable_t)((uint64)newpgtbl - PA2VA_OFFSET);
    proc->thread.sepc = USER_START; // elf_header.entry
    proc->thread.sscratch = USER_END;
    proc->thread.sstatus = 0x40020; // SUM = 1, SPP = 0, SPIE = 1

    unsigned long temp = (unsigned long)newpgtbl - PA2VA_OFFSET;
    temp = (unsigned long)temp >> 12;                      // PPN
    temp = (0x000fffffffffff & temp) | 0x8000000000000000; // mode是8，ASID置0

    csr_write(satp, temp);
    asm volatile("sfence.vma zero, zero");

    // allocate a page for user stack and update the newpgtbl for it. The user stack va range: [USER_END-PAGE_SIZE, USER_END];
    // set proc's sstatus, sscratch, and sepc like task_init() in Lab5; set proc->pgtbl to newpgtbl;
    return 0;
}
int sys_wait(uintptr_t *regs)
{
    // set self as pending state;
    // schedule;
    // printk("welcome to sys_wait\n");
    current->state = TASK_SLEEPING;
    schedule();
    return 0;
}
void sys_exit()
{
    // while (true)
    // {
    //     if (parent is pending)
    //         break;
    //     else
    //         schedule to give up cpu;
    // }
    // set parent as running state;
    // schedule to give up cpu;
    for (int i = 1; i < pid; i++)
    {
        if (task[i] == current)
        {
            task[i] = NULL;
        }
    }
    current->parent->state = TASK_RUNNING;
    schedule();
}

int sys_read(uint64 fd, char *buf, size_t count)
{
    struct sbiret ret;
    int i;
    // printk("welcome to sys_read\n");
    for (i = 0; i < count; i++)
    {
        ret = sbi_ecall(SBI_GETCHAR, 0, 0, 0, 0, 0, 0, 0);
        buf[i] = ret.value;
    }
    // printk("read finish\n");
    // buf[count] = '\0';
    return i;
}
