#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keydown = (bool)((*(volatile uint8_t *)KBD_ADDR)>>7);
  kbd->keycode = ((*(volatile uint32_t *)KBD_ADDR)<<1)>>1;
}
