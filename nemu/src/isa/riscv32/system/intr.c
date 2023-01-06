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
#include <isa.h>
#include "../local-include/reg.h"
void isa_reg_display();
#define IRQ_TIMER 0x80000007
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
	sr(MEPC) = epc;
	sr(MCAUSE) = NO;
	if(NO != IRQ_TIMER)
		sr(MSTATUS) = ((BITS(sr(MSTATUS), 31, 8)<<8) | (BITS(sr(MSTATUS), 3, 3)<<7) | (BITS(sr(MSTATUS), 6, 0))) & 0xFFFFFFF7;

#ifdef CONFIG_ETRACE
	Log("Exception Trace: PC=0x%08x status: %08x cause: %d\n", epc, sr(MSTATUS),sr(MCAUSE));
#endif
	//printf("Start raise. mepc:0x%08x, mcause: %d, mtvec:0x%08x\n",sr(MEPC),sr(MCAUSE),sr(MTVEC));
	//isa_reg_display();
	return sr(MTVEC);
}
#define MSTATUS_MIE_MASK 0x8
word_t isa_query_intr() {
	if(cpu.INTR && (sr(MSTATUS)&MSTATUS_MIE_MASK)){
		cpu.INTR = false;
		//Log("SWITCH");
//		return INTR_EMPTY;
		return IRQ_TIMER;
	}
  return INTR_EMPTY;
}
