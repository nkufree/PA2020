#include <am.h>
#include <nemu.h>
#include <klib.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t screen_width = 400, screen_height = 300;


void __am_gpu_init() {
  // int i;
  // int w = W;
  // outw(VGACTL_ADDR,w);
  // int h = H;
  // outw(VGACTL_ADDR,h);
  // uint32_t *fb = (uint32_t*)(uintptr_t)FB_ADDR;
  // for(i = 0;i < w*h;i++) fb[i] = 0;
  // outl(SYNC_ADDR,1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = inw(VGACTL_ADDR+2), .height = inw(VGACTL_ADDR),
    .vmemsz = 0
  };
  screen_width = cfg->width;
  screen_height = cfg->height;
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t (*fb)[screen_width] = (uint32_t (*)[screen_width])(uintptr_t)FB_ADDR;
  uint32_t (*pixels)[ctl->w] = ctl->pixels;
  int rw = ctl->x + ctl->w > screen_width ? screen_width : ctl->x + ctl->w;
  int bh = ctl->y + ctl->h > screen_height ? screen_height : ctl->y + ctl->h;
  for(int y = ctl->y; y < bh; y++)
  {
    for(int x = ctl->x; x < rw; x++)
    {
      fb[y][x] = pixels[y - ctl->y][x - ctl->x];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
