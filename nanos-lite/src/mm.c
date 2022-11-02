#include <memory.h>
#include <proc.h>
static void *pf = NULL;
void* new_page(size_t nr_page) {
	assert(pf + nr_page*PGSIZE <= heap.end);
	void *pf_start = pf;
	printf("new page [%08x, %08x)\n", (uintptr_t)pf_start, (uintptr_t)pf_start + nr_page*PGSIZE);
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
  current->max_brk = ROUNDUP(current->max_brk, PGSIZE);
	if(brk <= current->max_brk)	return 0;
	// alloc
	uint32_t pg_nr = ROUNDUP(brk - current->max_brk, PGSIZE)/PGSIZE;
	void* pg_start = new_page(pg_nr);	int i;
	for(i = 0; i<pg_nr; i++){
		map(&current->as,
			(void *)current->max_brk + i*PGSIZE,
			pg_start + i*PGSIZE,
			MMAP_READ | MMAP_WRITE
			);
	}
	current->max_brk += pg_nr * PGSIZE;
	return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
