#include <common.h>

#ifdef HAS_IOE

#include <device/map.h>
#include <SDL2/SDL.h>

#define AUDIO_PORT 0x200 // Note that this is not the standard
#define AUDIO_MMIO 0xa1000200
#define STREAM_BUF 0xa0800000
#define STREAM_BUF_MAX_SIZE 65536

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static SDL_AudioSpec s = {};

static inline void audio_play(void *userdata, uint8_t *stream, int len) {
	int nread = len;
  if (audio_base[reg_count] < len) 
		nread = audio_base[reg_count];
  memcpy(stream, sbuf, nread);
  audio_base[reg_count] -= nread;
  if (len > nread) memset(stream + nread, 0, len - nread);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
	assert(len == 4);
	if(!is_write)
		return;
  switch (offset) {
    case 0:  s.freq = audio_base[reg_freq]; break;
    case 4:  s.channels = audio_base[reg_channels]; break;
    case 8:  s.samples = audio_base[reg_samples]; break;
    default: break;
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (void *)new_space(space_size);
  add_pio_map("audio", AUDIO_PORT, (void *)audio_base, space_size, audio_io_handler);
  add_mmio_map("audio", AUDIO_MMIO, (void *)audio_base, space_size, audio_io_handler);

  sbuf = (void *)new_space(STREAM_BUF_MAX_SIZE);
  add_mmio_map("audio-sbuf", STREAM_BUF, (void *)sbuf, STREAM_BUF_MAX_SIZE, NULL);
	SDL_AudioSpec s = {};
	s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用16位有符号数来表示
	s.userdata = NULL;        // 不使用
	s.callback = audio_play;
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_OpenAudio(&s, NULL);
	SDL_PauseAudio(0);
}
#endif	/* HAS_IOE */
