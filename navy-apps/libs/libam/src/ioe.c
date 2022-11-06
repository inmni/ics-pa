#include <am.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <NDL.h>
static struct timeval tv;
static uint64_t boot_time = 0;
void __navy_timer_init(){
	gettimeofday(&tv, NULL);
	boot_time = tv.tv_sec * 1000000 + tv.tv_usec;
}
#define __KEYDOWN 1
#define __KEYUP	  0
void __navy_input_keybrd(AM_INPUT_KEYBRD_T *kbd){
	char buf[32];
	kbd->keydown = 0; kbd->keycode = 0;
	if(NDL_PollEvent(buf, 32)){
		// If no mouse event, the buf will be "kd/ku KEYNAME KEYCODE"
		char *key = strtok(buf, " ");
		strtok(NULL, " ");
		char *code= strtok(NULL, "");
		//assert(key[0]=='k' && code!=NULL);	// Maybe mouse event will be add to NDL
		switch(key[1]){
			case 'd': kbd->keydown = __KEYDOWN; break;
			case 'u': kbd->keydown = __KEYUP;   break;
			//default:	assert(0);// Maybe event write error ?
		}
		kbd->keycode = atoi(code);
	}
}
void __navy_timer_rtc(AM_TIMER_RTC_T *rtc){
	rtc->second = 0;
	rtc->minute = 0;
	rtc->hour   = 0;
	rtc->day    = 0;
	rtc->month  = 0;
	rtc->year   = 1900;
}
void __navy_timer_uptime(AM_TIMER_UPTIME_T *uptime){
	gettimeofday(&tv,NULL);
	uptime->us = tv.tv_sec * 1000000 + tv.tv_usec - boot_time;
}
void __navy_gpu_config(AM_GPU_CONFIG_T *cfg){
	AM_GPU_CONFIG_T tmp;
	if(cfg==NULL){
		cfg = &tmp;
	}
	cfg->present = true; cfg->has_accel = false; cfg->vmemsz = 0;
	
	int fd = open("/proc/dispinfo", 0);
	char buf[64];
	char *key1 = strtok(buf, ":");
	char *val1 = strtok(NULL, "\n");
	char *key2 = strtok(NULL, ":");
	char *val2 = strtok(NULL, "\n");
	assert(key1&&key2&&val1&&val2);
	//Maybe other information will be stored in.
	if(strcmp(key1, "WIDTH")==0){cfg->width = atoi(val1);}
	else if(strcmp(key1, "HEIGHT")==0){cfg->height = atoi(val1);}
	//else{assert(0);}

	if(strcmp(key2, "WIDTH")==0){cfg->width = atoi(val2);}
	else if(strcmp(key2, "HEIGHT")==0){cfg->height = atoi(val2);}
	//else{assert(0);}
	
}
void __navy_gpu_status(AM_GPU_STATUS_T *status){
	status->ready = true;
}
void __navy_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl){
	
  	AM_GPU_CONFIG_T cfg;
  	__navy_gpu_config(&cfg);
  	uint32_t width = cfg.width << 2;
  	
  	int fd = open("/dev/fb", 0);
  	lseek(fd, ctl->y * width + ((ctl->x)<<2), SEEK_SET);
  	
  	int i;
  	uintptr_t pixels = (uintptr_t)(ctl->pixels);
  	for (i = 0; i < ctl->h; ++ i) {
    		write(fd, (void *)pixels, ctl->w<<2);
    		lseek(fd, width - (ctl->w<<2), SEEK_CUR);
    		pixels += ctl->w << 2;
  	}
}

static void __navy_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __navy_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __navy_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }
static void __navy_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __navy_timer_config,
  [AM_TIMER_RTC   ] = __navy_timer_rtc,
  [AM_TIMER_UPTIME] = __navy_timer_uptime,
  [AM_INPUT_CONFIG] = __navy_input_config,
  [AM_INPUT_KEYBRD] = __navy_input_keybrd,
  [AM_GPU_CONFIG  ] = __navy_gpu_config,
  [AM_GPU_FBDRAW  ] = __navy_gpu_fbdraw,
  [AM_GPU_STATUS  ] = __navy_gpu_status,
  [AM_UART_CONFIG ] = __navy_uart_config,
};

static void fail(void *buf) { puts("access nonexist register"); assert(0);}

bool ioe_init() {
  for (int i = 0; i < 128; i++)
    if (!lut[i]) lut[i] = fail;
  __navy_timer_init();
  return true;
}

void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
