#ifndef __TRACE_H__
#define __TRACE_H__

#include <common.h>
#include <elf.h>
#include <device/map.h>

#define ST_FUNC 18
#define MAX_NR_IRB 128
#define MAX_NR_MRWB 256
#define MAX_NR_FTB 64
#define MAX_NR_ETB 64
#define SINGLE_BUF_LEN 128
#define MAX_ADD_DISPLAY_LEN 18
typedef struct trace_buf{
	int cur_len; int st_index;
	char *buf[256];
} tr_buf;
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

//TODO:Unused, but prepared for function ring trace
typedef struct function_trace_buf{
	int cur_len;
	int st_index;
	char *buf[MAX_NR_FTB];
} FTB;
// TODO:Maybe adaptaion for 64 bits is needed
void init_ftrace(const char *elf_file);
void call_to_ftrace(uint32_t dst_pc);
void ret_to_ftrace(uint32_t src_pc);

void read_dtrace(uint32_t addr, int len, IOMap *map, uint32_t ret);
void write_dtrace(uint32_t addr, int len, uint32_t data, IOMap *map);
#endif
