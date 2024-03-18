#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t get_ramdisk_size();
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, 
// FD_NWM_EVT, FD_NWM_CTL, FD_NWM_FB, 
FD_EVENT, FD_FB, FD_INFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  // [FD_NWM_EVT]= {"nwmevt", 0, 0, nwm_events_read, nwm_events_write},
  // [FD_NWM_CTL]= {"nwmctl", 0, 0, invalid_read, nwm_ctl_write},
  // [FD_NWM_FB] = {"nwmfb", 0, 0, invalid_write, nwm_fb_write},
  [FD_EVENT]  = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_INFO]   = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  file_table[FD_FB].size = w * h * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  int fd;
  int file_num = sizeof(file_table) / sizeof(Finfo);
  for(fd = 0; fd < file_num; fd++)
  {
    if(strcmp(pathname, file_table[fd].name) == 0)
    {
      file_table[fd].open_offset = 0;
      return fd;
    }
  }
  printf("no such file: %s\n", pathname);
  assert(0);
}

size_t fs_read(int fd, void *buf, size_t len) {
  Finfo* file = &file_table[fd];
  // printf("read openoff: %d, len: %d\n", file->open_offset, len);
  size_t free_len = file->size - file->open_offset;
  size_t read_len = free_len < len ? free_len : len;
  if(file->read != NULL) {
    read_len = len;
    read_len = file->read(buf, file->disk_offset + file->open_offset, read_len);
  }
  else {
    read_len = ramdisk_read(buf, file->disk_offset + file->open_offset, read_len);
  }
  file->open_offset += read_len;
  return read_len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  Finfo* file = &file_table[fd];
  // printf("write openoff: %d, len: %d\n", file->open_offset, len);
  size_t free_len = file->size - file->open_offset;
  size_t write_len = free_len < len ? free_len : len;
  if(file->write != NULL) {
    write_len = len;
    write_len = file->write(buf, file->disk_offset + file->open_offset, write_len);
  }
  else {
    write_len = ramdisk_write(buf, file->disk_offset + file->open_offset, write_len);
  }
  file->open_offset += write_len;
  return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo* file = &file_table[fd];
  // printf("fd: %d, openoff: %d, off: %d, whe: %d\n", fd, file->open_offset, offset, whence);
  switch (whence)
  {
  case SEEK_SET:
    assert(offset >= 0 && offset <= file->size);
    file->open_offset = offset;
    break;
  case SEEK_CUR:
    assert((file->open_offset + offset) >=0 && (file->open_offset + offset) <= file->size);
    file->open_offset += offset;
    break;
  case SEEK_END:
    assert((file->open_offset - offset) >=0 && (file->open_offset - offset) <= file->size);
    file->open_offset = file->size - offset;
    break;
  default:
    break;
  }
  return file->open_offset;
}

int fs_close(int fd) {
  return 0;
}