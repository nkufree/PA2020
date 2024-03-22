#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    // printf("segments[%d]: %p, %p\n", i, segments[i].start, segments[i].end);
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_cr3(kas.ptr);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
  // printf("protect: updir: %p, kas.ptr: %p\n", updir, kas.ptr);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->cr3 = (vme_enable ? (void *)get_cr3() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->cr3 != NULL) {
    set_cr3(c->cr3);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  uint32_t dir = (uint32_t)va >> 22;
  uint32_t page = ((uint32_t)va >> 12) & 0x3ff;
  PTE* cr3 = (PTE*)as->ptr;
  if (!(cr3[dir] & 0x1)) {
    cr3[dir] = (PTE)pgalloc_usr(PGSIZE) | 0x1;
    printf("alloc new page table: %p +cr3 = %p, va: %p\n", dir, cr3[dir], va);
  }
  PTE* pdir = (PTE*)(cr3[dir] & ~0xfff);
  // printf("cr3: %p, dir: %p, pdir: %p\n", cr3, dir, pdir);
  // if(pdir[page] & 0x1) {
  //   printf("page already map,dir: %p, pdir: %p, pdir[page]: %p\n", dir, pdir, pdir[page]);
  //   assert(0);
  // }
  pdir[page] = ((PTE)pa & ~0xfff) | 0x1;
}

Context* ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context* c = (Context*)(kstack.end - sizeof(Context));
  memset(c, 0, sizeof(Context));
  c->cs = 8;
  c->cr3 = as->ptr;
  c->eflags = 0x2;
  c->eip = (uintptr_t)entry;
  return c;
}
