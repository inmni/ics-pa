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
			//printf("to alloc %d pages [%08x, %08x) based on %08x\n",pg_nr, pg_start, pg_end, (uintptr_t)(pcb->as.ptr));
			void *pg_ptr = new_page(pg_nr);
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
			memset((void *)(pg_ptr + pg_off + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
			//printf("after memset\n");
	}

	fs_close(fd);
	return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB* p, void (*entry)(void *), void* arg) {
	Area kstack;
	kstack.start = &(p->cp);
	kstack.end = p->stack + sizeof(p->stack);

	p->cp = kcontext(kstack, entry, arg);
	p->prio = 1;
}

void context_uload(PCB* p, const char *filename, char *const argv[], char *const envp[]) {
	protect(&p->as);
	void *ustack = new_page(8);
	for(int i=0; i<8; i++){
		map(&p->as, 
		p->as.area.end - (8 - i)*PGSIZE, 
		ustack + i*PGSIZE, 
		MMAP_READ | MMAP_WRITE);
	}
	uint32_t* ustack_start = ustack + 4;
	uint32_t* ustack_end = ustack + STACK_SIZE;
//	printf("MALLOC [%p, %p)\n", ustack, ustack_end);
	// copy arguments
	int argv_c = 0; int envp_c = 0;
	while(argv && argv[argv_c]){
		ustack_end -= strlen(argv[argv_c]) + 1; // keep zero ternimating
		strcpy((char *)ustack_end, argv[argv_c]);
		*ustack_start++ = (uint32_t)ustack_end;
		argv_c++;
	}
	while(envp && envp[envp_c]){
		ustack_end -= strlen(envp[envp_c]) + 1;
		strcpy((char *)ustack_end, envp[envp_c]);
		*ustack_start++ = (uint32_t)ustack_end;
		envp_c++;
	}
	*(uint32_t *)ustack = argv_c + envp_c;
	Area kstack;
	kstack.start = p->stack;
	kstack.end = p->stack + STACK_SIZE;
//	printf("KERNEL stack [%p, %p)\n", kstack.start, kstack.end);
//	printf("try to load %s\n",filename);
	uintptr_t entry = loader(p, filename);
	printf("%s's entry: %08x\n",filename, entry);
	p->cp = ucontext(&(p->as), kstack, (void *)entry);
	p->cp->GPRx = (uintptr_t)ustack;
	p->prio = 1;
	printf("prio set:%d, addr: %p\n", p->prio, &p->prio);
//	printf("args begin: %p, argc: %d, argv begin: %p, argv[0] value: %s\n", ustack, *(uint32_t *)ustack, ustack + 4, *(char **)(ustack + 4));
}
