#include <stdio.h>
#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#if 1
unsigned long NDL_GetTicks();
#define getTime() NDL_GetTicks()
#else
static timeval tv;
#define getTime() {gettimeofday(&tv,NULL); tv;}
#endif
int main() {
	unsigned long lastTime = getTime();
	unsigned long currTime = getTime();
	int count = 20;
	while(count){
			currTime = getTime();
			if( currTime - lastTime >= 500){
					printf("Timer-test!\n");
					count--;
					lastTime = getTime();
			}
	}
  return 0;
}
