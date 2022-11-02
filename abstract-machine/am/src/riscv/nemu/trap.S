#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((32 + 4 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  (32 * XLEN)
#define OFFSET_STATUS (33 * XLEN)
#define OFFSET_EPC    (34 * XLEN)
#define OFFSET_NP			(35 * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:
	// c->sp = sp has been set in ucontext and kcontext
	STORE sp, (-CONTEXT_SIZE + OFFSET_SP)(sp)	
	// swap sp, ksp
	csrrw sp, mscratch, sp
	// if ksp!=0, fix swap
	bnez sp, save_context
	// if ksp==0, swap back
	csrrw sp, mscratch, sp

	// c->np = kstack.end if USER else 0
save_context:
	// now sp is pointing to the kernel stack
	// now ksp is kstack.end if USER else 0
	addi sp, sp, -CONTEXT_SIZE
	
  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
	
	csrr t0, mscratch
	STORE t0, OFFSET_NP(sp)
	csrw mscratch, zero

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle
	mv sp, a0
	
	LOAD t0, OFFSET_NP(sp)// if c->np=0, ksp keeps 0, else ksp = c->np = sp
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
	csrw mscratch, t0
  csrw mstatus, t1
  csrw mepc, t2

  MAP(REGS, POP)
	
	addi sp, sp, CONTEXT_SIZE

	csrrw sp, mscratch, sp
	bnez sp, return
	csrrw sp, mscratch, sp
return:
	LOAD sp, (-CONTEXT_SIZE + OFFSET_SP)(sp)
  mret
