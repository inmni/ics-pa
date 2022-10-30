#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void context_kload(PCB* p, void (*entry)(void *), void* arg);
void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void naive_uload(PCB *pcb, const char *filename);
void init_proc() {
	context_kload(&pcb[0], hello_fun, NULL);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
//	naive_uload(NULL, "/bin/hello");
}

void context_kload(PCB* p, void (*entry)(void *), void* arg) {
	Area kstack;
	kstack.start = &(p->cp);
	kstack.end = p->stack + sizeof(p->stack);

	p->cp = kcontext(kstack, entry, arg);
}

Context* schedule(Context *prev) {
	current->cp = prev;

	current = &pcb[0]; // Need to change

  return current->cp;
}
