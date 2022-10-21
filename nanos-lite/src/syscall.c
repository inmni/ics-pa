#include <common.h>
#include "syscall.h"

#ifdef CONFIG_STRACE
static const char *syscall_table[] = {
"SYS_exit", "SYS_yield", "SYS_open", "SYS_read", "SYS_write",/*Others need implement*/
};
#endif
uint32_t temp;
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
#ifdef CONFIG_STRACE
	printf("SYScall trace: %s(%d %d %d)\n",syscall_table[a[0]],a[1],a[2],a[3]);
#endif
  switch (a[0]) {
		case SYS_exit:	halt(a[1]);						break;
		case SYS_yield:	yield();		c->GPRx=0;break;
		case SYS_write: {
				if(a[1]==1||a[1]==2){
						for(temp = 0;temp<a[3];temp++)putch(*((char *)a[1]+temp));
						c->GPRx = 0;									break;
				}
				printf("%d\n",a[1]);
		}
		default: panic("Unhandled syscall ID = %d", a[0]);
  }
#ifdef CONFIG_STRACE
	printf("%s -> %d\n", syscall_table[a[0]],c->GPRx);
#endif
}
