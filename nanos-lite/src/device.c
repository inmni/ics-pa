#include <common.h>
#include <device.h>
#include <fs.h>
#define MULTIPROGRAM
#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,
static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
	MULTIPROGRAM_YIELD();
	int i;
		for(i=0; i<len; i++){
				putch(*((char *)buf+i));
		}
		return len;
}

void switch_prog(uint32_t id);
size_t events_read(void *buf, size_t offset, size_t len) {
		MULTIPROGRAM_YIELD();
		AM_INPUT_KEYBRD_T input = io_read(AM_INPUT_KEYBRD);
		if( input.keycode == AM_KEY_NONE ){
				return 0;
		}
		if( input.keydown ){
				switch(input.keycode){
					case AM_KEY_F1:	switch_prog(1);	break;
					case AM_KEY_F2: switch_prog(2); break;
					case AM_KEY_F3: switch_prog(3); break;
					default:												break;
				}
				return snprintf(buf, len, "kd %s %d", keyname[input.keycode], input.keycode);
		}
		return snprintf(buf, len, "ku %s %d", keyname[input.keycode],input.keycode);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  	AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
		return snprintf(buf, len, "WIDTH:%d\nHEIGHT:%d", gpu_config.width,gpu_config.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
		MULTIPROGRAM_YIELD();
		if(offset + len > fs_size(4)){
				len = fs_size(4) - offset;
		}
		//printf("%08x, %d, %08x\n", (uintptr_t)buf, offset, (uint32_t)len);	
		AM_GPU_MEMCPY_T gpu_memcpy;
		
		gpu_memcpy.dest = offset;
		gpu_memcpy.src = buf;
		gpu_memcpy.size = len;

		ioe_write(AM_GPU_MEMCPY, &gpu_memcpy);
		
		return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
