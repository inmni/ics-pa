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
#include <isa.h>
extern const char* regs[];
// this should be enough
const char* ops[] = {
"+","-","*",
};
const char* pre_ops[]={
"$","-",
};
const char num[]={
'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
};
#define NR_OP 3
#define NR_PRE_OP 2
#define MAX_NR_BUF 65536
//#define TEST_32
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
static void gen_dnum(int digit);
static void gen_hnum(int digit);
static void gen_str(const char *str);
static void gen_char(char ch);
static void gen_num();
static void gen_op();
//static void gen_blankspace();
word_t expr(char *e,bool *success);
uint32_t choose(uint32_t num){
	return rand()%num;
}
static void gen_rand_expr();
static void gen_rand_2expr(){
	switch(choose(2)){
		case 0:gen_char('-');gen_rand_expr();break;//gen_char('$');gen_str(regs[choose(32)]);break;
		case 1:gen_char('-');gen_rand_expr();break;
	}
}
static void gen_rand_expr() {
	extern int nr_buf;
	if(nr_buf>=(MAX_NR_BUF>>1)){
		gen_num();
		return;
	}
//	gen_blankspace();
  switch(choose(4)){
	case 0:gen_num();break;
	case 1:gen_char('(');gen_rand_expr();gen_char(')');break;
	case 2:gen_char('(');gen_rand_2expr();gen_char(')');break;
	default: ;gen_rand_expr();gen_op();gen_rand_expr();break;
  }
 // gen_blankspace();
}
//static void gen_blankspace(){
//int count =choose(16);
//for(;count>0;count--){
//gen_char(' ');
//}
//}
static void gen_op(){
	gen_str(ops[choose(NR_OP)]);
}
static void gen_num(){
#ifdef TEST_32
	int digit = choose(32)+1;
#else
	int digit = choose(MAX_DIGIT)+1;
#endif
	switch(choose(2)){
		case 0:gen_dnum(digit);break;
		case 1:gen_hnum(digit);break;
	}
	return;
}
static void gen_dnum(int digit){
	digit--;
	gen_char(num[choose(9)+1]);
	for(;digit>1;digit--){
		gen_char(num[choose(10)]);
	}
}
static void gen_hnum(int digit){
	gen_str("0x");
	for(;digit>0;digit--){
		gen_char(num[choose(16)]);
	}
	gen_char(' ');
}
static void gen_str(const char* str){
	extern int nr_buf;
	int str_len = strlen(str);
	strncpy(&buf[nr_buf],str,str_len);
	nr_buf+=str_len;
	return;
}
static void gen_char(char ch){
	extern int nr_buf;
	buf[nr_buf] = ch;
	nr_buf++;
	return;
}
int test(int argc) {
  extern int nr_buf;
  int seed = time(0);
  srand(seed);
  int loop = argc;
//  if (argc > 1) {
//	      sscanf(argv[1], "%d", &loop);
//  }
  int i;
  bool success = false;
  uint32_t temp = 0;
  //FILE *res = fopen("/tmp/result.txt","w");  assert(res!=NULL);  
  for (i = 0; i < loop; i ++) {
printf("before reset\n");
	  memset(buf,0,MAX_NR_BUF);
	  nr_buf=0;
   // printf("before generate");
	  gen_rand_expr();
	buf[nr_buf]='\0';
//	printf("expression:%s\n",buf);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -w /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    printf("to get the correct anwser\n");
    if(!fscanf(fp, "%d", &result)){pclose(fp);continue;}
    else{pclose(fp);}
    temp = expr(buf,&success);
    if(temp!=result){
    	printf("%s\ncorrect:%u,expr:%u\n",buf,result,temp);
    	assert(0);
    }
    //fprintf(res,"%u %s\n",result,buf);
    //printf("%u %s\n", result, buf);
  }
  //pclose(res);
  return 0;
}
