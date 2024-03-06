#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define W 400
#define H 300

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
  // uint32_t *fb = (uint32_t*)(uintptr_t)FB_ADDR;
  // int x = ctl->x, y = ctl->y, w = ctl->w,h = ctl->h;
  // uint32_t *pixels = ctl->pixels;
  // for(int i = 0;i < h;i++)
  //   for(int j = 0;j < w;j++)
  //   {
  //     if(y+i < H&&x+j <W)
  //     {
  //       fb[W*(y+i)+x+j] = pixels[w*i+j];
  //       //printf("%x\n",fb[W*(y+i)+x+j]);
  //     }
  //   }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
