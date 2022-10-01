#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define match(tmp, fmt, ap, count) ({\
memset(tmp,0,256);\
if(*fmt!='%'){\
		count = 1;\
		*tmp++=*fmt++;\
		*tmp=0;\
}\
else{\
	fmt++;\
	if(*fmt=='d'){\
			count = itoa(va_arg(ap, int), tmp, 10)-tmp;\
			fmt++;\
	}\
	else if(*fmt=='s'){\
			tmp = va_arg(ap, char *);\
			count = strlen(tmp);\
			tmp++;\
	}\
	else{\
			panic("No found format");return 0;\
	}\
}\
\
})
int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int move=0;
	char tmp[256]={0};
	while(*fmt){
		char *head = tmp;
		match(head, fmt, ap, move);
		strcpy(out, tmp);
		out+=move;
	continue;	
		if(*fmt!='%'){
			*out++ = *fmt++;
			continue;
		}
		fmt++;
		if(*fmt=='d'){
			int d = va_arg(ap, int);
			out = itoa(d, out, 10);
			fmt++;
		}
		else if(*fmt=='s'){
			char *str = va_arg(ap, char *);
			size_t len = strlen(str);
			strcpy(out,str);
			out+=len;
			fmt++;
		}
		else{panic("No found format");return 0;}
	}
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
