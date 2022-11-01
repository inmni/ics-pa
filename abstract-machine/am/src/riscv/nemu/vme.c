#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;
  
	return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}
#define PTESIZE 4
#define PTE_PPN_MASK		0xFFFFFC00
#define PTE_POFF_MASK		0x3FF
#define VA_POFF_MASK		0xFFF
#define PA_POFF_MASK		0xFFF
#define PTE uintptr_t
// PTE is 'page-table entry', is a pointer
#define VPN_0(va)				((((uintptr_t)va)>>12)&0x3FF)
#define VPN_1(va)				((((uintptr_t)va)>>22)&0x3FF)
#define PTE_PPN(pte)		(((uintptr_t)pte)>>10)
#define PTE_PPN_0(pte)	(((uintptr_t)pte>>10)&0x3FF)
#define PTE_PPN_1(pte)	(((uintptr_t)pte>>20)&0xFFF)
void map(AddrSpace *as, void *va, void *pa, int prot) {
	printf("map va[%08x]->pa[%08x] with prot[%x]\n", (uintptr_t)va, (uintptr_t)pa, prot);
	// LEVEL 1
	PTE *pte = as->ptr + VPN_1(va)*PTESIZE;
	// if the pte is not valid
	if(!(*pte & PTE_V)){
		// alloc leaf page
		PTE alloced_page = (PTE)pgalloc_usr(PGSIZE);
		// keep permission
		*pte = (*pte & PTE_POFF_MASK)|((alloced_page>>2) & PTE_PPN_MASK)|0x1;
	//	printf("To alloc leaf page in:%p, va:%p\n", pte, va);
	}
	PTE *leaf_pte = (PTE *)(PTE_PPN(*pte)*PGSIZE + VPN_0(va)*PTESIZE);
	//printf("set leaf page va:%p, pa:%p, pte:%p\n", va, pa, leaf_pte);
	// Set permission
	*leaf_pte = (((PTE)pa>>2) & PTE_PPN_MASK) | PTE_V | PTE_W | PTE_R | PTE_X;
	
	assert(PTE_PPN(*leaf_pte) * PGSIZE + ((uintptr_t)va & VA_POFF_MASK) == (uintptr_t)pa);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context *)(kstack.end - sizeof(Context));
	memset(c, 0, sizeof(Context));
	c->mepc = (uintptr_t)entry;
	c->mcause = EVENT_NULL;
	c->pdir = as->ptr;
	return c;
}
