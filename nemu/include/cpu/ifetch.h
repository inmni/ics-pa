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

#ifndef __CPU_IFETCH_H__
#include <memory/vaddr.h>
#define SINGLE_INST_LEN 32
#define MAX_NR_IRB 16
typedef struct inst_ring_buf{
	int cur_len;			 
	int st_index;
	char *insts[MAX_NR_IRB];
	uint64_t codes[MAX_NR_IRB];
	uint64_t pc[MAX_NR_IRB];
}	IRB;
IRB iRingBuffer;
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
static inline uint32_t inst_fetch(vaddr_t *pc, int len) {
  uint32_t inst = vaddr_ifetch(*pc, len);
	char inst_str[SINGLE_INST_LEN]; uint8_t code[4];
	disassemble(inst_str, SINGLE_INST_LEN, *pc, code, len);
  if(iRingBuffer.cur_len<MAX_NR_IRB){
		iRingBuffer.insts[iRingBuffer.cur_len] = (char *)malloc(SINGLE_INST_LEN);
		strcpy(iRingBuffer.insts[iRingBuffer.cur_len],inst_str);
		iRingBuffer.codes[iRingBuffer.cur_len] = (code[0]<<24)+(code[1]<<16)+(code[2]<<8)+code[3];
		iRingBuffer.pc[iRingBuffer.cur_len] = *pc;
		iRingBuffer.cur_len++;
	}
	else{
		strcpy(iRingBuffer.insts[iRingBuffer.st_index],inst_str);
		iRingBuffer.codes[iRingBuffer.st_index] = (code[0]<<24)+(code[1]<<16)+(code[2]<<8)+code[3];
		iRingBuffer.pc[iRingBuffer.st_index] = *pc;
		iRingBuffer.st_index ++;
		iRingBuffer.st_index %= MAX_NR_IRB;
	}
	(*pc) += len;
  return inst;
}

#endif
