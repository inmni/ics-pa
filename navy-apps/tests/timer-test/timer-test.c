#include <sys/time.h>
#include <stdio.h>
#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif
int main() {
	struct timeval tv;
	unsigned long long lastTime = 0;
	int count = 20;
	while(count){
			gettimeofday(&tv, NULL);
			if( tv.tv_usec+tv.tv_sec*1000000 - lastTime >= 500000){
					printf("Timer-test!\n");
					count--;
					lastTime = tv.tv_usec + tv.tv_sec*1000000;
			}
	}
  return 0;
}
