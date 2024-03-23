#include <memory.h>
#include <proc.h>

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
  Log("max_brk: %p, brk: %p",current->max_brk, brk);
  if(current->max_brk >= brk) {
    return 0;
  }
  // uintptr_t gap = brk - current->max_brk;
  // size_t pages = (gap + PGSIZE - 1) / PGSIZE;
  // void* paddr = new_page(pages);
  // for(int i = 0; i < pages; i++) {
  //   map(&current->as, (void*)(current->max_brk + i * PGSIZE), paddr + i * PGSIZE, 0);
  //   Log("map vaddr: %p, paddr: %p", (void*)(current->max_brk + i * PGSIZE), paddr + i * PGSIZE);
  // }
  // current->max_brk += pages * PGSIZE;
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
