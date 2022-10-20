#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define uintN		uint64_t
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define uintN		uint32_t
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fr_r, i;
	uintN offset;
	Elf_Ehdr ehdr; Elf_Phdr phdr;
	
	fr_r = ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
	printf("%d\n", *(uint32_t *)ehdr.e_ident);	
	assert(fr_r==sizeof(Elf_Ehdr));
	assert(*(uint32_t *)ehdr.e_ident == 1/*To complete*/);
	assert(ehdr.e_phoff!=0);
	
	offset = ehdr.e_phoff;
	for(i = 0; i < ehdr.e_phnum; i++){
			offset += ehdr.e_phentsize;
			ramdisk_read(&phdr, offset, sizeof(Elf_Phdr));

			if(phdr.p_type != PT_LOAD)continue;
			
	}
	return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

