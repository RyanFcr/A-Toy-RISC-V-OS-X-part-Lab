#pragma once

#include "proc.h"
#include "sbi.h"
#include "vm.h"
#include "elf.h"
#include "fs.h"
#include "stddef.h"
#include "mm.h"
#include "rand.h"


#define SYS_OPENAT 56
#define SYS_CLOSE 57
#define SYS_READ 63 //加
#define SYS_WRITE 64
#define SYS_EXIT 93 //加
#define SYS_GETPID 172
#define SYS_MUNMAP 215
#define SYS_FORK 220 // clone
#define SYS_EXECVE 221
#define SYS_MMAP 222
#define SYS_MPROTECT 226
#define SYS_WAIT 260 // wait4 加

struct pt_regs
{
    uint64 reg[32];
    uint64 sepc;
    uint64 sstatus;
    uint64 sscratch;
};

int sys_write(uint64 fd, const char *buf, size_t count);

int sys_read(uint64 fd, char *buf, size_t count);

long sys_getpid();

void forkret();

uint64 do_fork(struct pt_regs *regs);

uint64 clone(struct pt_regs *regs);

uint64 sys_exec(const char *path);

int exec(const char *path);

int proc_exec(struct task_struct *proc, const char *path);

int sys_wait(uintptr_t *regs);

void sys_exit();
