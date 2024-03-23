#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>
paddr_t isa_mmu_translate(vaddr_t vaddr, int type, int len) {
  uint32_t dir = (uint32_t)vaddr >> 22;
  uint32_t page = (uint32_t)vaddr >> 12 & 0x3ff;
  uint32_t offset = (uint32_t)vaddr & 0xfff;
  uint32_t pde = paddr_read((cpu.cr3 & 0xfffff000) | (dir << 2), 4);
  if((pde & 0x1) == 0) {
    Log("vaddr: %x, pde = %x", vaddr, pde);
    return MEM_RET_FAIL;
  }
  uint32_t pte_val = paddr_read((pde & 0xfffff000) | (page <<  2), 4);
  if((pte_val & 0x1) == 0) {
    Log("vaddr: %x, pte_val = %x", vaddr, pte_val);
    return MEM_RET_FAIL;
  }
  if(offset + len > PAGE_SIZE) {
    return MEM_RET_CROSS_PAGE;
  }
  return MEM_RET_OK;
}

paddr_t page_table_walk(vaddr_t vaddr) {
  uint32_t dir = (uint32_t)vaddr >> 22;
  uint32_t page = (uint32_t)vaddr >> 12 & 0x3ff;
  uint32_t offset = (uint32_t)vaddr & 0xfff;
  uint32_t pde = paddr_read((cpu.cr3 & 0xfffff000) | dir << 2, 4);
  uint32_t pte_val = paddr_read((pde & 0xfffff000) | page <<  2, 4);
  return (pte_val & 0xfffff000) | offset;
}