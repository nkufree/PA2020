#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i = 0;
  while (s[i] != '\0')
  {
    i++;
  }
  return i;
}

char *strcpy(char* dst,const char* src) {
  int i = 0;
  while(src[i] != '\0')
  {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  if(n == 0)
    return dst;
  int i = 0;
  while (src[i] != '\0' || i < n)
  {
    dst[i] = src[i];
    i++;
  }
  while (i < n)
  {
    dst[i] = '\0';
  }
  return dst;
}

char* strcat(char* dst, const char* src) {
  int len = strlen(dst);
  int i = 0;
  while(src[i] != '\0')
  {
    dst[i + len] = src[i];
    i++;
  }
  dst[i + len] = '\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  int i = 0;
  while (s1[i] != '\0' || s2[i] != '\0')
  {
    if(s1[i] != s2[i])
      return s1[i] - s2[i];
    i++;
  }
  return s1[i] - s2[i];
}

int strncmp(const char* s1, const char* s2, size_t n) {
  if(n == 0)
    return 0;
  int i = 0;
  while ((s1[i] != '\0' || s2[i] != '\0') && i < n)
  {
    if(s1[i] != s2[i])
      return s1[i] - s2[i];
    i++;
  }
  return s1[i] - s2[i];
}

void* memset(void* v,int c,size_t n) {
  char* tmp = v;
  for(int i = 0; i < n; i++)
  {
    *tmp = c;
    tmp++;
  }
  return v;
}

void* memmove(void* dst,const void* src,size_t n) {
  void* ret = dst;
  for(int i = 0; i < n; i++)
  {
    *(char*)dst = *(char*)src;
    dst++;
    src++;
  }
  return ret;
}

void* memcpy(void* out, const void* in, size_t n) {
  if(in == NULL || out == NULL)
    return NULL;
  void* ret = out;
  if(out < in || out > in + n) 
  {
    for(int i = 0; i < n; i++)
    {
      *(char*)out = *(char*)in;
      out = (char*)out + 1;
      in = (char*)in + 1;
    }
  }
  else
  {
    out = (char*)out + n;
    in = (char*)in + n;
    for(int i = 0; i < n; i++)
    {
      *(char*)out = *(char*)in;
      out = (char*)out - 1;
      in = (char*)in - 1;
    }
  }
  return ret;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  for(int i = 0; i < n; i++)
  {
    if(*(char*)s1 != *(char*)s2)
      return *(char*)s1 - *(char*)s2;
    s1++;
    s2++;
  }
  return 0;
}

#endif
