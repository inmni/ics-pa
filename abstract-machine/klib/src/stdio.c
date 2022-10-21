#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
typedef struct{
		char *str_begin;
		char *str_end;
		uint32_t count;
} buf;
typedef void (* op)(char, void *);
// func deals a character into the out.
void format(op func, void *out,const char *fmt, va_list ap);
static inline uint64_t get_num(va_list* ap, int l_count);
static inline void print_num(uint64_t num, int base, int l_count, int unsigned_flag, int width, void *out, op func);

void output(char ch, int* count){
		putch(ch);
		(*count)++;
}
void putToStr(char ch, buf* out){
		*(out->str_begin) = ch;
		out->str_begin++;
		out->count++;
}
void putToStrn(char ch, buf* out){
		if(out->str_begin >= out->str_end)return;

		*(out->str_begin) = ch;
		out->str_begin++;
		out->count++;
}
int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int count = 0;
  format((op)output, &count, fmt, ap);
	va_end(ap);
	return 1;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
		buf out_buf;
		out_buf.str_begin = out;
	 	out_buf.str_end = NULL;
		out_buf.count = 0;	
		format((op)putToStr, &out_buf, fmt, ap);
		return out_buf.count;
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsprintf(out, fmt, ap);
	va_end(ap);
	return 1;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
	va_start(ap, fmt);
	vsnprintf(out, n, fmt, ap);
	va_end(ap);
	return 1;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
	buf out_buf;
	out_buf.str_begin = out;
	out_buf.str_end = out + n - 1;
	out_buf.count = 0;
	format((op)putToStrn, &out_buf, fmt, ap);
	*(out_buf.str_begin) = 0;
	return out_buf.count;
}
void format(op func, void *out, const char *fmt, va_list ap) {
		const char *tmp;
		while(1) {
				// Common string
				while(*fmt != '%'){
						if(*fmt == 0)	return;
						func(*fmt, out);	fmt++;
				}
				fmt++;
				// Now *fmt='%', check the nearest
				int l_count = 0; int unsigned_flag = 0; int width = -1;
rematch:
				switch(*fmt++){
					// Check prefix
								case 'u':// unsigned prefix
												unsigned_flag = 1;							goto rematch;
								case 'l':// long prefix
												l_count++;											goto rematch;
								case '0':// reset width
												width = 0;											goto rematch;
								case '1'...'9':// width prefix
												while(*fmt >='0' && *fmt <='9'){
															width*=10; width+=*fmt-'0'; fmt++;
												}																goto rematch;
					// Check postfix(types)
								case 'c':// char
												 // 'char' will be promoted to 'int' through '...'
												func((char)va_arg(ap, int), out);			break;
								case 's':// string
												if(!(tmp = va_arg(ap, char *))){
															tmp = "(NULL STRING)";
												}
												while(*tmp){func(*tmp, out);tmp++;}		break;
								case 'd':// base-10 32bits number
											rnum:
												print_num(get_num(&ap, l_count), 10, l_count, unsigned_flag, width, out, func);															break;
								case 'x':// base-16 number
												print_num(get_num(&ap, l_count), 16, l_count, unsigned_flag, width, out, func);															break;
								case 'p':// pointer
												print_num((uint64_t)(intptr_t)va_arg(ap, void *), 16, l_count, unsigned_flag, width, out, func);						break;
					// Check special
								case '%':// Skip
												func('%', out);												break;
								default: // Unrecognized
												if( unsigned_flag || l_count )		goto rnum;
												func('%', out);
												while(*fmt-- != '%'){}; fmt+=2;				break;
				}
		}
}
const char* hex_num = "0123456789abcdef";
static inline void print_num(uint64_t num, int base, int l_count, int unsigned_flag, int width, void *out, op func) {
		//putstr("\nStart print num\n");
		if( base==16 ){ func('0',out); func('x',out);}
		else if( (!unsigned_flag) && base==10){
				switch(l_count) {
						case 0:	if((int32_t)num < 0){ func('-', out); num &= 0x7FFFFFFF; }																											break;
						case 1: if((long)num < 0){ func('-', out); num &= ~(-1l); }																															break;
						default:if((int64_t)num < 0){ func('-', out); num &= ~(-1ll); }																													break;
		
				}
		}
		putstr("\nStart get max width\n");
		// Now only need to deal with unsigned num.
		register uint64_t div_num = num;	int n = 0; uint64_t k;
		switch(base){
				case 2:	while(div_num){div_num>>=1;n++;}							break;
				case 8:	while(div_num){div_num>>=3;n++;}							break;
				case 10:while(div_num){div_num/=10;n++;}							break;
				case 16:while(div_num){div_num>>=4;n++;}							break;
		}
		while(width-- > n) func('0', out);
		putstr("\nStart get div num\n");
		div_num = 1; width = n-1;
		switch(base){
				case 2:	while(width){div_num<<=1;width--;}						break;
				case 8:	while(width){div_num<<=3;width--;}						break;
				case 10:while(width){div_num=(div_num<<3)+(div_num<<1);width--;}
																															break;
				case 16:while(width){div_num<<=4;width--;}						break;
		}
		putstr("\nStart output num\n");
		switch(base){
				case 2: // Optimize the branch
								while(n--){
												if(num<div_num)func('0',out);
												else{num-=div_num;func('1',out);}
								}																							break;
				case 8:
								while(n--){
												if(num<div_num)func('0',out);
												else{k = num/div_num;num-=k*div_num;func((char)k-'0',out);}
												div_num>>=3;
								}																							break;
				case 10:
								while(n--){
												if(num<div_num)func('0',out);
												else{k = num/div_num;num-=k*div_num;func((char)k-'0',out);}
												div_num/=10;
								}																							break;
				case 16:
								while(n--){
												if(num<div_num)func('0',out);
												else{k = num/div_num;num-=k*div_num;func(hex_num[k],out);}
												div_num>>=4;
								}																							break;
		}
}
static inline uint64_t get_num(va_list *ap, int l_count) {
		switch(l_count){
				case 0:		return (uint64_t)(unsigned int)va_arg(*ap, int);
				case 1:		return (uint64_t)(unsigned long)va_arg(*ap, long);
				default:	return (uint64_t)(unsigned long long)va_arg(*ap, long long);
		}
}
#endif
