#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_off_x = 0, canvas_off_y = 0;
static int canvas_w = 0, canvas_h = 0;
static int fd_event = -1, fd_dispinfo = -1;
FILE* f_fb = NULL;

void get_dispinfo();
int open(const char *pathname, int flags, int mode);

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int ret = read(fd_event, buf, len);
  return ret;
}

void NDL_OpenCanvas(int *w, int *h) {
  get_dispinfo();
  // printf("screenw: %d, screenh: %d\n", screen_w, screen_h);
  if(*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  canvas_w = *w < screen_w ? *w : screen_w;
  canvas_h = *h < screen_h ? *h : screen_h;
  canvas_off_x = (canvas_w - *w) >> 1;
  canvas_off_y = (canvas_h - *h) >> 1;
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void get_dispinfo() {
  char tmp[40] = {};
  read(fd_dispinfo, tmp, sizeof(tmp));
  char* curr = tmp;
  int find_height = 0, find_width = 0;
  int* num = NULL;
  while (!(find_height && find_width)) {
    if(strncmp(curr, "HEIGHT", 6) == 0) {
      curr += 6;
      find_height = 1;
      num = &screen_h;
    }
    else if(strncmp(curr, "WIDTH", 5) == 0) {
      curr += 5;
      find_width = 1;
      num = &screen_w;
    }
    if(num == NULL) {
      curr++;
      continue;
    }
    while(*curr == ' ') curr++;
    assert(*curr == ':');
    curr++;
    *num = atoi(curr);
    num = NULL;
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int width = x + w < canvas_w ? w : canvas_w - x;
  int height = y + h < canvas_h ? y + h : canvas_h;
  for(int row = y; row < height; row++)
  {
    fseek(f_fb, (row * screen_w + x + canvas_off_x) << 2, SEEK_SET);
    fwrite(pixels + (row - y + canvas_off_y) * w, 1, width << 2, f_fb);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  fd_event = open("/dev/events", 0, 0);
  fd_dispinfo = open("/proc/dispinfo", 0, 0);
  f_fb = fopen("/dev/fb", "w");
  return 0;
}

void NDL_Quit() {
}
