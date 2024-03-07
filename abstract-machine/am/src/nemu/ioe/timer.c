#include <am.h>
#include <nemu.h>

uint64_t start_time;

void __am_timer_init() {
  uint64_t low = inl(RTC_ADDR);
  uint64_t high = inl(RTC_ADDR + 4);
  start_time = high * 1000000  + low;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t low = inl(RTC_ADDR);
  uint64_t high = inl(RTC_ADDR + 4);
  uptime->us = high * 1000000 + low - start_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
