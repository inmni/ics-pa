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
#include <memory/paddr.h>
#include <memory/vaddr.h>
#define SATP_T uint32_t
#define PTE_T uint32_t
#define LEVELS								2
#define PTESIZE								4
#define PAGESIZE							4096
#define ADDR_PGOFF_MASK				0xFFF
#define VA_VPN_1(va)					(((uint32_t)va)>>22)
#define VA_VPN_0(va)					((((uint32_t)va)>>12)&0x3FF)
#define VA_PGOFF(va)					(((uint32_t)va)&ADDR_PGOFF_MASK)
#define PTE_V(pte)						(pte&0x1)
#define PTE_R(pte)						(pte&0x2)
#define PTE_W(pte)						(pte&0x4)
#define PTE_X(pte)						(pte&0x8)
#define PTE_U(pte)						(pte&0x10)
#define PTE_G(pte)						(pte&0x20)
#define PTE_A(pte)						(pte&0x40)
#define PTE_D(pte)						(pte&0x80)
#define PTE_PPN(pte)					(pte>>10)
#define SATP_PPN_MASK					0x3FFFFF
#define SATP_PPN(satp)				(satp&SATP_PPN_MASK)
#define SATP_MODE(satp)				(satp>>31)
inline int isa_mmu_check(vaddr_t vaddr, int len, int type) {
	SATP_T satp = cpu.sr[SATP];
	switch(SATP_MODE(satp)) {
		case 0:		return MMU_DIRECT;
		case 1:		return MMU_TRANSLATE;
		default:	return MMU_FAIL;
	}
}
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  SATP_T satp = cpu.sr[SATP];
	// Step 1
	uint32_t i = LEVELS - 1;//1
	// Step 2
	uintptr_t pte1_addr = SATP_PPN(satp)*PAGESIZE + VA_VPN_1(vaddr)*PTESIZE;
	PTE_T pte1_val = paddr_read(pte1_addr, sizeof(PTE_T));
	// printf("PTE: %08x\n", pte1_val);
	// Step 3
	if(((!PTE_V(pte1_val)) || ((!PTE_R(pte1_val))&&PTE_W(pte1_val)))){
		printf("Error in translate %08x, type %d\n", vaddr, type);
		printf("PTE on %08x: %08x, %x, %x, %x\n", (uint32_t)pte1_addr, pte1_val, PTE_V(pte1_val), PTE_R(pte1_val), PTE_W(pte1_val));
		nemu_state.state = NEMU_ABORT;
		return 0;
		assert(0);
	}
	// Step 4
	i--;
	// Perform Step 2 again
	uintptr_t leaf_pte_addr = PTE_PPN(pte1_val)*PAGESIZE + VA_VPN_0(vaddr)*PTESIZE;
	PTE_T leaf_pte_val = paddr_read(leaf_pte_addr, sizeof(PTE_T));
	// Perform Step 3 again
		if(((!PTE_V(leaf_pte_val)) || ((!PTE_R(leaf_pte_val))&&PTE_W(leaf_pte_val)))){
				printf("Error in translate %08x, type %d\n", vaddr, type);
				printf("PTE on %08x: %08x, %x, %x, %x\n", (uint32_t)pte1_addr, pte1_val, PTE_V(pte1_val), PTE_R(pte1_val), PTE_W(pte1_val));
				printf("Leaf PTE on %08x: %08x, %x, %x, %x\n", (uint32_t)leaf_pte_addr, leaf_pte_val, PTE_V(leaf_pte_val), PTE_R(leaf_pte_val), PTE_W(leaf_pte_val));
				nemu_state.state = NEMU_ABORT;
				return 0;
				assert(0);
		}
	printf("\n%08x\n", leaf_pte_val);
	// Assert Step 4 for this is Sv32
	assert(PTE_R(leaf_pte_val) || PTE_X(leaf_pte_val));
	// Step 5
	// Skip for PA request
	
	// Step 6
	// Skip for Sv32, i=0 here
	assert(i==0);
	// Step 7
	// Skip for PA request

	// Step 8
	return  PTE_PPN(leaf_pte_val)*PAGESIZE | VA_PGOFF(vaddr);
}
