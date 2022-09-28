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
		}
		else if(*fmt=='s'){
			char *str = va_arg(ap, char *);
			int len = strlen(str);
			strcpy(out,str);
			out+=len;
		}
		else{panic("No found format");}
	}
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
