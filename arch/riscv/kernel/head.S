.extern start_kernel

    .section .text.init
    .globl _start
_start:
    la sp, boot_stack_top
    call setup_vm
    call relocate
    
    la x5, _traps
    csrw stvec, x5
    csrr x6, sie
    ori x6, x6, 32
    csrw sie, x6
    call mm_init
    call setup_vm_final
    call task_init
    
    rdtime x7
    li x6, 10000000
    add x7, x7, x6
    add a0, x0, x7
    mv a1, x0
    mv a2, x0
    mv a3, x0
    mv a4, x0
    mv a5, x0
    mv a6, x0
    mv a7, x0
    ecall
    
    #csrr x5, sstatus
    #ori x5, x5, 2
    #csrw sstatus, x5
    j start_kernel
    
relocate:
    # set ra = ra + PA2VA_OFFSET(0xffffffdf80000000)
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    mv a1, x0
    mv a2, x0
    lui a1, 0xfffff
    addi a1, a1, 2043
    addi a1, a1, 2020
    slli a1, a1, 32
    li a2, 8
    slli a2, a2, 28
    add a1, a1, a2
    add ra, ra, a1
    add sp, sp, a1
    
    # set satp with early_pgtbl
    mv a1, x0
    lui a1, 0x80000
    slli a1, a1, 32
    la a2, early_pgtbl
    srli a2, a2, 12
    or a1, a1, a2
    csrw satp, a1
    
    # flush tlb
    sfence.vma zero, zero

    ret
    
    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 
    .globl boot_stack_top
boot_stack_top:
