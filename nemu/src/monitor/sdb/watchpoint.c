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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char EXPR[32];
  int currValue,targetValue;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
WP* find_wp(int n);
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}
void print_wp(WP* wp){
	printf("NO:%d,ADDRESS:%s,CURRENT_VALUE:%d,TARGET_VALUE:%d\n",wp->NO,wp->EXPR,wp->currValue,wp->targetValue);
}
/* TODO: Implement the functionality of watchpoint */
void wp_all_display(){
	if(head==NULL){
		printf("No watchpoints now!\n");
		return;
	}
	WP* wp = head;
	while(wp){
		print_wp(wp);
		wp = wp->next;
	}
}
void wp_display(int n){
	if(head==NULL){
		printf("No watchpoints now!\n");
		return;
	}
	WP* wp = find_wp(n);
	if(wp){
		print_wp(wp);
		return;
	}
	printf("NO.%d watchpoint does not exist or not active",n);
	return;
}
WP* find_wp(int n){
	WP* wp = head;
	while(wp){
		if(wp->NO==n){
			return wp;
		}
		wp = wp->next;
	}
	return NULL;
}
