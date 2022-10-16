/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN AS IS BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_IS, TYPE_U, TYPE_S,	TYPE_R,	TYPE_B,	TYPE_J,
  TYPE_N, // none
};
#define Rsrc1 R(src1)
#define Rsrc2 R(src2)
#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immIS() do { *imm = SEXT(BITS(i, 24, 20), 5); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immB() do { *imm = SEXT((BITS(i, 31, 31)<<11)|(BITS(i, 7, 7)<<10)|(BITS(i, 30, 25)<<4)|BITS(i, 11, 8), 12)<<1; } while(0)
#define immJ() do { *imm = SEXT((BITS(i, 31, 31)<<19)|(BITS(i, 19, 12)<<11)|(BITS(i, 20, 20)<<10)|BITS(i, 30, 21),20)<<1; } while(0)
static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
  /*
  	case TYPE_I:           immI(); break;
    	case TYPE_IS:         immIS(); break;
    	case TYPE_U:                   immU(); break;
    	case TYPE_S:  immS(); break;
	case TYPE_R: 	      break;
	case TYPE_B:  immB(); break;
	case TYPE_J: 		       immJ(); break;*/
  
    	case TYPE_I: src1R();          immI(); break;
    	case TYPE_IS:src1R();         immIS(); break;
    	case TYPE_U:                   immU(); break;
    	case TYPE_S: src1R(); src2R(); immS(); break;
	case TYPE_R: src1R(); src2R(); 	     ; break;
	case TYPE_B: src1R(); src2R(); immB(); break;
	case TYPE_J: 		       immJ(); break;
  }
	//printf(dest:%d,src1:%08x,src2;%08x,imm:%d,type:%d\n,*dest,*src1,*src2,*imm,type);
}

static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;
#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
	/*printf(pc:0x%08x	Execute inst: %s	\n,s->pc,str(name));*/\
	decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
	/*isa_reg_display();*/\
}

  INSTPAT_START();
  	INSTPAT(00000000000000000000000000110111, lui    , U, R(dest) = imm);//y
	INSTPAT(00000000000000000000000000010111, auipc  , U, R(dest) = imm + s->pc);//y
	INSTPAT(00000000000000000000000000110011, add	 , R, R(dest) = src1 + src2);//y
	INSTPAT(01000000000000000000000000110011, sub	 , R, R(dest) = src1 - src2);//y
	INSTPAT(00000000000000000001000000110011, sll	 , R, R(dest) = src1 << (src2&31));//y
	INSTPAT(00000000000000000010000000110011, slt	 , R, R(dest) = ((int)src1 < (int)src2));
	INSTPAT(00000000000000000011000000110011, sltu	 , R, R(dest) = (src1 < src2));//y
	INSTPAT(00000000000000000100000000110011, xor	 , R, R(dest) = src1 ^ src2);//y
	INSTPAT(00000000000000000101000000110011, srl	 , R, R(dest) = src1 >> (src2&31));//y
	INSTPAT(01000000000000000101000000110011, sra	 , R, R(dest) = ((int)src1 >> (src2&31)));//y
	INSTPAT(00000000000000000110000000110011, or 	 , R, R(dest) = src1 | src2);//y
	INSTPAT(00000000000000000111000000110011, and	 , R, R(dest) = src1 & src2);
	INSTPAT(00000010000000000000000000110011, mul	 , R, R(dest) = src1 * src2);
	INSTPAT(00000010000000000001000000110011, mulh	 , R, R(dest) = (uint32_t)(((int64_t)(int)src1*(int64_t)(int)src2)>>32));//y
	INSTPAT(00000010000000000011000000110011, mulhu	 , R, R(dest) = (uint32_t)(((uint64_t)src1*(uint64_t)src2)>>32));
	INSTPAT(00000010000000000100000000110011, div	 , R, R(dest) = ((int)src1 / (int)src2));//y
	INSTPAT(00000010000000000101000000110011, divu   , R, R(dest) = src1 / src2);
	INSTPAT(00000010000000000110000000110011, rem	 , R, R(dest) = ((int)src1) % ((int)src2));//y
	INSTPAT(00000010000000000111000000110011, remu	 , R, R(dest) = src1 % src2);
	INSTPAT(00000000000000000000000000010011, addi   , I, R(dest) = src1 + imm);//y
	INSTPAT(00000000000000000011000000010011, sltiu	 , I, R(dest) = src1 < imm);//y
	INSTPAT(00000000000000000100000000010011, xori	 , I, R(dest) = src1 ^ imm);//y
	INSTPAT(00000000000000000110000000010011, ori	 , I, R(dest) = src1 | imm);
	INSTPAT(00000000000000000111000000010011, andi   , I, R(dest) = src1 & imm);
	INSTPAT(00000000000000000000000001110011, ebreak , I, NEMUTRAP(s->pc, R(10))); //ok R(10) is $a0
	INSTPAT(00000000000000000000000001100111, jalr   , I, s->dnpc = (src1 + imm)&~1, R(dest) = s->pc + 4
#ifdef CONFIG_FTRACE
			,ret_to_ftrace(R(dest))						
#endif
			);//y
	//I type Special 
	INSTPAT(00000000000000000001000000010011, slli	 , IS, R(dest) = src1<<imm );//y
	INSTPAT(00000000000000000101000000010011, srli	 , IS, R(dest) = src1>>imm );//y
	INSTPAT(01000000000000000101000000010011, srai   , IS, R(dest) = ((int)src1)>>imm);//y
	INSTPAT(00000000000000000000000000000011, lb	 , I, R(dest) = Mr(src1 + imm, 1); if(R(dest)&0x80)R(dest)|=0xFFFFFFF0);
	INSTPAT(00000000000000000001000000000011, lh	 , I, R(dest) = Mr(src1 + imm, 2); if(R(dest)&0x8000)R(dest)|=0xFFFF0000);//y
  	INSTPAT(00000000000000000010000000000011, lw     , I, R(dest) = Mr(src1 + imm, 4));//y
	INSTPAT(00000000000000000100000000000011, lbu	 , I, R(dest) = Mr(src1 + imm, 1));//y
	INSTPAT(00000000000000000101000000000011, lhu	 , I, R(dest) = Mr(src1 + imm, 2));//y
	INSTPAT(00000000000000000010000000010011, slti	 , I, R(dest) = (int)src1<(int)imm);
	INSTPAT(00000000000000000000000000100011, sb	 , S, Mw(src1 + imm, 1, src2&0xFF));//y
  	INSTPAT(00000000000000000001000000100011, sh	 , S, Mw(src1 + imm, 2, src2&0xFFFF));//y
	INSTPAT(00000000000000000010000000100011, sw     , S, Mw(src1 + imm, 4, src2));//y
	INSTPAT(00000000000000000000000001100011, beq	 , B, if(src1==src2)s->dnpc = s->pc+imm);//y
	INSTPAT(00000000000000000001000001100011, bne	 , B, if(src1!=src2)s->dnpc = s->pc+imm);//y
	INSTPAT(00000000000000000100000001100011, blt	 , B, if((int)src1<(int)src2)s->dnpc =s->pc+imm);//y
  	INSTPAT(00000000000000000101000001100011, bge	 , B, if((int)src1>=(int)src2)s->dnpc = s->pc+imm);//y
	INSTPAT(00000000000000000110000001100011, bltu	 , B, if(src1<src2)s->dnpc = s->pc+imm);//y
	INSTPAT(00000000000000000111000001100011, bgeu	 , B, if(src1>=src2)s->dnpc = s->pc+imm);
	INSTPAT(00000000000000000000000001101111, jal    , J, R(dest) = s->pc + 4, s->dnpc = s->pc + imm
#ifdef CONFIG_FTRACE
			,call_to_ftrace(s->dnpc)							
#endif
			);//y
  	INSTPAT(00000000000000000000000000000000, inv    , N, INV(s->pc));//y
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0
  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
