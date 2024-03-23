#include <memory.h>
#include <proc.h>

#define OFFSET 0xfff
#define PGSIZE 4096
#define page_begin_vaddr(A) ((void*)((uintptr_t)(A)&(~OFFSET)))
#define page_end_vaddr(A) ((void*)(((uintptr_t)(A)&(~OFFSET))+PGSIZE))
#define min(A,B) ((size_t)(A) < (size_t)(B)? (A):(B))
#define vaddr_offset(A) ((uintptr_t)(A)&OFFSET)

static void *pf = NULL;
extern PCB *current;

void* new_page(size_t nr_page) {
  void* start = pf;
  pf += nr_page * PGSIZE;
  return start;
}

static inline void* pg_alloc(int n) {
  size_t nr_page = (n + PGSIZE - 1) / PGSIZE;
  void* p = new_page(nr_page);
  memset(p, 0, nr_page * PGSIZE);
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk){
  if(current->max_brk >= brk) {
    current->max_brk = brk;
    return 0;
  }
  uintptr_t gap = brk - current->max_brk;
  size_t pages = (gap + PGSIZE - 1) / PGSIZE;
  void* paddr = new_page(pages);
  for(int i = 0; i < pages; i++) {
    map(&current->as, (void*)(current->max_brk + i * PGSIZE), paddr + i * PGSIZE, 0);
  }
  return 0;

}
// int mm_brk(uintptr_t brk)
// {
//   if (current->max_brk >= brk)
//   {
//     return 0;
//   }
//   else
//   {
//     void *vaddr = (void *)current->max_brk;
//     void *end = (void *)brk;
//     void *paddr = NULL;
//     uintptr_t len = (uintptr_t)min(page_end_vaddr(vaddr) - vaddr, end - vaddr);
//     if(len < PGSIZE) {
//       vaddr += len;
//     } else if(len == PGSIZE) {
//       assert(vaddr_offset(vaddr) == 0);
//     } else assert(0);
//     while((uintptr_t)vaddr < (uintptr_t)end)
//     {
//       len = (uintptr_t)min(page_end_vaddr(vaddr) - vaddr, end - vaddr);
//       paddr = new_page(1);
//       map(&current->as,page_begin_vaddr(vaddr),paddr,0);
//       vaddr += len;
//     }
//     current->max_brk = brk;
//   }
//   return 0; 
// }

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
