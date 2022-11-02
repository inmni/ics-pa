#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;
void __am_get_cur_as(Context* c);
void __am_switch(Context* c);

#define IRQ_TIMER 0x80000007
Context* __am_irq_handle(Context *c) {
	
//	printf("CONTEXT, mepc:%08x, mcause: %d, mstatus: %08x\nREGS:\n",c->mepc,c->mcause,c->mstatus);
  
	__am_get_cur_as(c);
	if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
				case EVENT_YIELD:	ev.event = c->GPR1==-1 ? EVENT_YIELD:EVENT_SYSCALL; break;
				case IRQ_TIMER:	ev.event = EVENT_IRQ_TIMER;	break;
				default: ev.event = EVENT_ERROR; printf("EVENT_ERROR\n");break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
	__am_switch(c);
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
	c->mstatus = 0xa0001800 | 0x80; //For DiffTest, though there is not its implement, maybe it will exist in the future.
	c->np = 0;	// 0 means kernel
	c->GPRx = (uintptr_t)arg;	// Only argument 0 stored, maybe incorrect but sufficient for now.
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
