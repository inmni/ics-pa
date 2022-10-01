#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define KEYDOWN_MASK 0x8000
#define KEYCODE_MASK 0x7FFF
void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keydown = (bool)(((*(volatile uint32_t *)KBD_ADDR)&KEYDOWN_MASK)>>15);
  kbd->keycode = (*(volatile uint32_t *)KBD_ADDR) & KEYCODE_MASK;
	int tmp = *(volatile uint32_t *)KBD_ADDR;
	if(tmp)
	printf("keydown:%d,keycode:%d\n",kbd->keydown,kbd->keycode);
}
