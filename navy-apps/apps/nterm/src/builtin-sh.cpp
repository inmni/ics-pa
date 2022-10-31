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
static int cmd_export(char *);static int helper_export(char *);
static int cmd_echo(char *);	static int helper_echo(char *);
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
		{ "echo", "Output the value of the expression", helper_echo, cmd_echo},
		{ "export", "Add new variable", helper_export, cmd_export},
		{ "exit", "Quit terminal", helper_exit, cmd_exit},
};
static inline int __get_nr_cmd(){
		static int __nr_cmd = (int)(sizeof(cmd_table) / sizeof(cmd_table[0]));
		return __nr_cmd;
}
static void sh_handle_cmd(const char *_cmd) {
		int i = strlen(_cmd);	// used as the size of _cmd and then the index of cmd_table

		char *cmd = (char *)malloc(i+1);
		strcpy(cmd, _cmd);

		char *end = cmd + i - 1;
			*end-- = 0;// To set the last '\n' to 0
		char *cmd_name = strtok(cmd, " ");
		if(cmd_name == NULL)	return;

		char *args = cmd + strlen(cmd_name) + 1;
		if(args >= end)	args = NULL;

		int ret;
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
	setenv("PATH","/bin",0);
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
		char *argv[8]; int i = 0;
		char *img_to_run = strtok(args, " ");
		while(argv[i++] = strtok(NULL, " "));
		if(img_to_run==NULL)	return 1;
		printf("Try to run %s with arguments: %s and so on\n",img_to_run, argv[0]);
		execvp(img_to_run, argv);
		return 0;
}
static int cmd_exit(char *_){exit(0);return 0;}
static int cmd_echo(char *key){
		char *value = getenv(key);
		if(value==NULL){
				sh_printf("%s\n", key);
				return 0;
		}
		sh_printf("%s\n", value);
		return 0;
}
static int cmd_export(char *args){
		char *key; char *value;
		while((key = strtok(NULL, "="))){
				value = strtok(NULL, " ");
				if(value==NULL){
						sh_printf("No set for '%s' and the following all\n", key);
						break;
				}
				printf("%d\n", setenv(key, value, 1));
		}
		return 0;
}
static int helper_help(char *_){
		sh_printf("Show help about the command\nFor example:\n	help run\n");
		return 0;
}
static int helper_run(char *_){return 0;}
static int helper_exit(char *_){return 0;}
static int helper_export(char *_){return 0;}
static int helper_echo(char *_){return 0;}
