#include <common.h>
#include "syscall.h"
#ifdef CONFIG_STRACE
static const char *syscall_table[] = {
"SYS_exit", "SYS_yield",/*Others need implement*/
};
#endif
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
#ifdef CONFIG_STRACE
	Log("SYScall trace: %s(%d %d %d)",syscall_table[a[0]],a[1],a[2],a[3]);
#endif
  switch (a[0]) {
		case SYS_exit:	halt(a[1]);						break;
		case SYS_yield:	yield();		c->GPRx=0;break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
