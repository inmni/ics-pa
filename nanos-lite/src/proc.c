#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB* p, const char *filename, char *const argv[], char *const envp[]);
void context_kload(PCB* p, void (*entry)(void *), void* arg);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    if(j%10000==0){
			Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
		}
		j ++;
  }
}

void init_proc() {
//	context_kload(&pcb[0], hello_fun, "AAAAAA");
	//context_kload(&pcb[1], hello_fun, "ZZZZZZ");
	char *arg1[] = {"ZZZZZZ", NULL};
	char *arg2[] = {"/bin/cat", "/share/games/bird/atlas.txt", NULL};
	char *empty[] = {NULL};
  context_uload(&pcb[0], "/bin/hello", arg2, empty);
//	context_uload(&pcb[1], "/bin/hello", arg2, empty);
//	printf("arg1: %s, arg2: %s\n", arg1[0], arg2[0]);
	context_uload(&pcb[1], "/bin/hello", arg1, empty);
	pcb[0].prio = 100;
	switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
	// naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
	static int curr_pcb_id = 0;
	static int count = 1;
	current->cp = prev;
	
	if(count < current->prio){
		count++;
		return current->cp;
	}
	count = 1;
	do{
		curr_pcb_id++;
		curr_pcb_id %= MAX_NR_PROC;
	} while(pcb[curr_pcb_id].prio==0);

	current = &pcb[curr_pcb_id]; // Need to change

  return current->cp;
}
