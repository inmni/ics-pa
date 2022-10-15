#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
typedef struct watchpoint{
	int NO;
	struct watchpoint *next;
	char EXPR[32];
	int oldValue,targetValue;
} WP;
WP* find_wp(int n);
void init_wp_pool();
void print_wp(WP* wp);
void wp_display(int n);
void wp_all_display();
void fwp_all_display();
WP* get_wp_head();
WP* find_wp(int n);
WP* new_wp(char *EXPR);
void free_wp(WP* wp);
#endif
