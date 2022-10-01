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
int sym_count;
char **str_table;
int str_count;

int str_split(char **out, char *src, const char *sep, size_t len, int flag);

void init_ftrace(const char *elf_file){
		FILE *file = fopen(elf_file, "r");
		assert(file!=NULL);
		fseek(file, 0, SEEK_END);
		printf("File bytes:%ld\n", ftell(file));
		fseek(file, 0, SEEK_SET);
		int fr_r;	
		Elf32_Ehdr ehdr;
		Elf32_Shdr *shdrs;
		char *tmp_str=NULL;
		//Read the ELF header
		fr_r = fread(&ehdr, sizeof(ehdr), 1, file);
		assert(fr_r);

		//Read the section headers
		shdrs = (Elf32_Shdr *)malloc(ehdr.e_shnum*sizeof(Elf32_Shdr));//TODO: to c style
		fseek(file, ehdr.e_shoff, SEEK_SET);
		fr_r = fread(shdrs, sizeof(Elf32_Shdr), ehdr.e_shnum, file);
		
		//Read the Symbol and String table
		int s_idx = 0;
		for(; s_idx < ehdr.e_shnum; s_idx++){
				Elf32_Shdr *sh = &shdrs[s_idx];
				if(sh->sh_type == SHT_SYMTAB){
					sym_table = (Elf32_Sym *)malloc(sh->sh_size);
					fseek(file, sh->sh_offset, SEEK_SET);
					sym_count = fread(sym_table, sizeof(Elf32_Sym), sh->sh_size/sizeof(Elf32_Sym), file);
					str_table = (char **)malloc(fr_r*sizeof(char *));
					printf("Get symbol table result:%d, %ld bytes per unit, offset:%d, size:%d\n", sym_count, sizeof(Elf32_Sym), sh->sh_offset, sh->sh_size);
				}
				else if(sh->sh_type == SHT_STRTAB){
					tmp_str = (char *)malloc(sh->sh_size);
					fseek(file, sh->sh_offset, SEEK_SET);
					fr_r = fread(tmp_str, sizeof(char), sh->sh_size/sizeof(char), file);
					
					str_count = str_split(str_table, tmp_str, "\0", fr_r, 1);

					printf("Get string table result:%d, %ld bytes per unit, offset:%d, size:%d\n", str_count, sizeof(char), sh->sh_offset, sh->sh_size);
					break;
				}
		}

		Elf32_Sym sym;
		for(int i=0;i<sym_count;i++){
				sym = sym_table[i];
				printf("value:%08x, st_name:%d",sym.st_value,sym.st_name);
				//if(sym.st_info!=STT_FUNC)continue;
				printf("%s\n",tmp_str+sym.st_name);
		}
		free(str_table);
		free(sym_table);
		free(shdrs);
		fclose(file);
}

void call_to_ftrace(uint32_t dst_pc){
		int idx = 0;
		Elf32_Sym sym;
		for(; idx<sym_count; idx++){
				sym = sym_table[idx];
				if(sym.st_info!=STT_FUNC)continue;
				if(dst_pc < sym.st_value || dst_pc >= sym.st_value+sym.st_size)continue;
				
		}
}

void ret_to_ftrace(uint32_t src_pc){

}

