#include <am.h>
#include <nemu.h>
#include <stdio.h>
static uint64_t boot_time = 0;

static uint64_t read_time(){
		uint64_t hi = ((uint64_t)(*(volatile uint32_t *)(RTC_ADDR+4)))<<32;
		uint64_t lo = *(volatile uint32_t *)RTC_ADDR;
		return hi+lo;
}

void __am_timer_init() {
	boot_time = read_time();
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	uptime->us = read_time() - boot_time;
	printf("time:%d, %d\n", (int)(uptime->us>>32), (int)uptime->us);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
