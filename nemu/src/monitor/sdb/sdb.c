/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <watchpoint.h>
#include "sdb.h"
static int is_batch_mode = false;
void init_regex();
word_t paddr_read(paddr_t addr,int len);
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	nemu_state.state=NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static int cmd_help(char *args);
static int cmd_temp(char *args);
static int cmd_save(char *args);
static int cmd_load(char *args);
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step execution",cmd_si},
  { "info", "Print the info of Registers or Watchpoints",cmd_info},
  { "x", "Print the 4N Bytes from EXPR address",cmd_x},
  { "p", "Get the value of EXPR", cmd_p},
  { "w", "Set a watchpoint at EXPR address", cmd_w},
  { "d", "Delete the N_th watchpoint",cmd_d},
  /* TODO: Add more commands */
  { "t", "Temp",cmd_temp},
	{ "save", "Save snap to the path file",cmd_save},
	{ "load", "Load snap from the path file",cmd_load},
};
typedef struct __Nemu_Snap_Session_Header{
	uint32_t offset;
	size_t size;
}	NS_shdr;
typedef struct __Nemu_Snap_Header{
	uint32_t indent;
	uint32_t version;
	NS_shdr pc_shdr;
	NS_shdr sr_shdr;
	NS_shdr gpr_shdr;
	NS_shdr pmem_shdr;
	NS_shdr rtc_shdr;
	NS_shdr data_shdr;
	NS_shdr vga_shdr;
	NS_shdr audio_shdr;
	NS_shdr disk_shdr;
	NS_shdr serial_shdr;
	NS_shdr fb_shdr;
	NS_shdr sb_shdr;
} NS_hdr;
static int cmd_load(char *args){
	return 0;
}
static int cmd_save(char *args){
	char *path = strtok(NULL, " ");
	if(path==NULL){
		printf("Please input the path to save\n");
	}

	FILE* file = NULL;
	file = fopen(path, "w+");
	if(file==NULL){
		printf("Open file '%s' error\n", path);
		return 0;
	}
	// Save headers
	NS_hdr ns_hdr;
	ns_hdr.indent = 0;	ns_hdr.version = 0;
	NS_shdr pc_shdr = {sizeof(ns_hdr.indent) + sizeof(ns_hdr.version), sizeof(cpu.pc)};	
	NS_shdr sr_shdr = {pc_shdr.offset + pc_shdr.size, sizeof(cpu.sr)};
	NS_shdr gpr_shdr = {sr_shdr.offset + sr_shdr.size, sizeof(cpu.gpr)};
	NS_shdr pmem_shdr = {gpr_shdr.offset + gpr_shdr.size, CONFIG_MSIZE};
#ifdef CONFIG_DEVICE
	NS_shdr rtc_shdr = {pmem_shdr.offset + pmem_shdr.size, 7};
	NS_shdr data_shdr = {rtc_shdr.offset + rtc_shdr.size, 4};
	NS_shdr vga_shdr = {data_shdr.offset + data_shdr.size, 8};
	NS_shdr audio_shdr = {vga_shdr.offset + vga_shdr.size, 24};
	NS_shdr disk_shdr = {audio_shdr.offset + audio_shdr.size, 0};
	NS_shdr serial_shdr = {disk_shdr.offset + disk_shdr.size,8};
	NS_shdr fb_shdr = {serial_shdr.offset + serial_shdr.size, MUXDEF(CONFIG_VGA_SIZE_400x300, 800*600, 400*300)*sizeof(uint32_t)};
	NS_shdr sb_shdr = {fb_shdr.offset + fb_shdr.size, 0};
#endif
	ns_hdr.pc_shdr = pc_shdr;		ns_hdr.sr_shdr = sr_shdr;
	ns_hdr.gpr_shdr = gpr_shdr;	ns_hdr.pmem_shdr = pmem_shdr;
	ns_hdr.rtc_shdr = rtc_shdr; ns_hdr.data_shdr = data_shdr;
	ns_hdr.vga_shdr = vga_shdr;	ns_hdr.audio_shdr = audio_shdr;
	ns_hdr.disk_shdr = disk_shdr;	ns_hdr.serial_shdr = serial_shdr;
	ns_hdr.fb_shdr = fb_shdr;		ns_hdr.sb_shdr = sb_shdr;
	
	fseek(file, 0, SEEK_SET);
	// Save headers
	fwrite(&ns_hdr, sizeof(NS_hdr), 1, file);
	// Save registers
	fseek(file, pc_shdr.offset, SEEK_SET);
	fwrite(&cpu.pc, pc_shdr.size, 1, file);
	
	fseek(file, sr_shdr.offset, SEEK_SET);
	fwrite(cpu.sr, sr_shdr.size, 1, file);

	fseek(file, gpr_shdr.offset, SEEK_SET);
	fwrite(cpu.gpr, gpr_shdr.size, 1, file);
	// Save memory
	fseek(file, pmem_shdr.offset, SEEK_SET);
	printf("To save physics memory\n");
	fwrite((void *)(CONFIG_MBASE), pmem_shdr.size, 1, file);
	// Save device
#ifdef CONFIG_DEVICE
	printf("To save rtc mmio\n");
	fseek(file, rtc_shdr.offset, SEEK_SET);
	printf("[0x%08x, 0x%08x]\n", CONFIG_RTC_MMIO, (uint32_t)(CONFIG_RTC_MMIO+rtc_shdr.size));
	fwrite((void *)(CONFIG_RTC_MMIO), rtc_shdr.size, 1, file);
	
	printf("To save data mmio\n");
	fseek(file, data_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_I8042_DATA_MMIO), data_shdr.size, 1, file);

	printf("To save vga ctl mmio\n");
	fseek(file, vga_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_VGA_CTL_MMIO), vga_shdr.size, 1, file);

	fseek(file, audio_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_AUDIO_CTL_MMIO), audio_shdr.size, 1, file);

	fseek(file, disk_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_DISK_CTL_MMIO), disk_shdr.size, 1, file);

	fseek(file, serial_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_SERIAL_MMIO), serial_shdr.size, 1, file);

	printf("To save fb content\n");
	fseek(file, fb_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_FB_ADDR), fb_shdr.size, 1, file);

	printf("To save sb content\n");
	fseek(file, sb_shdr.offset, SEEK_SET);
	fwrite((void *)(CONFIG_SB_ADDR), sb_shdr.size, 1, file);
#endif
	// Save watchpoints
	//
	printf("To close the file\n"); 
	fclose(file);
	printf("Save snap successfully\n");
	return 0;
}
static int cmd_si(char *args){
	uint64_t n =1;
	/* args have been stored in a static buffer
	 * at sdb_mainloop()*/
	char *token = strtok(NULL, " ");
	if(token){
		n = strtoull(token,NULL,10);//parse token to uint64_t
		if(!n){
			printf("Wrong argument! It should be integer!\n");
			return 0;
		}
	}
	cpu_exec(n);
	return 0;
}
static int cmd_info(char *args){
	char *token = strtok(NULL, " ");
	if(!token){
		return 0;
	}
	if(!strcmp(token,"r")){
		isa_reg_display();
	}
	else if(!strcmp(token,"w")){
		token = strtok(NULL," ");
		if(!token){
			wp_all_display();
			return 0;
		}
		int n = atoi(token);// 0 or error, and to display 0th watchpoint
		if(n==-1){
			fwp_all_display();
			return 0;
		}
		wp_display(n);
	}
	return 0;
}
static int cmd_x(char *args){
	char *arg1 = strtok(NULL," ");
	int N;
	if(!arg1||(N=atoi(arg1))==0){
		printf("Wrong argument1!\n");	return 0;
	}
	char *arg2 = strtok(NULL," ");
	if(!arg2){
		printf("Wrong argument2!\n");	return 0;
	}
	bool success;
	word_t expr_val = expr(arg2,&success);
	if(!success){
		printf("Failed to convert %s to %d",arg2,expr_val);
		return 0;
	}
	for(;N>0;N--){
		printf("address:0X%08X	value:0X%08X\n",expr_val,paddr_read(expr_val,4));
		expr_val+=4;
	}
	return 0;
}
static int cmd_p(char *args){
	char *arg = strtok(NULL, "");
	if(arg==NULL){
		printf("No argument!\n");
	}
	bool success = false;
	printf("%i\n",expr(arg,&success));
	return 0;
}
static int cmd_w(char *args){
	if(args==NULL){
		printf("No arguments!\n");
	}
	new_wp(args);

	return 0;
}
static int cmd_d(char *args){
	char *arg = strtok(NULL," ");
	if(arg==NULL){
		printf("No argument!\n");
	}
	int n = atoi(arg);
	WP* wp_to_free = find_wp(n);
	free_wp(wp_to_free);
	return 0;
}
//int test(int loop);
static int cmd_temp(char *args){
	//test(100);
	for(int i=0;i<1000;i++){
		for(int j=0;j<1000;j++){
			printf("%d",i*j);
		}
		printf("\n");
	}
	return 0;
}
#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
