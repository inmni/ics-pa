#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define KEYDOWN_MASK 0x8000
#define KEYCODE_MASK 0x7FFF
void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
	uint32_t scancode = *(volatile uint32_t *)KBD_ADDR;
  kbd->keydown = scancode & KEYDOWN_MASK ? true:false;
  kbd->keycode = scancode & KEYCODE_MASK;
}
