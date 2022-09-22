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


/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <isa.h>
#include <regex.h>
//const char *regs[];
// isa_reg_str2val(const char *s,bool *success);
enum {
  TK_NOTYPE = 256, TK_EQ,
	TK_DNUM,TK_HNUM,
	TK_A,
  /* TODO: Add more token types */
  TK_REG,
  TK_NEQ,
  TK_GEQ,
  TK_LEQ,
  TK_NOT,
  TK_AND,
  TK_OR,
  TK_REF,
  TK_DEREF,
  TK_NEG,
  TK_PLUS,
  TK_SUB,
  TK_MUL,
  TK_DIV,
  TK_LPA,
  TK_RPA,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* Add more rules.
   * Pay attention to the precedence level of different rules.
   */
	{"\\$", TK_REG},
	{"[$a-z][a-z0-9]+\\b",TK_A},
	{"&&",TK_AND},
	{"\\|\\|",TK_OR},
	{"!=",TK_NEQ},
	{"<=",TK_LEQ},
	{">=",TK_GEQ},
	{"!", TK_NOT},
	{"&", TK_REF},
  {" +", TK_NOTYPE},    // spaces
  {"\\b[0-9]+\\b",TK_DNUM},		// dec number
  {"\\b0[xX][0-9a-zA-Z]+",TK_HNUM},		// hex number
  {"\\+", TK_PLUS},         // plus
  {"\\-", TK_SUB},		// sub
  {"\\*", TK_MUL},		// mul
  {"\\/", TK_DIV},		// div
  {"\\(", TK_LPA},		// lpa
  {"\\)", TK_RPA},		// rpa
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)
#define NR_TK_STR 32
static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[NR_TK_STR];
} Token;
word_t paddr_read(paddr_t addr,int len);
int check_deref(int type){
	switch(type){
		case TK_PLUS:
		case TK_SUB:
		case TK_MUL:
		case TK_DIV:
			return 1;
		default:
			return 0;
	}
}
int check_neg(int type){
	switch(type){
		case TK_PLUS:
		case TK_SUB:
		case TK_MUL:
		case TK_DIV:
		case TK_LPA:
			return 1;
		default:
			return 0;
	}
}
static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while ( e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
       if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

         /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	 tokens[nr_token].type = rules[i].token_type;
	 memset(tokens[nr_token].str,0,NR_TK_STR);
	 memcpy(tokens[nr_token].str,substr_start,substr_len);
         //strncpy(tokens[nr_token].str, substr_start,substr_len);
	 nr_token++;

         switch (tokens[nr_token].type) {
		case TK_DNUM:
		case TK_HNUM:
  			if(substr_len>32){
                                printf("A number's length in this expr is longer than 32\n");
                                return 0;
                          }
		case TK_MUL:
	 		if(i>0&&check_deref(tokens[i-1].type)){
				tokens[i].type = TK_DEREF;
 	 		}
			break;
		case TK_SUB:
			if(i==0||check_neg(tokens[i-1].type)){
				tokens[i].type = TK_NEG;
 	 		}
			break;
		default: break;printf("expr.c:no special setting for type %d\n",rules[i].token_type);
         }  

        break;
       }   
     }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return 0;
    }
  }

  return 1;
}
int check_parentheses(int left,int right){
	if(tokens[left].type!=TK_LPA||tokens[right].type!=TK_RPA){
		return 0;
	}
	int left_left_par = 0;
	for(int i=left+1;i<right;i++){
		if(tokens[i].type==TK_LPA){
			left_left_par++;
		}
		else if(tokens[i].type==TK_RPA){
 			if(left_left_par){
				left_left_par--;
			 }
 			else{
				return 0;
			} 
		}
	}
	return !left_left_par;
}
int getPr(int type){
	switch(type){
		case TK_DNUM:
		case TK_HNUM:
		case TK_A:
			return 100;
		case TK_DEREF:
		case TK_REG:
		case TK_NOT:
		case TK_PLUS:
		case TK_SUB:
			return 2;
		case TK_MUL:
		case TK_DIV:
			return 4;
		case TK_EQ:
		case TK_NEQ:
			return 8;
		case TK_AND:
			return 12;
		case TK_OR:
			return 13;
		default:
			return 0;
	}
}
int cut(int left, int right){
	int res=left;
	int temp;
	int lPr = 9999999;
	int paCount = 0;
	for(int i=left;i<=right;i++){
		if(tokens[i].type==TK_LPA){
			paCount++;
			continue;
		}
		else if(tokens[i].type==TK_RPA){
			paCount--;
			continue;
		}
		if(paCount){
			continue;
		}
		temp = getPr(tokens[i].type);
		printf("%d\t",temp);
		if(temp<=lPr){
			lPr = temp;
			res = i;
		}
	}
	return res;
}
word_t eval_expr(int left,int right){
	if(left>right){
		//Bad expression
		return 0;
	}
	else if(left==right){
		//It must be a number
		if(tokens[left].type==TK_DNUM){
			//dec
			return strtoul(tokens[left].str,NULL,10);
		}
		else if(tokens[left].type==TK_HNUM){
			//hex
			//printf("%s\n",tokens[left].str);
			//printf("%08X\n",(uint32_t)strtoul(tokens[left].str,NULL,16));
			return strtoul(tokens[left].str,NULL,16);
		}
		printf("Not a number found in \" left==right\" in expr.c(eval_expr)");
		assert(0);
	}
	else{ 
		if(check_parentheses(left,right)){
			return eval_expr(left+1,right-1);
	 	}
		int cut_point = cut(left,right);
		printf("cut point:%d\n",cut_point);
		uint32_t left_val = 0;
		if(tokens[cut_point].type!=TK_DEREF || tokens[cut_point].type!=TK_NEG){
			left_val = eval_expr(left,cut_point-1);
		}
		
		uint32_t right_val = 0;
		if(tokens[cut_point].type!=TK_REG){
			right_val = eval_expr(cut_point+1,right);
		}

		switch(tokens[cut_point].type){
			case TK_PLUS:return left_val+right_val;
			case TK_SUB:return left_val-right_val;
			case TK_MUL:return left_val*right_val;
			case TK_DIV:return left_val/right_val;
			case TK_REG:
				    bool success; 
				    uint32_t r= isa_reg_str2val(tokens[cut_point+1].str,&success);
				    printf("%08X\n",r);
				    return r;
			case TK_DEREF:return paddr_read(right_val,4);
			case TK_NEG:return -right_val;
			case TK_EQ:return left_val==right_val;
			case TK_NEQ:return left_val!=right_val;
			case TK_LEQ:return left_val<=right_val;
			case TK_GEQ:return left_val>=right_val;

			case TK_AND:return left_val&&right_val;
			case TK_OR:return left_val||right_val;
			default:assert(0);
	 	}
		return 0;
	}
}
word_t expr(char *e, int *success) {
  if (!make_token(e)) {
    *success = 0;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  word_t result = eval_expr(0,nr_token-1);
  *success = 1;
  return result;
}
