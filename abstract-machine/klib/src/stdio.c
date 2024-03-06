#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int d;
  char* s;
  va_start(ap, fmt);
  int out_index = 0;
  while (*fmt)
  {
    if(*fmt != '%')
    {
      out[out_index] = *fmt;
      out_index++;
    }
    else
    {
      fmt++;
      switch(*fmt)
      {
        case 'c':
          out[out_index] = va_arg(ap, int);
          out_index++;
          break;
        case 'd':
          d = va_arg(ap, int);
          if(d < 0)
          {
            out[out_index] = '-';
            out_index++;
            d = -d;
          }
          if(d == 0)
          {
            out[out_index] = '0';
            out_index++;
            break;
          }
          while (d != 0)
          {
            int tmp = d % 10;
            d = d / 10;
            out[out_index] = tmp + '0';
            out_index++;
          }
          break;
        case 's':
          s = va_arg(ap, char*);
          d = strlen(s);
          strncpy(out + out_index, s, d);
          out_index += d;
          break;
        default:
          assert(0);
      }
    }
    fmt++;
  }
  
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return 0;
}

#endif
