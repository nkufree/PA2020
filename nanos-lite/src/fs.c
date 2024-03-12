#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t get_ramdisk_size();

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
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
  assert(0);
}

size_t fs_read(int fd, void *buf, size_t len) {
  Finfo* file = &file_table[fd];
  size_t read_len = file->size < len ? file->size : len;
  ramdisk_read(buf, file->disk_offset + file->open_offset, read_len);
  file->open_offset += read_len;
  return read_len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  Finfo* file = &file_table[fd];
  size_t free_len = file->size - file->open_offset;
  size_t write_len = free_len < len ? free_len : len;
  ramdisk_write(buf, file->disk_offset + file->open_offset, write_len);
  file->open_offset += write_len;
  return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo* file = &file_table[fd];
  printf("openoff: %d, off: %d\n", file->open_offset, offset);
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