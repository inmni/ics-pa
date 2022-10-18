/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <trace.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10
#define MAX_NR_IRB 16
CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
#ifdef CONFIG_ITRACE_RING
IRB iRB;
#endif

void device_update();
#ifdef CONFIG_WATCHPOINT
#include <watchpoint.h>
word_t expr(char *e,bool *success);
#endif
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  log_write("%s\n", _this->logbuf);
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_WATCHPOINT
  WP* wp = get_wp_head();
  int hPrint = 0;
  while(wp){
	bool success = false;
	int newValue = expr(wp->EXPR,&success);
	if(newValue!=wp->oldValue){
		if(nemu_state.state==NEMU_RUNNING){
			nemu_state.state = NEMU_STOP;
		}
		if(!hPrint){
			hPrint = 0;
			printf("The value of expression %s changes from %08x to %08x, stop at pc=0X%08X now\n", wp->EXPR,wp->oldValue,newValue,cpu.pc);
		}
		wp->oldValue = newValue;
	}
	wp = wp->next;
  }
#endif
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
#ifdef CONFIG_ITRACE_RING
	char *head = p;
#endif
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;
	
	void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
	disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
			MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
// adjust order to make it faster.
#ifdef CONFIG_ITRACE_RING
	if(iRB.cur_len<MAX_NR_IRB){
		iRB.buf[iRB.cur_len] = (char *)malloc(128);
		strcpy(iRB.buf[iRB.cur_len], head);
		iRB.cur_len++;
	}
	else{
		strcpy(iRB.buf[iRB.st_index], head);
		iRB.st_index++;
		iRB.st_index%=MAX_NR_IRB;
	}
#endif
#endif
}
#include <time.h>
#include <sys/time.h>
struct timespec time_start = {0,0},time_end={0,0};
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    clock_gettime(CLOCK_REALTIME, &time_start);
		exec_once(&s, cpu.pc);
		clock_gettime(CLOCK_REALTIME, &time_end);
		if(time_end.tv_nsec-time_start.tv_nsec>1500){
			printf("Time spent on pc = 0x%08x is %lu ns\n", s.pc,time_end.tv_nsec-time_start.tv_nsec);
		}
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
#ifdef CONFIG_ITRACE_RING
	int tmp=iRB.st_index;
	do{
		log_write("%s\n", iRB.buf[tmp]);
		tmp++;
		tmp%=MAX_NR_IRB;
	}while(tmp!=iRB.st_index &&tmp<iRB.cur_len);
	printf("Latest instructions have been stored in nemu-log\n");
#endif
	isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
