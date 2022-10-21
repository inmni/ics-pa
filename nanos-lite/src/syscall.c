#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <sys/time.h>
//#define CONFIG_STRACE
#ifdef CONFIG_STRACE
static const char *syscall_table[] = {
"SYS_exit", "SYS_yield", "SYS_open", "SYS_read", 
"SYS_write", "SYS_kill", "SYS_getpid", "SYS_close", 
"SYS_lseek", "SYS_brk", "SYS_fstat", "SYS_time",
"SYS_signal", "SYS_execve", "SYS_fork", "SYS_link",
"SYS_unlink", "SYS_wait", "SYS_times", "SYS_gettimeofday",
};
#endif
uint32_t temp;
extern char end;

static inline int syscall_gettimeofday(struct timeval *tv, struct timezone *tz);

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
		case SYS_write: c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
#ifdef CONFIG_STRACE
										printf("Write %s to file: %s\n", (char *)a[2], fs_name(a[1]));
#endif
																										break;
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
		case SYS_gettimeofday:	c->GPRx = syscall_gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]);
																										break;
		default: panic("Unhandled syscall ID = %d", a[0]);
  }
#ifdef CONFIG_STRACE
	printf("================syscall trace end================\n");
#endif
}

static inline int syscall_gettimeofday(struct timeval *tv, struct timezone *tz){
		assert(tv);
		uint64_t uptime = io_read(AM_TIMER_UPTIME).us;
		tv->tv_sec = uptime/1000000;
		tv->tv_usec = uptime%1000000;
		if(tz){
				// TODO: use correct timezone
				tz->tz_minuteswest = 0;
				tz->tz_dsttime = 0;
		}
		return 0;
}
