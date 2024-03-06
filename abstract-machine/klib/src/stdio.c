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

static void itora(int num, char* s, int* len)
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
    tmp = num % 10;
    num = num / 10;
    s[curr_len] = tmp + '0';
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
            itora(va_arg(fmtp->ap, int), fmtp->helpd, &fmtp->helplen);
            break;
          case 's':
            fmtp->fmt++;
            fmtp->state = FMT_S;
            fmtp->helplen = -1;
            fmtp->helps = va_arg(fmtp->ap, char*);
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

// int vsprintf(char *out, const char *fmt, va_list ap) {
//   int d;
//   char* s;
//   int out_index = 0;
//   while (*fmt)
//   {
//     if(*fmt != '%')
//     {
//       out[out_index] = *fmt;
//       out_index++;
//     }
//     else
//     {
//       fmt++;
//       switch(*fmt)
//       {
//         case 'c':
//           out[out_index] = va_arg(ap, int);
//           out_index++;
//           break;
//         case 'd':
//           d = va_arg(ap, int);
//           if(d < 0)
//           {
//             out[out_index] = '-';
//             out_index++;
//             d = -d;
//           }
//           if(d == 0)
//           {
//             out[out_index] = '0';
//             out_index++;
//             break;
//           }
//           int len = 0;
//           int dc = d;
//           while (dc != 0)
//           {
//               len++;
//               dc = dc / 10;
//           }
//           len += out_index - 1;
//           int tmp;
//           while (d != 0)
//           {
//             tmp = d % 10;
//             d = d / 10;
//             out[len] = tmp + '0';
//             len--;
//             out_index++;
//           }
//           break;
//         case 's':
//           s = va_arg(ap, char*);
//           d = strlen(s);
//           strncpy(out + out_index, s, d);
//           out_index += d;
//           break;
//         default:
//           assert(0);
//       }
//     }
//     fmt++;
//   }
//   out[out_index] = '\0';
//   return 0;
// }

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
  return 0;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return 0;
}

#endif
