#include <am.h>
#include <nemu.h>

uint64_t start_time;

void __am_timer_init() {
  uint64_t usec = inl(RTC_ADDR);
  uint64_t sec = inl(RTC_ADDR + 4);
  start_time = sec * 1000000  + usec;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t usec = inl(RTC_ADDR);
  uint64_t sec = inl(RTC_ADDR + 4);
  uptime->us = sec * 1000000 + usec - start_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
