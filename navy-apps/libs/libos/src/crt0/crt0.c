#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  printf("call_main\n");
	char *empty[] =  {NULL };
  environ = empty;
	int argc = *(int *)args;
	char **argv = (char **)(args + 1);
	char **envp = (char **)(args + 2 + argc);
  environ = envp;
	printf("To exec %p with arguments:\n", (int *)main);
	for(int i=0;i<argc;i++){
			printf("	[%d]: %s\n", i, argv[i]);
	}
	exit(main(argc, argv, envp));
  assert(0);
}
