#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define W 400
#define H 300

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = inw(VGACTL_ADDR+2), .height = inw(VGACTL_ADDR),
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *tmp = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t (*fb)[W] = (uint32_t (*)[W])(uintptr_t)tmp;
  uint32_t (*pixels)[W] = ctl->pixels;
  int rw = ctl->x + ctl->w > W ? W : ctl->x + ctl->w;
  int bh = ctl->y + ctl->h > H ? H : ctl->y + ctl->h;
  for(int x = ctl->x; x < rw; x++)
  {
    for(int y = ctl->y; y < bh; y++)
    {
      fb[y][x] = pixels[y - ctl->h][x - ctl->x];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
