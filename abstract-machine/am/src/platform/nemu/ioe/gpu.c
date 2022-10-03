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
static int w = 400;
static int h = 300;
void __am_gpu_init() {
				int i;
				w = *(volatile uint16_t *)VGACTL_ADDR;
				h = *(volatile uint16_t *)(VGACTL_ADDR+2);
				uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
				for(i = 0; i < w * h; i ++) fb[i] = i;
				outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = *(volatile uint16_t *)VGACTL_ADDR, .height = *(volatile uint16_t *)(VGACTL_ADDR+2),
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
/*	int row,col;
	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	uint32_t *pixels = ctl->pixels;
	for(row = 0; row < ctl->h; row++){
			for(col = 0; col < ctl->w; col++){
					*(fb+SCREEN_W*(row+ctl->y)+col+ctl->x)=*(pixels+row*ctl->w+col);
			}
	}*/
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
