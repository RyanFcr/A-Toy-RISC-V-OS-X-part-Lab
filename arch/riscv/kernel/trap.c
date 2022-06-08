#include "../include/syscall.h"

extern struct task_struct *current;
extern void create_mapping(uint64 *pgtbl2, uint64 va, uint64 pa, uint64 sz, int perm);

void trap_handler(uint64 scause, uint64 sepc, struct pt_regs *regs)
{
	// 通过 `scause` 判断trap类型
	// printk("%ld\n",scause);
	long signedscause = (long)scause;
	long cause = (scause << 1) >> 1;
	if (signedscause < 0)
	{
		if (cause == 5)
		{
			do_timer();
			clock_set_next_event();
		}
	}
	// exception:
	else
	{
		if (cause == 8)
		{
			switch (regs->reg[17])
			{
			case SYS_GETPID:
				long pid2 = sys_getpid();
				regs->sepc = regs->sepc + 4;
				regs->reg[10] = (uint64)pid2;
				break;
			case SYS_WRITE:
				char *buf = (char *)(regs->reg[11]);
				int count = (int)(regs->reg[12]);
				int num = sys_write(1, buf, count);
				regs->sepc = regs->sepc + 4;
				regs->reg[10] = (uint64)num;
				break;
			case SYS_FORK:
				uint64 pid1 = clone(regs);
				break;
			case SYS_READ:
				char *buf1 = (char *)(regs->reg[11]);
				size_t count1 = (size_t)(regs->reg[12]);
				int num1 = sys_read(1, buf1, count1);
				regs->sepc = regs->sepc + 4;
				regs->reg[10] = (uint64)num1;
				break;
			case SYS_EXIT:
				regs->sepc = regs->sepc + 4;
				sys_exit();
				break;
			case SYS_WAIT:
				regs->sepc = regs->sepc + 4;
				sys_wait((uintptr_t)regs->reg[10]);
				break;
			case SYS_EXECVE:
				sys_exec((char *)regs->reg[10]);
				regs->sepc = USER_START;
				break;
			}
		}
		else if (cause == 12 || cause == 13 || cause == 15)
		{
			do_page_fault(regs);
		}
	}
	return;
}

void do_page_fault(struct pt_regs *regs)
{
	/*
	1. 通过 stval 获得访问出错的虚拟内存地址（Bad Address）
	2. 通过 scause 获得当前的 Page Fault 类型
	3. 通过 find_vm() 找到对应的 vm_area_struct
	4. 通过 vm_area_struct 的 vm_flags 对当前的 Page Fault 类型进行检查
		4.1 Instruction Page Fault      -> VM_EXEC
		4.2 Load Page Fault             -> VM_READ
		4.3 Store Page Fault            -> VM_WRITE
	5. 最后调用 create_mapping 对页表进行映射
	*/
	uint64 stval_, scause_, sepc_;
	uint64 uapp_start = PHY_START + OPENSBI_SIZE + 0x5000;
	uint64 uapp_end = uapp_start + PGSIZE;
	csr_read(stval, &stval_);
	csr_read(scause, &scause_);
	csr_read(sepc, &sepc_);
	printk("[S] PAGE_FAULT: scause: 0x%lx, sepc: 0x%lx, badaddr: 0x%lx\n", scause_, sepc_, stval_);
	struct vm_area_struct *p = find_vma(current->mm, stval_);
	if (p == NULL)
	{
		printk("FAIL\n");
		return;
	}
	uint64 flags = p->vm_flags;
	if ((scause_ == 12 && ((flags & VM_EXEC) == 0)) || (scause_ == 13 && ((flags & VM_READ) == 0)) || (scause_ == 15 && ((flags & VM_WRITE) == 0)))
	{
		printk("DO NOT MATCH!\n");
		return;
	}
	flags = (flags << 1) | 0x11;
	if (stval_ >= USER_START && stval_ < USER_START + PGSIZE)
	{
		flags = (flags << 1) | 0x11;
		pagetable_t temppgd = (pagetable_t)((uint64)(current->pgd) + PA2VA_OFFSET);
		create_mapping(temppgd, stval_, uapp_start + stval_, PGSIZE, flags);
	}
	else
	{
		pagetable_t temppgd = (pagetable_t)((uint64)(current->pgd) + PA2VA_OFFSET);
		if (current->thread_info->user_sp == 0)
		{
			uint64 *area = (uint64 *)kalloc();
			create_mapping(temppgd, PGROUNDDOWN(stval_), (uint64)area - PA2VA_OFFSET, PGSIZE, flags);
		}
		else
		{
			create_mapping(temppgd, PGROUNDDOWN(stval_), current->thread_info->user_sp - PGSIZE, PGSIZE, flags);
		}
	}
}