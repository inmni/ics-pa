#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
typedef struct watchpoint{
	int NO;
	struct watchpoint *next;
	char EXPR[32]; int oldValue;
} WP;
void init_wp_pool();
WP* find_wp(int n);
WP* get_wp_head();
WP* new_wp(char *EXPR);
void free_wp(WP* wp);
void wp_all_display();
void wp_display(int n);
void fwp_all_display();
#endif
