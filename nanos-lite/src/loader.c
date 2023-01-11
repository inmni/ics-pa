#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define uintN		uint64_t
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define uintN		uint32_t
#endif

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fr_r, i;
	uintN offset;
	Elf_Ehdr ehdr; Elf_Phdr phdr;
	
	int fd = fs_open(filename, 0, 0);
	assert(fd!=-1);
	fs_lseek(fd, 0, SEEK_SET);
	
	fr_r = fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
	printf("filename:%s, e_ident:%d\n", filename, *(uint32_t *)ehdr.e_ident);	
	assert(fr_r==sizeof(Elf_Ehdr));
	assert(*(uint32_t *)ehdr.e_ident == 0x464C457F/*To complete*/);
	assert(ehdr.e_phoff!=0);
	
	offset = ehdr.e_phoff - ehdr.e_phentsize;
	for(i = 0; i < ehdr.e_phnum; i++){
			offset += ehdr.e_phentsize;
			fs_lseek(fd, offset, SEEK_SET);
			//printf("iteration %dth\n",i);
			fs_read(fd, &phdr, sizeof(Elf_Phdr));

			if(phdr.p_type != PT_LOAD)continue;
			//printf("after continue at %dth iteration\n", i);
			fs_lseek(fd, phdr.p_offset, SEEK_SET);
			uint32_t pg_start = phdr.p_vaddr & 0xFFFFF000;
			uint32_t pg_end = (phdr.p_vaddr + phdr.p_memsz - 1) & 0xFFFFF000;
			uint32_t pg_off = phdr.p_vaddr & 0xFFF;
			uint32_t pg_nr = (pg_end - pg_start)/PGSIZE + 1;
			//printf("to alloc %d pages [%08x, %08x) based on %08x\n",pg_nr, pg_start, pg_end + PGSIZE, (uintptr_t)(pcb->as.ptr));
			void *pg_ptr = new_page(pg_nr);
			//printf("pa start: %08x\n", (uintptr_t)pg_ptr);
			//printf("alloc %d pages\n", pg_nr);
			for(int j=0; j < pg_nr; j++){
				map(&pcb->as,
						(void *)(pg_start + j*PGSIZE),
						pg_ptr + j*PGSIZE,
						MMAP_READ | MMAP_WRITE
				);
			}
			fs_read(fd, pg_ptr + pg_off, phdr.p_filesz);
			//printf("after fs_read\n");
			pcb->max_brk = pg_end + PGSIZE;
			//Log("max_brk: %08x, addr: %p", pcb->max_brk, &pcb->max_brk);
			//memset((void:w
			//*)(pg_ptr + pg_off + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
			//printf("after memset\n");
	}

	fs_close(fd);
	return ehdr.e_entry;
}
void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
  assert(0);
}
#define ceil_up_to_4(x) ((x+3)&(~0x11))
void context_kload(PCB *p, void (*entry)(void *), void *arg){
  Area kstack;
  kstack.start = &p->cp;
  kstack.end = &p->cp + STACK_SIZE;

  p->cp = kcontext(kstack, entry, arg);
}
void context_uload(PCB* p, const char *filename, char *const argv[], char *const envp[]) {
	int i; protect(&p->as);

	void *ustack = new_page(8);
	// For the definition of protect, it is neccessary to begin at the end of the area.	
	for(i=8; i>=1; i--){
		map(&p->as,	p->as.area.end - i*PGSIZE, 
		ustack + STACK_SIZE - i*PGSIZE, MMAP_READ | MMAP_WRITE);
	}
	// Record the bottom of user stack
	uint32_t* ustack_end = ustack + STACK_SIZE;
	// The Top of current user stack
	char* ustack_cur = (char *)(ustack_end - 1);
	// printf("MALLOC [%p, %p)\n", ustack, ustack_end);
	// copy arguments
	int argv_c = 0; int envp_c = 0;
	if(argv) for(; argv[argv_c]; argv_c++){} 
	if(envp) for(; envp[envp_c]; envp_c++){}
	char **argv_ptr = (char **)malloc(argv_c*sizeof(char **));
	char **envp_ptr = (char **)malloc(envp_c*sizeof(char **));
	for(i=0; i<argv_c; i++){
		ustack_cur -= ceil_up_to_4(strlen(argv[i]) + 1); // keep zero ternimating
		strcpy(ustack_cur, argv[i]);
		argv_ptr[i] = ustack_cur;
	}
	for(i=0; i<envp_c; i++){
		ustack_end -= ceil_up_to_4(strlen(envp[i]) + 1);
		strcpy(ustack_cur, envp[envp_c]);
		envp_ptr[i] = ustack_cur;
	}
	uintptr_t *ustack_cur_ptr = (uintptr_t *)ustack_cur;
	ustack_cur_ptr--; *ustack_cur_ptr = 0;
	for(i=envp_c-1; i>=0; i--){
		ustack_cur_ptr--;
		*ustack_cur_ptr = (uintptr_t)envp_ptr[i];
	}
	ustack_cur_ptr--; *ustack_cur_ptr = 0;
	for(i=argv_c-1; i>=0; i--){
		ustack_cur_ptr--;
		*ustack_cur_ptr = (uintptr_t)argv_ptr[i];
	}
	ustack_cur_ptr--; *ustack_cur_ptr = argv_c;
	ustack_cur_ptr--; *ustack_cur_ptr = 0; // Leave space for t0_buffer which is used in trap
																				 
	// printf("KERNEL stack [%p, %p)\n", kstack.start, kstack.end);
//	printf("try to load %s\n",filename);
	uintptr_t entry = loader(p, filename);
	// printf("%s's entry: %08x\n",filename, entry);
	Area kstack;
	kstack.start = &p->cp;
	kstack.end = &p->cp + STACK_SIZE;

	p->cp = ucontext(&(p->as), kstack, (void *)entry);

	// Set sp
	p->cp->gpr[2] = (uintptr_t)ustack_cur_ptr - (uintptr_t)ustack_end + (uintptr_t)(p->as.area.end);
	
	// p->cp->GPRx = p->cp->gpr[2] + 4;
	// printf("prio set:%d, addr: %p\n", p->prio, &p->prio);
	// printf("args begin: %p, argc: %d, argv begin: %p, argv[0] value: %s\n", ustack, *(uint32_t *)ustack, ustack + 4, *(char **)(ustack + 4));
}
