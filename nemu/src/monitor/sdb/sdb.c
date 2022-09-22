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
#include "sdb.h"
static int is_batch_mode = false;
void init_regex();
void init_wp_pool();
void wp_all_display();
void wp_display();
void fwp_all_display();
typedef struct watchpoint WP;
void free_wp(WP* wp);
WP* new_wp(char* EXPR);
WP* find_wp(int n);
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
};
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
		printf("Wrong argument1!\n");
		return 0;
	}
	char *arg2 = strtok(NULL," ");
	if(!arg2){
		printf("Wrong argument2!\n");
	}
	bool success;
	word_t expr_val = expr(arg2,&success);
	if(!success){
		printf("Failed to convert %s to %d",arg2,expr_val);
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
static int cmd_temp(char *args){
	FILE *test = fopen("/tmp/result.txt","r");
	char *line="";
	uint32_t result=0;
	uint32_t temp=0;
	bool success = false;
	printf("begin\n");
	while(fscanf(test,"%u %s",&result,line)){
		printf("once\n");
		temp = expr(line,&success);
		if(temp!=result){
			printf("%s\ncorrect:%u,expr:%u\n",line,result,temp);
		}
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
