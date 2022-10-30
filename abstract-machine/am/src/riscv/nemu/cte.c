#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
	/*
	printf("CONTEXT, mepc:%d, mcause: %d, mstatus: %d\nREGS:\n",c->mepc,c->mcause,c->mstatus);
  */
	if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
						case EVENT_YIELD:	ev.event = c->GPR1==-1 ? EVENT_YIELD:EVENT_SYSCALL; break;
				default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
	Context *c = (Context *)(kstack.end - sizeof(Context));
	memset(c, 0, sizeof(Context));
	c->mcause = EVENT_NULL;
	c->mepc = (uintptr_t)entry;
	c->mstatus = 0xa0001800; //For DiffTest, though there is not its implement, maybe it will exist in the future.

	c->GPR2 = (uintptr_t)arg;	// Only argument 0 stored, maybe incorrect but sufficient for now.
	return c;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
