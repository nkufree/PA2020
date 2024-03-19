#include <cpu/exec.h>
#include "local-include/rtl.h"

void raise_intr(DecodeExecState *s, word_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

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
  TODO();
}
