#ifndef __TRACE_H__
#define __TRACE_H__

#include <common.h>
#include <elf.h>

#define MAX_NR_IRB 16
#define MAX_NR_MRWB 256
#define SINGLE_BUF_LEN 128
#define MAX_ADD_DISPLAY_LEN 18
typedef struct inst_ring_buf{
	int cur_len;
	int st_index;
	char *buf[MAX_NR_IRB];
} IRB;
//IRB iRingBuffer;
typedef struct memory_rw_buf{
	int cur_len;
	int st_index;
	char *buf[MAX_NR_MRWB];
} M_RW_B;
enum{M_WRITE, M_READ};
void buf_mem_op(M_RW_B *mrwb, uint32_t addr, int len, uint32_t data, int op);
void parse_mem_op(char *out, uint32_t addr, int len, uint32_t data, int op);

typedef struct function_trace_buf{
	
} FTB;
// TODO:Maybe adaptaion for 64 bits is needed
Elf32_Sym *sym_table;
char *str_table;
void init_ftrace(const char *elf_file);
#endif
