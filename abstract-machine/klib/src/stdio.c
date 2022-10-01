#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
typedef int (*op2str)(char *str1, char *str2);
typedef int (*op2ch)(char *str, char ch);
static int ch_copy(char *dst, char ch){
		*(dst+1)=ch;
		putch(ch);
		return 1;
}
static int ch_put(char *no_str, char ch){
		putch(ch);
		return 0;
}
static int str_copy(char *dst, char *src){
		strcpy(dst,src);
		putstr(src);
		return strlen(src);
}
static int str_put(char *no_str, char *str){
		putstr(str);
		return 0;
}
int format(char *tmp, op2str op1, op2ch op2, const char *fmt, va_list ap){
		char buf[16] = {0};
		while(*fmt){
				switch(*fmt){
						case '%':fmt++;
						case 'd':{
									itoa(va_arg(ap, int), buf, 10);
									tmp+=op1(tmp, buf);
									fmt++;
									break;
						}
						case 's':{
									char *arg = va_arg(ap, char *);
									putstr("aaaaaaaaaaaaa\n");
									tmp+=op1(tmp, arg);
									fmt++;
									break;
						}
						default: op2(tmp, *fmt); tmp++; fmt++;
				}
		}
		return 1;
}
int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
  format(NULL, str_put, ch_put, fmt, &ap);
	va_end(ap);
	return 1;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
		return format(out, str_copy, ch_copy, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsprintf(out, fmt, ap);
	va_end(ap);
	return 1;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
