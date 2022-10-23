#include <am.h>
#include <nemu.h>

#ifdef CONFIG_VGA_SIZE_800x600
	#define SCREEN_W 800
	#define SCREEN_H 600
#else
	#define SCREEN_W 400
	#define SCREEN_H 300
#endif

#define SYNC_ADDR (VGACTL_ADDR + 4)
void __am_gpu_init() {
				/*
				int i;
				int w = *(volatile uint16_t *)(VGACTL_ADDR+2);
				int h = *(volatile uint16_t *)VGACTL_ADDR;
				uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
				for(i = 0; i < w * h; i ++) fb[i] = i;
				outl(SYNC_ADDR, 1);
				*/
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = *(volatile uint16_t *)(VGACTL_ADDR+2), .height = *(volatile uint16_t *)VGACTL_ADDR,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
	int row,col;
	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR+SCREEN_W*ctl->y+ctl->x;
	uint32_t *pixels = (uint32_t *)ctl->pixels;
	uint64_t *tmp1, *tmp2;
	row = ctl->h;
	if(ctl->w & 1){
		while(row--){
			col = ctl->w>>1;
			tmp1 = (uint64_t *)(fb+1); tmp2 = (uint64_t *)(pixels+1);
			*fb = *pixels;
			while(col--)*tmp1++ = *tmp2++;
			fb+=SCREEN_W; pixels+=ctl->w;
		}
	}
	else{
		while(row--){
			col = ctl->w>>1;
			tmp1 = (uint64_t *)fb; tmp2 = (uint64_t *)pixels;
			while(col--)*tmp1++ = *tmp2++;
			fb+=SCREEN_W; pixels+=ctl->w;
		}
	}
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

typedef struct {
	uint64_t u1; uint64_t u2; uint64_t u3; uint64_t u4;
}								 			__32UNIT;
typedef uint64_t 			__8UNIT;
typedef uint32_t 			__4UNIT;
typedef char					__1UNIT;
#include <stdio.h>
#define gpu_memcpy(len)	do{	\
		while(mcpy->size >= len){	\
				*((__##len##UNIT *)dest) = *((__##len##UNIT *)src);	\
				dest+=len;	src+=len;	mcpy->size-=len;	\
		}	\
}	while(0)
void __am_gpu_memcpy(AM_GPU_MEMCPY_T *mcpy){
		// This will modify the GPU_MEMCPY input.
		uintptr_t dest = FB_ADDR;
		uintptr_t src = (uintptr_t)(mcpy->src);
		//gpu_memcpy(32);
		//gpu_memcpy(8);
		//gpu_memcpy(4);
		gpu_memcpy(1);
}
void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
