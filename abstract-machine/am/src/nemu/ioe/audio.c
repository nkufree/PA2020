#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static int buf_size;

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  buf_size = cfg->bufsize;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  printf("freq: %d, channels: %d, samples:%d\n", ctrl->freq, ctrl->channels, ctrl->samples);
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int len = ctl->buf.end - ctl->buf.start;
  while(len > 0)
  {
    int nwrite = len > buf_size ? buf_size : len;
    while (inl(AUDIO_COUNT_ADDR) != 0);
    memcpy((uint32_t*)(uintptr_t)AUDIO_SBUF_ADDR, ctl->buf.start, nwrite);
    outl(AUDIO_COUNT_ADDR, nwrite);
    len -= nwrite;
  }
  
}
