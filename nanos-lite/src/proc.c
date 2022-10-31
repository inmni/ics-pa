#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
uintptr_t outside_loader(PCB* p, char *filename);
void context_uload(PCB* p, char *filename, char *const argv[], char *const envp[]);
void context_kload(PCB* p, void (*entry)(void *), void* arg);
void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
    j ++;
    yield();
  }
}

void naive_uload(PCB *pcb, const char *filename);
void init_proc() {
	//context_kload(&pcb[0], hello_fun, "AAAAAA");
	//context_kload(&pcb[1], hello_fun, "ZZZZZZ");
//	char *arg1[] = {"FIRST", NULL};
//	char *arg2[] = {"SECOND", NULL};
	char *empty[] = {NULL};
//  context_uload(&pcb[0], "/bin/hello", arg1, empty);
//	context_uload(&pcb[1], "/bin/hello", arg2, empty);
//	printf("arg1: %s, arg2: %s\n", arg1[0], arg2[0]);
	context_uload(&pcb[0], "/bin/exec-test", empty, empty);
	switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
	// naive_uload(NULL, "/bin/menu");
}

void context_kload(PCB* p, void (*entry)(void *), void* arg) {
	Area kstack;
	kstack.start = &(p->cp);
	kstack.end = p->stack + sizeof(p->stack);

	p->cp = kcontext(kstack, entry, arg);

	p->cp->mstatus = 0xa0001800;// For DiffTest, though there is not its implement;
}

void context_uload(PCB* p, char *filename, char *const argv[], char *const envp[]) {
	void* ustack = new_page(8);
	uint32_t* ustack_start = ustack + 4;
	uint32_t* ustack_end = ustack + STACK_SIZE;
	printf("MALLOC [%p, %p)\n", ustack, ustack_end);
	// copy arguments
	int argv_c = 0; int envp_c = 0;
	while(argv && argv[argv_c]){
		ustack_end -= strlen(argv[argv_c]) + 1; // keep zero ternimating
		strcpy((char *)ustack_end, argv[argv_c]);
		*ustack_start++ = (uint32_t)ustack_end;
		argv_c++;
	}
	while(envp && envp[envp_c]){
		ustack_end -= strlen(envp[envp_c]) + 1;
		strcpy((char *)ustack_end, envp[envp_c]);
		*ustack_start++ = (uint32_t)ustack_end;
		envp_c++;
	}
	*(uint32_t *)ustack = argv_c + envp_c;
	
	Area kstack;
	kstack.start = p->stack;
	kstack.end = p->stack + STACK_SIZE;
	printf("KERNEL stack [%p, %p)\n", kstack.start, kstack.end);
	uintptr_t entry = outside_loader(p, filename);
	p->cp = ucontext(&(p->as), kstack, (void *)entry);
	p->cp->GPRx = (uintptr_t)ustack;
	printf("args begin: %p, argc: %d, argv begin: %p, argv[0] value: %s\n", ustack, *(uint32_t *)ustack, ustack + 4, *(char **)(ustack + 4));
}
Context* schedule(Context *prev) {
	current->cp = prev;

	current = (current==&pcb[0] ? &pcb[0] : &pcb[0]); // Need to change

  return current->cp;
}
