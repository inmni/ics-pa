#include <am.h>
#include <nemu.h>
static uint64_t boot_time = 0;
// No include macro.h, so copy the BITS from it.
#define BITS(x, hi, lo) (((x) >> (lo))&((1ull << ((hi) - (lo) + 1)) - 1))

#define TIME_MMIO 0xa0000048	// The time mmio of client

static uint64_t read_time(){
		return *(volatile uint64_t *)TIME_MMIO;
}

void __am_timer_init() {
	boot_time = read_time();
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	uptime->us = read_time() - boot_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
