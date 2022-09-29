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
