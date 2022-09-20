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
#include <string.h>
#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char EXPR[32];
  int oldValue,targetValue;
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
	printf("NO:%d,EXPR:%s,CURRENT_VALUE:%d,TARGET_VALUE:%d\n",wp->NO,wp->EXPR,wp->oldValue,wp->targetValue);
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
void fwp_all_display(){
	if(free_==NULL){
		printf("No free watchpoints now!\n");
		return;
	}
	WP* wp = free_;
	while(wp){
		print_wp(wp);
		wp = wp->next;
	}
	return;
}
WP* get_wp_head(){
	return head;
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
WP* new_wp(char* EXPR){
	WP* wp = free_;
	if(wp==NULL){
		printf("No free watchpoints!\n");
		assert(0);
	}
	free_ = free_->next;
	wp->next = head;
	strncpy(wp->EXPR,EXPR,32);
	head = wp;
	bool success = false;
	wp->oldValue = expr(EXPR,&success);
	return wp;
}
void free_wp(WP* wp){
	if(wp==NULL){
		return;
	}
	if(wp==head){
		head = head->next;
		wp->next = free_;
		return;
	}
	WP* prev = head;
	WP* temp = head->next;
	while(temp){
		if(wp==temp){
			prev->next = temp->next;
			wp->next = free_;
			return;
		}
	}
	printf("No such watchpoint working!\n");
	return;
}
