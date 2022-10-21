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
	fs_info(fd);
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

			fs_read(fd, &phdr, sizeof(Elf_Phdr));

			if(phdr.p_type != PT_LOAD)continue;
			
			fs_lseek(fd, phdr.p_offset, SEEK_SET);
			fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
			memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
	}

	fs_close(fd);

	return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %d", entry);
  ((void(*)())entry) ();
}

