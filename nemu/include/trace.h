#ifndef __TRACE_H__
#define __TRACE_H__

#define SINGLE_INST_LEN 32
#define MAX_NR_IRB 16

typedef struct inst_ring_buf{
	int cur_len;
	int st_index;
	char *insts[MAX_NR_IRB];
	uint64_t pc[MAX_NR_IRB];
	uint64_t codes[MAX_NR_IRB];
} IRB;
//IRB iRingBuffer;

#endif
