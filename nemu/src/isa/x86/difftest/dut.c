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
      printf("[reg error]  reg %s exspect: %08x, but %08x", reg_name(i, 4), ref_r->gpr[i]._32, cpu.gpr[i]._32);
    }
    ret = false;
  }
  return ret;
}

void isa_difftest_attach() {
}
