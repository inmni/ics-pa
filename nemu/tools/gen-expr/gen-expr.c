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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
const char *regs[];
// this should be enough
static char* ops[] = {
"&&","||","!=","<=",">=",
"+","-","*","/","=="
}
static char* pre_ops[]={
"$","*"
}
static char* num[]={
'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
}
#define NR_OP ARRLEN(ops)
#define NR_PRE_OP ARRLEN(pre_ops)
#define MAX_NR_BUF 65536
#define TEST_32
#define MAX_DIGIT 8
static int nr_buf=0;
static char buf[MAX_NR_BUF] = {};
static char code_buf[MAX_NR_BUF + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
void gen_dnum(int digit);
void gen_hnum(int digit);
void gen_str(char *str);
void gen_char(char ch);
void gen_num();
void gen_op();
uint32_t choose(uint32_t num){
	return rand()%num;
}
static void gen_rand_expr();
static void gen_rand_2expr(){
	if(nr_buf>=MAX_NR_BUF-64){
		return;
	}
	switch(choose(2)){
		case 0:gen_char('$');gen_str(regs[choose(32)]);break;
		case 1:gen_char('*');gen_expr();break;
	}
}
static void gen_rand_expr() {
	if(nr_buf>=MAX_NR_BUF-64){
		return;
	}
  switch(choose(4)){
	case 0:gen_num();break;
	case 1:gen_char('(');gen_rand_expr();gen_char(')');break;
	case 2:gen_char('(');gen_rand_2expr();gen_char(')');break;
	default: gen_rand_expr();gen_op();gen_rand_expr();break;
  }
}
void gen_op(){
	if(nr_buf>=MAXNR_BUF-64){
		return;
	}
	gen_str(ops[choose(NR_OP)]);
}
void gen_num(){
	if(nr_buf>=MAX_NR_BUF-64){
		return;
	}
#ifdef TEST_32
	int digit = choose(32)+1;
#else
	int digit = choose(MAX_DIGIT)+1;
#endif
	switch(choose(2)){
		case 0:gen_dnum(digit);
		case 1:gen_hnum(digit);
	}
	return;
}
void gen_dnum(int digit){
	for(;digit>0;digit--){
		gen_char(num[choose(10)]);
	}
}
void gen_hnum(int digit){
	gen_str("0x");
	for(;digit>0;digit--){
		gen_char(num[choose(16)]);
	}
}
void gen_str(char* str){
	int str_len = strlen(str);
	strncpy(&buf[nr_buf],str,str_len);
	nr_buf+=str_len;
	return;
}
void gen_char(char ch){
	buf[nr_buf] = ch;
	nr_buf++;
	return;
}
void rand_expr(int loop) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
	buf[nr_buf]='\0';
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return;
}
