#include <memory.h>

static void *pf = NULL;

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
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
