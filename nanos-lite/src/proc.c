#include <proc.h>
#include <fs.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

int pg_idx = 1;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void switch_pg(int new_idx){
  if (new_idx == pg_idx)
    return ;
	assert(new_idx>=1 && new_idx<=3);
  switch_boot_pcb();  
  
  pg_idx = new_idx;
  pcb[0].cp->pdir = NULL;

  yield();
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    if (j % 100 == 0)
      Log("'%s' %dth time!", (char *)arg, j);
    j ++;
    yield();
  }
}

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);

#define PROG_PATH1 "/bin/menu"
#define PROG_PATH2 "/bin/pal"
#define PROG_PATH3 "/bin/bird"
void init_proc() {
	context_kload(&pcb[0], hello_fun, "RNMPA");
  char *argv1[] = {PROG_PATH1, NULL};
  char *argv2[] = {PROG_PATH2, NULL};
  char *argv3[] = {PROG_PATH3, NULL};
  context_uload(&pcb[1], PROG_PATH1, argv1, NULL);
  context_uload(&pcb[2], PROG_PATH2, argv2, NULL);
  context_uload(&pcb[3], PROG_PATH3, argv3, NULL);

  switch_boot_pcb();
}

Context* schedule(Context *prev) {
  current->cp = prev;

  current = (current == &pcb[0] ? &pcb[pg_idx] : &pcb[0]);
  return current->cp;
}


int execve(const char *filename, char *const argv[], char *const envp[]){
  if (fs_open(filename, 0, 0) == -1){
    return -1;
  }
  context_uload(&pcb[pg_idx], filename, argv, envp);
  switch_boot_pcb();  
  
  pcb[0].cp->pdir = NULL;
  yield();
  return 0;
}
