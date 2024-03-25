#include <cpu/exec.h>
#include "local-include/rtl.h"

#define IRQ_TIMER 32 

void raise_intr(DecodeExecState *s, word_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  clear_flag(F_IF);
  if((cpu.cs & 0x3) == 3) {
    uint32_t gdt_idx = cpu.tr;
    uint32_t gdt_addr = cpu.gdtr.base + gdt_idx;
    uint32_t base = vaddr_read(gdt_addr + 2, 2) 
                  | (vaddr_read(gdt_addr + 4, 1) << 16) 
                  | (vaddr_read(gdt_addr + 7, 1) << 24);
    uint32_t ksp = vaddr_read(base + 4, 4);
    uint32_t prev = cpu.esp;
    if(ksp !=0)
      cpu.esp = ksp;
    rtl_push(s,&cpu.ss);
    rtl_push(s,&prev);
    vaddr_write(base + 4, 0, 4);
  }
  uint32_t addr = cpu.idtr.base + 8 * NO;
  uint32_t low = vaddr_read(addr, 4);
  uint32_t high = vaddr_read(addr + 4, 4);
  assert((high & 0x00008000) != 0); // p != 0
  uint32_t dest_addr = (low & 0xffff) | (high & 0xffff0000);
  rtl_push(s, &cpu.eflags);
  rtl_push(s, &cpu.cs);
  rtl_push(s, &ret_addr);
  rtl_j(s, dest_addr);
}

void query_intr(DecodeExecState *s) {
  if (cpu.INTR && get_flag(F_IF)) {
    cpu.INTR = 0;
    raise_intr(s, IRQ_TIMER, cpu.pc);
    update_pc(s);
  }
}
