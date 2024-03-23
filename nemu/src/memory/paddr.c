#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <device/map.h>
#include <stdlib.h>
#include <time.h>

static uint8_t pmem[PMEM_SIZE] PG_ALIGN = {};

void* guest_to_host(paddr_t addr) { return &pmem[addr]; }
paddr_t host_to_guest(void *addr) { return (void *)pmem - addr; }

IOMap* fetch_mmio_map(paddr_t addr);

void init_mem() {
#ifndef DIFF_TEST
  srand(time(0));
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < PMEM_SIZE / sizeof(p[0]); i ++) {
    p[i] = rand();
  }
#endif
}

static inline bool in_pmem(paddr_t addr) {
  return (PMEM_BASE <= addr) && (addr <= PMEM_BASE + PMEM_SIZE - 1);
}

static inline word_t pmem_read(paddr_t addr, int len) {
  void *p = &pmem[addr - PMEM_BASE];
  switch (len) {
    case 1: return *(uint8_t  *)p;
    case 2: return *(uint16_t *)p;
    case 4: return *(uint32_t *)p;
#ifdef ISA64
    case 8: return *(uint64_t *)p;
#endif
    default: assert(0);
  }
}

static inline void pmem_write(paddr_t addr, word_t data, int len) {
  void *p = &pmem[addr - PMEM_BASE];
  switch (len) {
    case 1: *(uint8_t  *)p = data; return;
    case 2: *(uint16_t *)p = data; return;
    case 4: *(uint32_t *)p = data; return;
#ifdef ISA64
    case 8: *(uint64_t *)p = data; return;
#endif
    default: assert(0);
  }
}

/* Memory accessing interfaces */

inline word_t paddr_read(paddr_t addr, int len) {
  if (in_pmem(addr)) return pmem_read(addr, len);
  else return map_read(addr, len, fetch_mmio_map(addr));
}

inline void paddr_write(paddr_t addr, word_t data, int len) {
  if (in_pmem(addr)) pmem_write(addr, data, len);
  else map_write(addr, data, len, fetch_mmio_map(addr));
}

paddr_t page_table_walk(vaddr_t vaddr);

word_t vaddr_read_cross_page(vaddr_t vaddr ,int type,int len) {
  Log("vaddr_read_cross_page: vaddr = 0x%x, type = %d, len = %d", vaddr, type, len);
  paddr_t paddr = page_table_walk(vaddr);
  uint32_t offset = vaddr & 0xfff;
  uint32_t len1 = PAGE_SIZE - offset;
  uint32_t len2 = len - len1;
  word_t data1 = paddr_read(paddr, 4) & ((1 << (len1 * 8)) - 1);
  vaddr_t vaddr2 = (vaddr & 0xfffff000) + PAGE_SIZE;
  paddr_t paddr2 = page_table_walk(vaddr2);
  word_t data2 = paddr_read(paddr2, 4) & ((1 << (len2 * 8)) - 1);
  return (data2 << (len1 * 8)) | data1;
}

void vaddr_write_cross_page(vaddr_t vaddr ,word_t data,int len) {
  Log("vaddr_write_cross_page: vaddr = 0x%x, data = 0x%x, len = %d", vaddr, data, len);
  for(int i = 0; i < len; i++) {
    paddr_write(page_table_walk(vaddr + i), (data >> (i * 8)) & 0xff, 1);
  }
  return;
}

word_t vaddr_mmu_read(vaddr_t addr, int len, int type) {
  assert(len == 1 || len == 2 || len == 4);
  paddr_t pg_base = isa_mmu_translate(addr, type, len);
  if(pg_base == MEM_RET_OK) {
    paddr_t paddr = page_table_walk(addr);
    return paddr_read(paddr, len);
  }
  else if(pg_base == MEM_RET_CROSS_PAGE) {
    return vaddr_read_cross_page(addr, type, len);
  }
  else {
    printf("read error[%d], vaddr: %p, len: %d, type: %d, pc = 0x%x\n", pg_base, (void*)(uintptr_t)addr, len, type, cpu.pc);
    // assert(0);
  }
  return 0;
}

void vaddr_mmu_write(vaddr_t addr, word_t data, int len) {
  assert(len == 1 || len == 2 || len == 4);
  if(addr <= 0x400053b2 && addr + len >= 0x400053b2)
    Log("vaddr_mmu_write: vaddr = 0x%x, data = 0x%x, len = %d", addr, data, len);
  paddr_t pg_base = isa_mmu_translate(addr, data, len);
  if(pg_base == MEM_RET_OK) {
    paddr_t paddr = page_table_walk(addr);
    // if(addr >= 0x228dA00 && addr <= 0x228dA20)
    // Log("vaddr_mmu_write: vaddr = 0x%x, paddr = 0x%x, data = 0x%x, len = %d", addr, paddr, data, len);
    paddr_write(paddr, data, len);
  }
  else if(pg_base == MEM_RET_CROSS_PAGE) {
    vaddr_write_cross_page(addr, data, len);
  }
  else {
    printf("write error[%d], vaddr: %p, data: %x, len: %d, pc = 0x%x\n", pg_base, (void*)(uintptr_t)addr, data, len, cpu.pc);
    assert(0);
  }
  return;
}


#define def_vaddr_template(bytes) \
word_t concat(vaddr_ifetch, bytes) (vaddr_t addr) { \
  int ret = isa_vaddr_check(addr, MEM_TYPE_IFETCH, bytes); \
  if (ret == MEM_RET_OK) return paddr_read(addr, bytes); \
  else if(ret == MEM_RET_NEED_TRANSLATE) return vaddr_mmu_read(addr, bytes, MEM_TYPE_IFETCH); \
  return 0; \
} \
word_t concat(vaddr_read, bytes) (vaddr_t addr) { \
  int ret = isa_vaddr_check(addr, MEM_TYPE_READ, bytes); \
  if (ret == MEM_RET_OK) return paddr_read(addr, bytes); \
  else if(ret == MEM_RET_NEED_TRANSLATE) return vaddr_mmu_read(addr, bytes, MEM_TYPE_READ); \
  return 0; \
} \
void concat(vaddr_write, bytes) (vaddr_t addr, word_t data) { \
  int ret = isa_vaddr_check(addr, MEM_TYPE_WRITE, bytes); \
  if (ret == MEM_RET_OK) paddr_write(addr, data, bytes); \
  else if(ret == MEM_RET_NEED_TRANSLATE) vaddr_mmu_write(addr, data, bytes); \
}


def_vaddr_template(1)
def_vaddr_template(2)
def_vaddr_template(4)
#ifdef ISA64
def_vaddr_template(8)
#endif
