#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	if(s==NULL)return 0;
	size_t ans = 0;
	while(*s){
		ans++;
		s++;
	}
	return ans;
}

char *strcpy(char *dst, const char *src) {
	if(src==NULL||dst==NULL)return dst;
	char *head = dst;
	while((*dst++=*src++));
	return head;
}

char *strncpy(char *dst, const char *src, size_t n) {
	if(src==NULL||dst==NULL)return dst;
	size_t i;
	for(i = 0; i < n && !src[i]; i++){
		*(dst+i) = *(src+i);
	}
	for(; i<n;i++){
		*(dst+i) = 0;
	}
	return dst;
}

char *strcat(char *dst, const char *src) {
	if(src==NULL||dst==NULL)return dst;
	char *tmp = dst+strlen(dst);
	while(*src){
		*tmp = *src;
		tmp++;src++;
	}
	*tmp = 0;
	return dst;
}

int strcmp(const char *s1, const char *s2) {
	if(s1==NULL)return s2?-1:0;
	if(s2==NULL)return 1;
	int ans = 0;
	while(*s2 && !(ans = *(unsigned char*)s1++ - *(unsigned char*)s2++));
	//if *s1 is zero, then ans < 0, break
	//if *s2 is zero, break, last ans >=0, if last ans is not zero, break;
	if(ans) return ans<0?-1:1;
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	size_t i = 0;
	for(; i<n; i++){
		if(*(s1+i)==*(s2+i))	continue;
		return *(s1+i) < *(s2+i) ? -1:1;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	if(s==NULL)return s;
	char* pb = (char*)s;
	while(n-->0){
		*pb = c; pb++;
	}
	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
	if(s1==NULL)return s2?-1:0;
	if(s2==NULL)return 1;
	size_t i = 0;
	for(; i<n; i++){
		if(*(char *)(s1+i)==*(char *)(s2+i))continue;
		return *(char *)(s1+i) <*(char *)(s2+i) ? -1:1;
	}
	return 0;
}

char *itoa(int num, char *str, int base){
	if(num==0){
		*str++='0';
		*str = 0;
		return str;
	}
	char tmp[10] = {0};
	int i=0;
	int tmp_num = num % 10;
	int flag = num<0?1:0;
	while(num>0){
		tmp[i++] = tmp_num+'0';
		num = num/10;
		tmp_num = num%10;
	}
	i--;
	if(flag){*str++='-';}
	while(i>=0){
		*str++=tmp[i];
		i--;
	}
	*str=0;
	return str;
}
#endif
