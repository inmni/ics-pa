#include <trace.h>

void buf_mem_op(M_RW_B *mrwb, uint32_t addr, int len, uint32_t data, int op){
	if(mrwb->cur_len < MAX_NR_MRWB){
			mrwb->buf[mrwb->cur_len] = (char *)malloc(SINGLE_BUF_LEN);
			
			parse_mem_op(mrwb->buf[mrwb->cur_len], addr, len, data, op);

			mrwb->cur_len++;
	}
	else{
			parse_mem_op(mrwb->buf[mrwb->st_index], addr, len, data, op);
			
			mrwb->st_index++;
			mrwb->st_index %= MAX_NR_MRWB;
	}
}

void parse_mem_op(char *out, uint32_t addr, int len, uint32_t data, int op){
		out += snprintf(out, 16, FMT_WORD ":", addr);
		switch(len){
				case 1: out += snprintf(out, 4, " %02x", data);break;
				case 2: out += snprintf(out, 8, " %04x", data);break;
				case 4: out += snprintf(out, 16, " %08x", data);break;
				IFDEF(CONFIG_ISA64, case 8: out +=snprintf(out, 32, " %16ull", data); break);
				default: assert(0);
		}
		memset(out, ' ', (MAX_ADD_DISPLAY_LEN - len)*3 + 1);
		switch(op){
				case M_WRITE: out += sprintf(out, " write ");break;
				case M_READ:  out += sprintf(out, " read  ");break;
				default: assert(0);
		}

}

Elf32_Sym *sym_table;
char *str_table;
void init_ftrace(const char *elf_file){
		FILE *file = fopen(elf_file, "r");
		assert(file!=NULL);
		int fr_r;	
		Elf32_Ehdr ehdr;
		Elf32_Shdr *shdrs;
		//Read the ELF header
		fr_r = fread(&ehdr, 1, sizeof(ehdr), file);
		assert(fr_r);

		//Read the section headers
		shdrs = (Elf32_Shdr *)malloc(ehdr.e_shnum*sizeof(Elf32_Shdr));//TODO: to c style
		fseek(file, ehdr.e_shoff, SEEK_SET);
		fr_r = fread(shdrs, ehdr.e_shnum, sizeof(Elf32_Shdr), file);
		
		//Read the Symbol and String table
		int s_idx = 0;
		for(; s_idx < ehdr.e_shnum; s_idx++){
				Elf32_Shdr *sh = &shdrs[s_idx];
				if(sh->sh_type == SHT_SYMTAB){
					sym_table = (Elf32_Sym *)malloc(sh->sh_size*2);
					fseek(file, sh->sh_offset, SEEK_SET);
					fr_r = fread(sym_table, sizeof(Elf32_Sym), sh->sh_size, file);
					printf("Get symbol table result:%d, %ld bytes per unit, offset:%d, size:%d\n", fr_r, sizeof(Elf32_Sym), sh->sh_offset, sh->sh_size);
				}
				else if(sh->sh_type == SHT_STRTAB){
					str_table = (char *)malloc(sh->sh_size*2);
					fseek(file, sh->sh_offset, SEEK_SET);
					fr_r = fread(str_table, sizeof(char), sh->sh_size, file);
					printf("Get string table result:%d, %ld bytes per unit, offset:%d, size:%d\n", fr_r, sizeof(char), sh->sh_offset, sh->sh_size);
					break;
				}
		}
		printf("str_table:%s\n", str_table);
		free(sym_table);
		free(str_table);
		free(shdrs);
		fclose(file);
}
