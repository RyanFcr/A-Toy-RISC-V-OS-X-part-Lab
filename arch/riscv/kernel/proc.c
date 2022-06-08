#include "proc.h"

extern void __dummy();
extern void __switch_to(struct task_struct *prev, struct task_struct *next);
extern unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern void create_mapping(uint64 *pgtbl2, uint64 va, uint64 pa, uint64 sz, int perm);
extern int pid;
extern int proc_exec(struct task_struct *proc, const char *path);

struct task_struct *idle; // idle process
struct task_struct *new;
uint64 *unew;
struct task_struct *current;  // 指向当前运行线程的 `task_struct`
struct task_struct *task[20]; // 线程数组，所有的线程都保存在此

void task_init()
{
	idle = (struct task_struct *)kalloc();
	idle->state = TASK_RUNNING;
	idle->counter = 0;
	idle->priority = 0;
	idle->pid = 0;
	current = idle;
	task[0] = idle;

	// 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
	// 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
	// 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
	// 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址

	for (int i = 1; i < NR_TASKS; i++)
	{
		new = (struct task_struct *)kalloc(); // s-stack
		unew = (uint64 *)kalloc();			  // u-stack
		struct thread_info *info = (struct thread_info *)kalloc();
		info->kernel_sp = (uint64) new + PGSIZE;
		info->user_sp = 0;
		new->parent = NULL;
		new->thread_info = info;
		new->state = TASK_RUNNING;
		new->counter = 0;
		new->priority = rand();
		new->pid = i;
		new->thread.ra = (uint64)(&__dummy);
		new->thread.sp = (uint64) new + PGSIZE;

		// new->thread.sepc = USER_START;
		// new->thread.sstatus = 0x40020; //SUM = 1, SPP = 0, SPIE = 1
		// new->thread.sscratch = USER_END;

		struct mm_struct *newmm = (struct mm_struct *)kalloc();
		struct vm_area_struct *head = (struct vm_area_struct *)kalloc();
		head->vm_start = head->vm_end = 0;
		head->vm_mm = newmm;
		head->vm_next = head->vm_prev = NULL;
		newmm->mmap = head;
		new->mm = newmm;
		// printk("1\n");
		proc_exec(new, "shell");

		// new->thread.sstatus = TASK_SLEEPING;
		// pagetable_t temppgd = (pagetable_t)kalloc();
		// for(int i=0; i<512; i++){
		// 	temppgd[i] = swapper_pg_dir[i];
		// }

		// do_mmap(new->mm, USER_START, PGSIZE, VM_READ|VM_WRITE|VM_EXEC);
		// do_mmap(new->mm, USER_END-PGSIZE, PGSIZE, VM_READ|VM_WRITE);
		// new->pgd = (pagetable_t)((uint64)temppgd - PA2VA_OFFSET);
		task[i] = new;
	}
	printk("...proc_init done!\n");
}
void dummy()
{
	uint64 MOD = 1000000007;
	uint64 auto_inc_local_var = 0;
	int last_counter = -1;
	while (1)
	{
		if (last_counter == -1 || current->counter != last_counter)
		{
			last_counter = current->counter;
			auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
			printk("[PID = %d] is running. thread space begin at %lx\n", current->pid, current);
		}
	}
}
void switch_to(struct task_struct *next)
{
	if (next == 0)
		printk("ERROR!!!");
	if (current != next)
	{
		printk("\nswitch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
		struct task_struct *t = current;
		current = next;
		__switch_to(t, next);
	}
}

void do_timer(void)
{
	/* 1. 如果当前线程是 idle 线程 或者 当前线程运行剩余时间为0 进行调度 */
	/* 2. 如果当前线程不是 idle 且 运行剩余时间不为0 则对当前线程的运行剩余时间减1 直接返回 */

	/* YOUR CODE HERE */
	if (current->pid == 0 || current->counter == 0)
	{
		printk("schedule!!!!\n");
		schedule();
	}
	else if (current->pid != 0 && current->counter != 0)
	{
		printk("pid = %d is running, counter is %d\n", current->pid,current->counter);
		current->counter = (current->counter) - 1;
		return;
	}
}
#ifdef PRIORITY
void schedule(void)
{
	struct task_struct **p;
	struct task_struct *next;
	int c, i;
	while (1)
	{
		c = -1;
		next = 0;
		i = NR_TASKS;
		p = &task[NR_TASKS];
		while (--i)
		{
			if (!*--p)
				continue;
			if ((*p)->state == TASK_RUNNING && (long)((*p)->counter) > c)
			{
				c = (*p)->counter;
				next = task[i];
			}
		}
		if (c)
			break;
		for (p = &task[NR_TASKS - 1]; p > &task[0]; --p)
			if (*p)
			{
				(*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
				printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", (*p)->pid, (*p)->priority, (*p)->counter);
			}
	}
	switch_to(next);
}
#endif
#ifdef SJF
void schedule(void)
{
	struct task_struct *next;
	int i;
	int min = 9999999, index = 0;
	if (task == NULL)
	{
		printk("ERROR!\n");
	}
	int cnt = 0;
	min = 9999999;
	for (i = 1; i < pid; i++)
	{
		if (task[i]->counter == 0 ||task[i]->state == TASK_SLEEPING)
			cnt++;
	}
	if (cnt == pid - 1)
	{
		for (i = 1; i < pid; i++)
		{
			if (task[i]->state == TASK_RUNNING)
			{
				// task[i]->state = TASK_RUNNING;
				task[i]->counter = rand();
				printk("SET [PID = %d COUNTER = %d]\n", i, task[i]->counter);
			}
		}
	}
	for (i = 1; i < pid; i++)
	{
		if ((task[i]->counter) < min && task[i]->counter != 0 && task[i]->state == TASK_RUNNING)
		{
			
			min = task[i]->counter;
			next = task[i];
			index = i;
		}
	}
	// task[index]->state = TASK_RUNNING;
	next = task[index];

	switch_to(next);
}
#endif

/*
 * @mm          : current thread's mm_struct
 * @address     : the va to look up
 *
 * @return      : the VMA if found or NULL if not found
 */
struct vm_area_struct *find_vma(struct mm_struct *mm, uint64 addr)
{
	struct vm_area_struct *p = mm->mmap;
	while (p != NULL)
	{
		if ((addr >= p->vm_start) && (addr < p->vm_end))
		{
			return p;
		}
		p = p->vm_next;
	}
	return NULL;
}
/*
 * @mm     : current thread's mm_struct
 * @addr   : the suggested va to map
 * @length : memory size to map
 * @prot   : protection
 *
 * @return : start va
 */
uint64 do_mmap(struct mm_struct *mm, uint64 addr, uint64 length, int prot)
{
	uint64 begin = addr, end = addr + length;
	struct vm_area_struct *p = mm->mmap;
	struct vm_area_struct *q = p->vm_next;
	while (q != NULL)
	{
		if (addr >= p->vm_start && addr < q->vm_start && p != mm->mmap)
			break;
		p = p->vm_next;
		q = q->vm_next;
	}
	if (q == NULL)
	{
		struct vm_area_struct *new = (struct vm_area_struct *)kalloc();
		new->vm_mm = mm;
		new->vm_flags = (uint64)prot;
		new->vm_start = addr;
		new->vm_end = end;
		new->vm_next = q;
		p->vm_next = new;
		new->vm_prev = p;
	}
	else if (p->vm_end <= addr && q->vm_start >= end)
	{
		struct vm_area_struct *new = (struct vm_area_struct *)kalloc();
		new->vm_mm = mm;
		new->vm_flags = (uint64)prot;
		new->vm_start = addr;
		new->vm_end = end;
		new->vm_next = q;
		q->vm_prev = new;
		p->vm_next = new;
		new->vm_prev = p;
	}
	else
	{
		addr = get_unmapped_area(mm, length);
		uint64 res = do_mmap(mm, addr, length, prot);
	}
}

uint64 get_unmapped_area(struct mm_struct *mm, uint64 length)
{
	uint64 addr = 0;
	while (1)
	{
		struct vm_area_struct *p = mm->mmap;
		int flag = 1;
		while (p != NULL)
		{
			if (!((addr >= p->vm_end) || (addr + length <= p->vm_start)))
			{
				flag = 0;
				break;
			}
			p = p->vm_next;
		}
		if (flag == 1)
			return addr;
		addr += PGSIZE;
	}
}