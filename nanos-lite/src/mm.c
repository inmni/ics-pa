#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
	assert(pf + nr_page*PGSIZE <= heap.end);
	void *pf_start = pf;
//	memset(pf_start, 0, nr_page * PGSIZE);
	pf += nr_page * PGSIZE;
	return pf_start;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  void *pg =  new_page(n/PGSIZE);
	memset(pg, 0, n);
	return pg;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
