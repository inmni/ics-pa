#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
	char *strToPrint = NULL;
	if(argc>0 && argv){
			printf("The input is following:\n");
			for( int idx = 0; idx<argc; idx++){
				printf("	[%d]: %s\n", idx, argv[idx]);
			}
	}
	else{
		while(1){
			printf("No argument\n");
		}
	}
  while (argc>0 && argv) {
    j ++;
    if (j == 1000000) {
			//write(1, "Hello World from Navy-apps for the ",36);
			//printf("%d", i ++);
			//write(1," th times!\n",11);
      printf("%s\n", argv[0]);
      j = 0;
    }
  }
  return 0;
}
