    .section .text.entry
    .align 2
    .globl _traps 
_traps:
    # compare sscratch with 0 to check whether it's a S-mode thread
    csrr x5, sscratch
    beq x5, x0, Save
    csrr x5, sscratch
    csrw sscratch, sp
    mv sp, x5
    # 1. save 32 registers and sepc to stack
Save:
    addi sp, sp, -280
    SD x0, 0(sp)
    SD x1, 8(sp)
    SD x2, 16(sp)
    SD x3, 24(sp)
    SD x4, 32(sp)
    SD x5, 40(sp)
    SD x6, 48(sp)
    SD x7, 56(sp)
    SD x8, 64(sp)
    SD x9, 72(sp)
    SD x10, 80(sp)
    SD x11, 88(sp)
    SD x12, 96(sp)
    SD x13, 104(sp)
    SD x14, 112(sp)
    SD x15, 120(sp)
    SD x16, 128(sp)
    SD x17, 136(sp)
    SD x18, 144(sp)
    SD x19, 152(sp)
    SD x20, 160(sp)
    SD x21, 168(sp)
    SD x22, 176(sp)
    SD x23, 184(sp)
    SD x24, 192(sp)
    SD x25, 200(sp)
    SD x26, 208(sp)
    SD x27, 216(sp)
    SD x28, 224(sp)
    SD x29, 232(sp)
    SD x30, 240(sp)
    SD x31, 248(sp)
    csrr a0, sepc
    SD a0, 256(sp)
    csrr a0, sstatus
    SD a0, 264(sp)
    csrr a0, sscratch
    SD a0, 272(sp)
        
    # 2. call trap_handler
    csrr a0, scause
    csrr a1, sepc
    mv a2, sp
    call trap_handler
    # 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
    LD a0, 272(sp)
    csrw sscratch, a0
    LD a0, 264(sp)
    csrw sstatus, a0
    LD a0, 256(sp)
    csrw sepc, a0
    LD x0, 0(sp)
    LD x1, 8(sp)
    LD x3, 24(sp)
    LD x4, 32(sp)
    LD x5, 40(sp)
    LD x6, 48(sp)
    LD x7, 56(sp)
    LD x8, 64(sp)
    LD x9, 72(sp)
    LD x10, 80(sp)
    LD x11, 88(sp)
    LD x12, 96(sp)
    LD x13, 104(sp)
    LD x14, 112(sp)
    LD x15, 120(sp)
    LD x16, 128(sp)
    LD x17, 136(sp)
    LD x18, 144(sp)
    LD x19, 152(sp)
    LD x20, 160(sp)
    LD x21, 168(sp)
    LD x22, 176(sp)
    LD x23, 184(sp)
    LD x24, 192(sp)
    LD x25, 200(sp)
    LD x26, 208(sp)
    LD x27, 216(sp)
    LD x28, 224(sp)
    LD x29, 232(sp)
    LD x30, 240(sp)
    LD x31, 248(sp)
    LD x2, 16(sp)
   	addi sp, sp, 280
    csrr x5, sscratch
    beq x5, x0, trap_ret
    csrr x5, sscratch
    csrw sscratch, sp
    mv sp, x5
trap_ret:
    sret 

    .globl __dummy 
__dummy:
    csrr x5, sscratch
    csrw sscratch, sp
    mv sp, x5
	sret
	.globl __switch_to
__switch_to:

	addi a4, x0, 40
	add a3, a0, a4
	add a4, a1, a4
	SD ra, 0(a3)
	SD sp, 8(a3)
	SD s0, 16(a3)
	SD s1, 24(a3)
	SD s2, 32(a3)
	SD s3, 40(a3)
	SD s4, 48(a3)
	SD s5, 56(a3)
	SD s6, 64(a3)
	SD s7, 72(a3)
	SD s8, 80(a3)
	SD s9, 88(a3)
	SD s10, 96(a3)
	SD s11, 104(a3)
    csrr x5, sepc
    SD x5, 112(a3)
    csrr x5, sstatus
    SD x5, 120(a3)
    csrr x5, sscratch
    SD x5, 128(a3) 
    csrr x5, satp
    slli x5, x5, 20
    srli x5, x5, 8
    SD x5, 136(a3)
	
	LD ra, 0(a4)
	LD sp, 8(a4)
	LD s0, 16(a4)
	LD s1, 24(a4)
	LD s2, 32(a4)
	LD s3, 40(a4)
	LD s4, 48(a4)
	LD s5, 56(a4)
	LD s6, 64(a4)
	LD s7, 72(a4)
	LD s8, 80(a4)
	LD s9, 88(a4)
	LD s10, 96(a4)
	LD s11, 104(a4)
    LD x5, 112(a4)
    csrw sepc, x5
    LD x5, 120(a4)
    csrw sstatus, x5
    LD x5, 128(a4)
    csrw sscratch, x5
    LD x5, 136(a4)
    srli x5, x5, 12
    lui x6, 0x80000
    slli x6, x6, 32
    or x5, x5, x6
    csrw satp, x5
    sfence.vma zero, zero
	ret             
	
    .globl ret_from_fork
ret_from_fork:
    LD x5, 256(a0)
    csrw sepc, x5
    LD x0, 0(a0)
    LD x1, 8(a0)
    LD x2, 16(a0)
    LD x3, 24(a0)
    LD x4, 32(a0)
    LD x5, 40(a0)
    LD x6, 48(a0)
    LD x7, 56(a0)
    LD x8, 64(a0)
    LD x9, 72(a0)
    LD x11, 88(a0)
    LD x12, 96(a0)
    LD x13, 104(a0)
    LD x14, 112(a0)
    LD x15, 120(a0)
    LD x16, 128(a0)
    LD x17, 136(a0)
    LD x18, 144(a0)
    LD x19, 152(a0)
    LD x20, 160(a0)
    LD x21, 168(a0)
    LD x22, 176(a0)
    LD x23, 184(a0)
    LD x24, 192(a0)
    LD x25, 200(a0)
    LD x26, 208(a0)
    LD x27, 216(a0)
    LD x28, 224(a0)
    LD x29, 232(a0)
    LD x30, 240(a0)
    LD x31, 248(a0)
    LD x10, 80(a0)
rff_ret:
    sret


