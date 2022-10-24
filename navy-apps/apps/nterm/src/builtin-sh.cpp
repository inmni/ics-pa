#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
	term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}
// From NEMU sdb
static int cmd_run(char *);		static int helper_run(char *);
static int cmd_exit(char *);	static int helper_exit(char *);
static int cmd_help(char *);	static int helper_help(char *);
int void_cmd(char * _){sh_printf("to implement"); return 0;}
#define NR_CMD __get_nr_cmd()
static struct {
		const char *name;
		const char *description;
		int (*helper)(char *);
		int (*handler)(char *);
}	cmd_table [] = {
		{ "help", "Display information about commands", helper_help, cmd_help},
		{ "run", "Execute file", helper_run, cmd_run},
		{ "exit", "Quit terminal", helper_exit, cmd_exit},
};
static inline int __get_nr_cmd(){
		static int __nr_cmd = (int)(sizeof(cmd_table) / sizeof(cmd_table[0]));
		return __nr_cmd;
}
static void sh_handle_cmd(const char *_cmd) {
		char *cmd = (char *)malloc(strlen(_cmd)+1);
		strcpy(cmd, _cmd);
		for(int i = 0;i < strlen(cmd);i++){
				printf("%d ",cmd[i]);
		}
		printf("\n");
		char *end = cmd + strlen(cmd);
		
		char *cmd_name = strtok(cmd, " ");
		if(cmd_name == NULL)	return;

		char *args = cmd + strlen(cmd_name) + 1;
		if(args >= end)	args = NULL;

		int i;	int ret;
		for(i = 0; i < NR_CMD; i++){
				if(strcmp(cmd_name, cmd_table[i].name) == 0){
						if(cmd_table[i].handler(args)){
								cmd_table[i].helper(args);
						}
						break;
				}
		}
		if(i == NR_CMD)	{ sh_printf("Unknown command '%s'\n", cmd_name); }
		free(cmd);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
					sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
static int cmd_help(char *args){
		char *cmd_name = strtok(NULL, " ");
		if(cmd_name==NULL)	return 1;
		int i;
		for(i = 0; i<NR_CMD; i++){
				if(strcmp(cmd_name, cmd_table[i].name)==0){
						cmd_table[i].helper(strtok(NULL, " "));
						return 0;
				}
		}
		sh_printf("No found command: '%s'", cmd_name);
		return 0;
}
static int cmd_run(char *args){
		char *img_to_run = strtok(NULL, "");
		if(img_to_run==NULL)	return 1;
		execve(img_to_run, NULL, NULL);
		return 0;
}
static int cmd_exit(char *_){exit(0);return 0;}
static int helper_help(char *_){
		sh_printf("Show help about the command\nFor example:\n	help run\n");
		return 0;
}
static int helper_run(char *_){return 0;}
static int helper_exit(char *_){return 0;}
