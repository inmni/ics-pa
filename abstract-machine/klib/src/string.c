#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t ans = 0;
	while(*s++)ans++;
	return ans;
}

char *strcpy(char *dst, const char *src) {
	char *head = dst;
	while((*dst++=*src++));
	*dst=0;
	return head;
}

char *strncpy(char *dst, const char *src, size_t n) {
	size_t i;
	for(i = 0; i < n && !(*(src+i)); i++)*(dst+i) = *(src+i);
	for(; i<n;i++)*(dst+i) = 0;
	return dst;
}

char *strcat(char *dst, const char *src) {
	char *head = dst;
	while(*dst)dst++;
	while((*dst++=*src++));
	return head;
}

int strcmp(const char *s1, const char *s2) {
	while(*s1==*s2){
			if(*s1==0)return 0;
			s1++;s2++;
	}
	return *(unsigned char *)s1-*(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while(n-->0){
			if(*s1==*s2){
					if(*s1==0)return 0;
					s1++;s2++;continue;
			}
			return *s1<*s2?-1:1;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
//	printf("Set %d bytes\n",(uint32_t)n);
	char* pb = (char*)s;
	while(n-->0)*pb++=c;
	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *bs_dst = (char *)dst;
	char *bs_src = (char *)src;
	if(bs_dst>bs_src && (bs_src+n>bs_dst)){
		bs_dst += n-1;
		bs_src += n-1;
		while(n--){
				*bs_dst-- = *bs_src--;
		}
	}
	else{
		while(n--){
				*bs_dst++ = *bs_src++;
		}
	}
	return dst;
}
typedef char 				__1UNIT;
typedef uint32_t		__4UNIT;
typedef uint64_t		__8UNIT;
typedef struct{
uint64_t u1;uint64_t u2;uint64_t u3;uint64_t u4;
}										__32UNIT;
#define fcpy(len)	do{	\
		while(n >= len){	\
				*(__##len##UNIT *)out	= *(__##len##UNIT *)in;	\
				out += len; in += len; n -= len;	\
		}	\
}	while(0)	
void *memcpy(void *out, const void *in, size_t n) {
	void *tmp = out;
	printf("copy %d bytes\n", n);
	fcpy(32);
	fcpy(8);
	fcpy(4);
	fcpy(1);
	return tmp;
	char *bs_dst = (char *)out;
	char *bs_src = (char *)in;
	while(n--){
		*bs_dst++ = *bs_src++;
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	size_t i = 0;
	for(; i<n; i++){
		if(*(unsigned char *)(s1+i)==*(unsigned char *)(s2+i))continue;
		return *(unsigned char *)(s1+i) <*(unsigned char *)(s2+i) ? -1:1;
	}
	return 0;
}
char *itoa(int num, char *str, int base){
	int n = num<0?-num:num;
	int i=0;int tmp=0;
	while(n){
		tmp = n%base;
		if(tmp>=10){
			str[i++] = 55+tmp;
		}
		else{
			str[i++] = 48+tmp;
		}
		n/=base;
	}
	if(i==0)str[i++] = '0';
	if(num<0&&base==10)str[i++] = '-';
	str[i] = 0;
	tmp=0;i--;
	char t;
	while(tmp<i){
			t=str[tmp];str[tmp]=str[i];str[i]=t;
			tmp++;i--;
	}
	return str;
}
#endif
