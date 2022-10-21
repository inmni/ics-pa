#include <common.h>
#include "syscall.h"
#include <fs.h>
//#define CONFIG_STRACE
#ifdef CONFIG_STRACE
static const char *syscall_table[] = {
"SYS_exit", "SYS_yield", "SYS_open", "SYS_read", "SYS_write", "SYS_kill", "SYS_getpid", "SYS_close", "SYS_lseek", "SYS_brk",/*Others need implement*/
};
#endif
uint32_t temp;
extern char end;
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
#ifdef CONFIG_STRACE
	printf("===============syscall trace start================\n");
	printf("syscall trace: %s(%d %d %d)\n",syscall_table[a[0]],a[1],a[2],a[3]);
#endif
  switch (a[0]) {
		case SYS_exit:	halt(a[1]);											break;
		case SYS_yield:	yield();		c->GPRx=0;					break;
		case SYS_open:	c->GPRx = fs_open((char *)a[1], a[2], a[3]);
#ifdef CONFIG_STRACE
										printf("Open file: %s\n", fs_name(c->GPRx));
#endif
																										break;
		case SYS_read:	c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
#ifdef CONFIG_STRACE
										printf("Read %s from file: %s\n", (char *)a[2], fs_name(a[1]));
#endif
																										break;
		case SYS_write: {
				if(a[1]==1||a[1]==2){
						for(temp = 0;temp<a[3];temp++)putch(*((char *)a[2]+temp));								
						c->GPRx = a[3];
				}
				else{
						c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
#ifdef CONFIG_STRACE
										printf("Write %s to file: %s\n", (char *)a[2], fs_name(a[1]));
#endif
				}
																										break;
		}
		case SYS_close: c->GPRx = fs_close(a[1]);				
#ifdef CONFIG_STRACE
										printf("Close file: %s\n", fs_name(a[1]));
#endif
																										break;
		case SYS_lseek:	c->GPRx = fs_lseek(a[1], a[2], a[3]);
#ifdef CONFIG_STRACE
										printf("Set offset %d in file: %s\n", a[2], fs_name(a[1]));
#endif
																										break;
		case SYS_brk:								c->GPRx=0;					break;
		default: panic("Unhandled syscall ID = %d", a[0]);
  }
#ifdef CONFIG_STRACE
	printf("================syscall trace end================\n");
#endif
}
