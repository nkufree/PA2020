#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for(int i = 0; i < len; i++)
  {
    putch(((char*)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;
  sprintf(buf, "%s %s\n\0", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
//   printf("%s %s\n",ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
  return strlen((char*)buf);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  sprintf(buf, "WIDTH:%d\nHEIGHT:%d\n", io_read(AM_GPU_CONFIG).width, io_read(AM_GPU_CONFIG).height);
  return strlen((char*)buf);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int width = io_read(AM_GPU_CONFIG).width;
  int x = (offset >> 2) % width;
  int y = (offset >> 2) / width;
  int w = len >> 2;
  printf("x: %d, y: %d, w: %d\n", x, y, w);
  io_write(AM_GPU_FBDRAW, x, y, (uint32_t*)buf, w, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
