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

#ifndef __ISA_RISCV32_H__
#define __ISA_RISCV32_H__

#include <common.h>

typedef struct {
  word_t gpr[32];
  vaddr_t pc;
	word_t sr[4096];
	bool INTR;
} riscv32_CPU_state;
#define MCAUSE 		0x342
#define MEPC			0x341
#define MSCRATCH	0x340
#define MTVEC 		0x305
#define MSTATUS 	0x300
#define SATP			0x180
// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} riscv32_ISADecodeInfo;


#endif
