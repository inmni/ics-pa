#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
	printf("CONTEXT, mepc:%d, mcause: %d, mstatus: %d\nREGS:\n",c->mepc,c->mcause,c->mstatus);
	printf("-1 test:%d\n",-1);
	int i;
	int line_count = 1;
	for(i=0;i<32;i++,line_count++){
		printf("%d	", c->gpr[i]);
		if(line_count==4){
			line_count = 0;
			printf("\n");
		}
	}
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
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
  return NULL;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
