#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  char *empty[] =  {NULL };
  environ = empty;
	printf("The address of args: %p\n", args);
	int argc = *(int *)args;
	char **argv = (char **)(args + 1);
	char **envp = (char **)(args + 2);
  environ = envp;
	printf("The args are %d, %p, %p\n", argc, argv, envp);
	exit(main(argc, argv, envp));
  assert(0);
}
