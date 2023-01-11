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


#define NR_PAGE 8
#define PAGESIZE 4096

#define MAX(a, b)((a) > (b) ? (a) : (b))

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

void context_kload(PCB *pcb, void (*entry)(void *), void *arg){
  Area karea;
  karea.start = &pcb->cp;
  karea.end = &pcb->cp + STACK_SIZE;

  pcb->cp = kcontext(karea, entry, arg);
  printf("kcontext地址为:%p\n", pcb->cp);
}

static size_t ceil_4_bytes(size_t size){
  if (size & 0x3)
    return (size & (~0x3)) + 0x4;
  return size;
}


void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
  int envc = 0, argc = 0;
  AddrSpace *as = &pcb->as;
  protect(as);
  
  if (envp){
    for (; envp[envc]; ++envc){}
  }
  if (argv){
    for (; argv[argc]; ++argc){}
  }
  char *envp_ustack[envc];

  void *alloced_page = new_page(NR_PAGE) + NR_PAGE * 4096; //得到栈顶

  //这段代码有古怪，一动就会出问题，莫动
  //这个问题确实已经被修正了，TMD，真cao dan
  // 2021/12/16
  
  map(as, as->area.end - 8 * PAGESIZE, alloced_page - 8 * PAGESIZE, MMAP_READ | MMAP_WRITE); 
  map(as, as->area.end - 7 * PAGESIZE, alloced_page - 7 * PAGESIZE, MMAP_READ | MMAP_WRITE);
  map(as, as->area.end - 6 * PAGESIZE, alloced_page - 6 * PAGESIZE, MMAP_READ | MMAP_WRITE); 
  map(as, as->area.end - 5 * PAGESIZE, alloced_page - 5 * PAGESIZE, MMAP_READ | MMAP_WRITE);
  map(as, as->area.end - 4 * PAGESIZE, alloced_page - 4 * PAGESIZE, MMAP_READ | MMAP_WRITE); 
  map(as, as->area.end - 3 * PAGESIZE, alloced_page - 3 * PAGESIZE, MMAP_READ | MMAP_WRITE);
  map(as, as->area.end - 2 * PAGESIZE, alloced_page - 2 * PAGESIZE, MMAP_READ | MMAP_WRITE); 
  map(as, as->area.end - 1 * PAGESIZE, alloced_page - 1 * PAGESIZE, MMAP_READ | MMAP_WRITE); 
  
  char *brk = (char *)(alloced_page - 4);
  // 拷贝字符区
  for (int i = 0; i < envc; ++i){
    brk -= (ceil_4_bytes(strlen(envp[i]) + 1)); // 分配大小
    envp_ustack[i] = brk;
    strcpy(brk, envp[i]);
  }

  char *argv_ustack[envc];
  for (int i = 0; i < argc; ++i){
    brk -= (ceil_4_bytes(strlen(argv[i]) + 1)); // 分配大小
    argv_ustack[i] = brk;
    strcpy(brk, argv[i]);
  }
  
  intptr_t *ptr_brk = (intptr_t *)(brk);

  // 分配envp空间
  ptr_brk -= 1;
  *ptr_brk = 0;
  ptr_brk -= envc;
  for (int i = 0; i < envc; ++i){
    ptr_brk[i] = (intptr_t)(envp_ustack[i]);
  }

  // 分配argv空间
  ptr_brk -= 1;
  *ptr_brk = 0;
  ptr_brk = ptr_brk - argc;
  
  // printf("%p\n", ptr_brk);
  printf("%p\t%p\n", alloced_page, ptr_brk);
  //printf("%x\n", ptr_brk);
  //assert((intptr_t)ptr_brk == 0xDD5FDC);
  for (int i = 0; i < argc; ++i){
    ptr_brk[i] = (intptr_t)(argv_ustack[i]);
  }

  ptr_brk -= 1;
  *ptr_brk = argc;
  
  //这条操作会把参数的内存空间扬了，要放在最后
  uintptr_t entry = loader(pcb, filename);
  Area karea;
  karea.start = &pcb->cp;
  karea.end = &pcb->cp + STACK_SIZE;

  Context* context = ucontext(as, karea, (void *)entry);
  pcb->cp = context;

  printf("新分配ptr=%p\n", as->ptr);
  printf("UContext Allocted at %p\n", context);
  printf("Alloced Page Addr: %p\t PTR_BRK_ADDR: %p\n", alloced_page, ptr_brk);

  ptr_brk -= 1;
  *ptr_brk = 0;//为了t0_buffer
  //设置了sp
  context->gpr[2]  = (uintptr_t)ptr_brk - (uintptr_t)alloced_page + (uintptr_t)as->area.end;

  //似乎不需要这个了，但我还不想动
  context->GPRx = (uintptr_t)ptr_brk - (uintptr_t)alloced_page + (uintptr_t)as->area.end + 4;
  //context->GPRx = (intptr_t)(ptr_brk);
}
