#include <isa.h>
#include <monitor/difftest.h>
#include "../local-include/reg.h"
#include "difftest.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool ret = true;
  for(int i = 0; i < 8; i++)
  {
    if(ref_r->gpr[i]._32 != cpu.gpr[i]._32)
    {
      printf("[reg error]  reg %%%s expect: 0x%08x, but: 0x%08x\n", reg_name(i, 4), ref_r->gpr[i]._32, cpu.gpr[i]._32);
      ret = false;
    }
  }
  if(ref_r->pc != cpu.pc)
  {
    printf("[pc error]  pc expect: 0x%08x, but: 0x%08x\n", ref_r->pc, cpu.pc);
      ret = false;
  } 
  return ret;
}

void isa_difftest_attach() {
}
