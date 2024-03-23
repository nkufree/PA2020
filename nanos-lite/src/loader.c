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
Context* ucontext(AddrSpace *as, Area kstack, void *entry);
void* new_page(size_t nr_page);
void map(AddrSpace *as, void *va, void *pa, int prot);
void protect(AddrSpace *as);

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
    size_t build_size = 0;
    void* last_page = NULL;
    size_t len = 0;
    while (build_size < ph->p_filesz)
    {
      len = ph->p_filesz - build_size;
      if (len > PGSIZE)
      {
        len = PGSIZE;
      }
      last_page = new_page(1);
      fs_read(fd, last_page, len);
    //   Log("map vaddr: %p, len: %d, paddr: %p", start, len, last_page);
      map(&pcb->as, (void*)start, last_page, 0);
      start += len;
      build_size += len;
    }
    if(ph->p_filesz == ph->p_memsz)
    {
      continue;
    }
    // fs_read(fd, (void*)start, ph->p_filesz);
    // 构建BSS
    start = ph->p_vaddr + ph->p_filesz;
    // 最后一页没填满，不需要分配新页，直接写入
    if(len < PGSIZE) {
      size_t offset = len;
      len = ph->p_memsz - build_size < PGSIZE - len ? ph->p_memsz - build_size : PGSIZE - len;
    //   Log("write vaddr: %p, len: %d, paddr: %p", start, len, last_page);
      memset((void*)last_page+offset, 0, len);
      start += len;
      build_size += len;
    }
    while(build_size < ph->p_memsz)
    {
      len = ph->p_memsz - build_size;
      if (len > PGSIZE)
      {
        len = PGSIZE;
      }
      last_page = new_page(1);
      memset(last_page, 0, len);
    //   Log("map bss vaddr: %p, len: %d, paddr: %p", start, len, last_page);
      map(&pcb->as, (void*)start, last_page, 0);
      start += len;
      build_size += len;
    }
    // memset((void*)start, 0, ph->p_memsz - ph->p_filesz);
    pcb->max_brk = (uintptr_t)start % PGSIZE == 0 ? (uintptr_t)start : (uintptr_t)start + PGSIZE - (uintptr_t)start % PGSIZE;
  }
  fs_close(fd);

  return elf->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

// void context_uload(PCB* pcb, const char *filename, char *const argv[], char *const envp[]) {
//   // 解析参数和环境变量
//   int argc = 0, envc = 0;
//   int argvlen = 0, envplen = 0;
//   while(argv != NULL && argv[argc] != NULL)
//   {
//     argvlen += strlen(argv[argc]) + 1;
//     argc++;
//   }
//   while(envp != NULL && envp[envc] != NULL)
//   {
//     envplen += strlen(envp[envc]) + 1;
//     envc++;
//   }
// //   Log("argc: %d, envc: %d\n", argc, envc);
//   protect(&pcb->as);
// //   memset(pcb->as.ptr + (((uint32_t)pcb->as.area.start) >> 20), 0, (uint32_t)(pcb->as.area.end - pcb->as.area.start) >> 20);
//   void* start = new_page(8);
//   void* end = start + 8 * PGSIZE;
//   Area ustack = {start, end};
// //   printf("pcb->as.area.start: %p, pcb->as.area.end: %p\n", pcb->as.area.start, pcb->as.area.end);
//   for(int i = 0; i < 8; i++) {
//     map(&pcb->as, pcb->as.area.end - i * PGSIZE, end - i * PGSIZE, 0x7);
//     // printf("map vaddr: %p, paddr: %p\n", pcb->as.area.end - i * PGSIZE, end - i * PGSIZE);
//   }
//   end -= argvlen + envplen + (argc + envc + 2) * sizeof(char*) + sizeof(int) + 12;
//   *((int*)end) = argc;
//   char** argvp = end + sizeof(int);
//   char* string_area = end + sizeof(int) + (argc + envc + 2) * sizeof(char*);
// //   Log("ret: %p, argvp: %p, string_area: %p\n", ret, argvp, string_area);
//   for(int i = 0; i < argc; i++)
//   {
//     strcpy(string_area, argv[i]);
//     *argvp = string_area;
//     argvp++;
//     string_area += strlen(argv[i]) + 1;
//   }
//   argvp++;
//   for(int i = 0; i < envc; i++)
//   {
//     strcpy(string_area, envp[i]);
//     *argvp = string_area;
//     argvp++;
//     string_area += strlen(envp[i]) + 1;
//   }
//   uintptr_t entry = loader(pcb, filename);
//   Log("entry = %p", entry);
//   pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb->stack + STACK_SIZE }, (void*)entry);
//   pcb->cp->GPRx = (uintptr_t)(pcb->as.area.end - (ustack.end - end));
// }

#define PGSIZE 4096
#define stack_num 8
#define OFFSET 0xfff
#define page_begin_vaddr(A) ((void*)((uintptr_t)(A)&(~OFFSET)))
#define page_end_vaddr(A) ((void*)(((uintptr_t)(A)&(~OFFSET))+PGSIZE))
#define paddr_offset(A,B) ((void*)(((uintptr_t)(A)&(OFFSET))|(uintptr_t)(((uintptr_t)(B)>>12)<<12)))
#define min(A,B) ((uintptr_t)(A) < (uintptr_t)(B)? (A):(B))

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[])
{
  protect(&pcb->as);
  void *start = new_page(stack_num);
  void *end = start + stack_num * PGSIZE;
  for (int i = 1; i <= stack_num; i++)
  {
    map(&pcb->as, pcb->as.area.end - i * PGSIZE, end - i * PGSIZE, 0x7);
  }
  Area ustack = {start, end};

  uintptr_t entry = loader(pcb, filename);

  int size = 0, size_argv = 0, size_envp = 0, argc = 0, envc = 0;
  while (argv[argc] != NULL)
  {
    size_argv += strlen(argv[argc]) + 1;
    argc++;
  }
  while (envp[envc] != NULL)
  {
    size_envp += strlen(envp[envc]) + 1;
    envc++;
  }
  size = size_envp + size_argv + sizeof(uintptr_t) * (argc + 4 + envc);
  size = size - size % sizeof(uintptr_t);
  void *ret = pcb->as.area.end - size;
  void *args_start = ustack.end - size;
  void *str_start = args_start + sizeof(uintptr_t) * (argc + 3 + envc);

  memset(args_start, 0, ustack.end - args_start);
  *(uintptr_t *)args_start = argc;
  for (int i = 0; i < argc; i++)
  {
    memcpy(str_start, argv[i], strlen(argv[i]));
    *(uintptr_t *)(args_start + sizeof(uintptr_t) * (i + 1)) = (uintptr_t)str_start;
    str_start += strlen(argv[i]) + 1;
  }
  *(uintptr_t *)(args_start + sizeof(uintptr_t) * (1 + argc)) = 0;
  for (int i = 0; i < envc; i++)
  {
    memcpy(str_start, envp[i], strlen(envp[i]));
    *(uintptr_t *)(args_start + sizeof(uintptr_t) * (argc + 2 + i)) = (uintptr_t)str_start;
    str_start += strlen(envp[i]) + 1;
  }
  *(uintptr_t *)(args_start + sizeof(uintptr_t) * (argc + 2 + envc)) = 0;

  Area stack = {pcb->stack, pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(&pcb->as, stack, (void *)entry);
  pcb->cp->GPRx = (uintptr_t)ret;
  /* uintptr_t entry = loader(pcb, filename);
  Area stack = {pcb->stack,pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(&pcb->as,stack,(void*)entry);
  pcb->cp->GPRx = (uintptr_t)heap.end; */
  //while (1);
}