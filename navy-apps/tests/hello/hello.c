#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
			write(1, "Hello World from Navy-apps for the ",36);
			printf("%d", i ++);
			write(1," th times!\n",11);
      //printf("tHello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }
  return 0;
}
