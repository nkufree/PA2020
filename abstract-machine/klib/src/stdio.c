#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

enum {FMT_USUAL, FMT_S, FMT_D};

typedef struct {
  const char* fmt;
  va_list ap;
  int state;
  bool end;
  char helpd[20];
  int helplen;
  char* helps;
} fmt_parser;

static void itora(int num, char* s, int* len, int base)
{
  bool minus = num < 0;
  if(minus)
    num = -num;
  if(num == 0)
  {
    s[0] = '0';
    s[1] = '\0';
    *len = 1;
    return;
  }
  int curr_len = 0;
  int tmp;
  while (num != 0)
  {
    tmp = num % base;
    num = num / base;
    if(base == 10)
      s[curr_len] = tmp + '0';
    else if(base == 16)
    {
      if(tmp < 10)
        s[curr_len] = tmp + '0';
      else
        s[curr_len] = tmp - 10 + 'a';
    }
    curr_len++;
  }
  if(minus)
  {
    s[curr_len] = '-';
    curr_len++;
  }
  s[curr_len] = '\0';
  *len = curr_len;
}

static char get_char(fmt_parser* fmtp)
{
  if(fmtp->end)
    return 0;
repeat:
  switch(fmtp->state)
  {
    case FMT_USUAL:
      ;char ch = *(fmtp->fmt);
      if(ch == '\0')
      {
        fmtp->end = true;
        return 0;
      }
      else if(ch == '%')
      {
        fmtp->fmt++;
        switch(*(fmtp->fmt))
        {
          case 'c': 
            fmtp->fmt++;
            return va_arg(fmtp->ap, int);
          case 'd':
            fmtp->fmt++;
            fmtp->state = FMT_D;
            itora(va_arg(fmtp->ap, int), fmtp->helpd, &fmtp->helplen, 10);
            break;
          case 's':
            fmtp->fmt++;
            fmtp->state = FMT_S;
            fmtp->helplen = -1;
            fmtp->helps = va_arg(fmtp->ap, char*);
            break;
          case 'x':
            fmtp->fmt++;
            fmtp->state = FMT_D;
            itora(va_arg(fmtp->ap, int), fmtp->helpd, &fmtp->helplen, 16);
            break;
          case 'p':
            fmtp->fmt++;
            fmtp->state = FMT_D;
            itora(va_arg(fmtp->ap, int), fmtp->helpd, &fmtp->helplen, 16);
            while(fmtp->helplen < sizeof(uintptr_t) * 2)
            {
              fmtp->helpd[fmtp->helplen] = '0';
              fmtp->helplen++;
            }
            fmtp->helpd[fmtp->helplen] = 'x';
            fmtp->helpd[fmtp->helplen + 1] = '0';
            fmtp->helplen+=2;
            break;
        }
        goto repeat;
      }
      else
      {
        fmtp->fmt++;
        return ch;
      }
      break;
    case FMT_D:
      fmtp->helplen --;
      if(fmtp->helplen == 0)
        fmtp->state = FMT_USUAL;
      return fmtp->helpd[fmtp->helplen];
    case FMT_S:
      fmtp->helplen++;
      if(fmtp->helps[fmtp->helplen] == '\0')
        fmtp->state = FMT_USUAL;
      return fmtp->helps[fmtp->helplen];
  }
  return 0;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fmt_parser fmtp = {fmt, ap, false};
  while (!fmtp.end)
  {
    char ch = get_char(&fmtp);
    putch(ch);
  }
  va_end(ap);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  fmt_parser fmtp = {fmt, ap, false};
  while (!fmtp.end)
  {
    *out = get_char(&fmtp);
    out++;
  }
  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  fmt_parser fmtp = {fmt, ap, false};
  int len = 0;
  while (!fmtp.end && len < n)
  {
    *out = get_char(&fmtp);
    out++;
    len++;
  }
  return 0;
}

#endif
