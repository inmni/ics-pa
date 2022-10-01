#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keydown = (bool)((*(volatile uint8_t *)KBD_ADDR)>>7);
  kbd->keycode = ((*(volatile uint32_t *)KBD_ADDR)<<1)>>1;
	if(kbd->keydown||kbd->keycode)
	printf("down:%d code:%d\n",kbd->keydown,kbd->keycode);
}
