#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr __elf, *elf = &__elf;
  Elf_Phdr __ph, *ph = &__ph;
  fs_read(fd, elf, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)elf->e_ident == 0x464C457F);

  uint32_t ph_index;
  for(ph_index = 0; ph_index < elf->e_phnum; ph_index++)
  {
    size_t ph_off = elf->e_phoff + sizeof(Elf_Phdr) * ph_index;
    fs_lseek(fd, ph_off, SEEK_SET);
    fs_read(fd, ph, sizeof(Elf_Phdr));
    if(ph->p_type != PT_LOAD)
    {
      continue;
    }
    assert(ph->p_filesz <= ph->p_memsz);
    uintptr_t start = ph->p_vaddr;
    // 复制TEXT/DATA到指定位置
    fs_lseek(fd, ph->p_offset, SEEK_SET);
    fs_read(fd, (void*)start, ph->p_filesz);
    // 构建BSS
    start = ph->p_vaddr + ph->p_filesz;
    memset((void*)start, 0, ph->p_memsz - ph->p_filesz);
  }
  fs_close(fd);

  return elf->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

