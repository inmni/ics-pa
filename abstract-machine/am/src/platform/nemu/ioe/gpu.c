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
				int i;
				int w = *(volatile uint16_t *)(VGACTL_ADDR+2);
				int h = *(volatile uint16_t *)VGACTL_ADDR;
				uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
				for(i = 0; i < w * h; i ++) fb[i] = i;
				outl(SYNC_ADDR, 1);
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
	uint32_t *pixels = ctl->pixels;
	uint32_t *tmp1 = fb, *tmp2 = pixels;
	row = ctl->h;
	while(row--){
			col = ctl->w;
			tmp1 = fb;
			tmp2 = pixels;
			while(col--)*tmp1++ = *tmp2++;
			fb+=SCREEN_W; pixels+=ctl->w;
	}
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
