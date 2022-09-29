#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	while(*fmt){
		if(*fmt!='%'){
			*out++ = *fmt++;
			continue;
		}
		fmt++;
		if(*fmt=='d'){
			int d = va_arg(ap, int);
			itoa(d, out, 10);
			while(*out++);
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
