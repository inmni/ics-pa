#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//If flag == 1, auto malloc for out
//return the count of split string;
int str_split(char **out, char *src, const char* sep, size_t len, int flag){
		
		size_t out_idx = 0;
		size_t src_idx = 0;
		if(*sep!=0){
				char *token = strtok(src, sep);
				do{
						if(flag){
							*(out+out_idx) = (char *)malloc(strlen(token)+1);
						}
						strcpy(*(out+out_idx), token);
						out_idx++;
						token = strtok(NULL, sep);
				}while(token!=NULL);
				return out_idx;
		}
		int tmp_len;
		// if sep==0
		for(; src_idx<len; src_idx++){
				if(*(src+src_idx)==0){
						continue;
				}
				tmp_len = strlen(src+src_idx);
				//printf("try to malloc\n");
				if(flag){
						*(out+out_idx) = (char *)malloc(tmp_len+1);
				}
				//printf("try to strcpy\n");
				strcpy(*(out+out_idx), src+src_idx);
				src_idx+=tmp_len;
				out_idx++;
		}
		return out_idx;
}
