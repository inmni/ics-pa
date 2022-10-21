#include <stdio.h>
#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#if 0
unsigned long NDL_GetTicks();
#define getTime() NDL_GetTicks()
#else
#include <sys/time.h>
static struct timeval tv;
#define getTime() gettimeofday(&tv,NULL),(tv.tv_usec/1000+tv.tv_sec*1000)
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
