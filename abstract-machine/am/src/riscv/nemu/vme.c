#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <riscv/riscv.h>

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
  printf("kas.ptr addr: %p\n", kas.ptr);

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
  if (c->pdir != NULL){ //自行添加
    //printf("在__am_get_cur_as中设置为由%p，地址为%p，更改为，", c->pdir, &c->pdir);
    c->pdir = (vme_enable ? (void *)get_satp() : NULL);
    //printf("%p\n", c->pdir);
  }
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    //printf("在__am_switch中设置satp为%p\n", c->pdir);
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
	PTE *pte = as->ptr + VPN_1(va)*PTESIZE;
	if(!(*pte & PTE_V)){
		PTE alloced_page = (PTE)pgalloc_usr(PGSIZE);
		*pte = (alloced_page>>2) |0x1;
	}
	PTE *leaf_pte = (PTE *)(PTE_PPN(*pte)*PGSIZE + VPN_0(va)*PTESIZE);
	*leaf_pte = ((PTE)pa>>2) | 0xf;
}
Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *context = kstack.end - sizeof(Context);
  context->mstatus = 0xC0000 | 0x80;//MPP设置为U模式，MXR=1，SUM=1
  context->mepc    = (uintptr_t)entry;
  context->pdir    = as->ptr;
  context->np      = 0;//USER

  return context;
}
