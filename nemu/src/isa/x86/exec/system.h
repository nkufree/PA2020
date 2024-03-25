#include <monitor/difftest.h>

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);
void raise_intr(DecodeExecState *s, word_t NO, vaddr_t ret_addr);

static inline def_EHelper(lidt) {
  cpu.idtr.limit = (*ddest) >> 16;
  cpu.idtr.base = vaddr_read(*(s->isa.mbase) + s->isa.moff + 2, 4);
  if(id_dest->width == 2)
    cpu.idtr.base &= 0x00ffffff;
  print_asm_template1(lidt);
}

static inline def_EHelper(ltr) {
  cpu.tr = *ddest;
  print_asm_template1(ltr);
}

static inline def_EHelper(lgdt) {
  cpu.gdtr.limit = (*ddest)>>16;
  cpu.gdtr.base = vaddr_read(*(s->isa.mbase) + s->isa.moff + 2, 4);
  if(id_dest->width == 2)
    cpu.gdtr.base &= 0x00ffffff;
  print_asm_template1(lgdt);
}

static inline def_EHelper(mov_r2cr) {
  switch(id_dest->reg) {
    case 0:
      cpu.cr0 = *dsrc1;
      break;
    case 3:
      cpu.cr3 = *dsrc1;
      break;
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src1->reg, 4), id_dest->reg);
}

static inline def_EHelper(mov_cr2r) {
  switch(id_src1->reg) {
    case 0:
      *ddest = cpu.cr0;
      break;
    case 3:
      *ddest = cpu.cr3;
      break;
  }
  print_asm("movl %%cr%d,%%%s", id_src1->reg, reg_name(id_dest->reg, 4));

#ifndef __DIFF_REF_NEMU__
  difftest_skip_ref();
#endif
}

static inline def_EHelper(int) {
  raise_intr(s, *ddest, s->seq_pc);
  print_asm("int %s", id_dest->str);

#ifndef __DIFF_REF_NEMU__
  difftest_skip_dut(1, 2);
#endif
}

static inline def_EHelper(iret) {
  rtl_pop(s, s0);
  rtl_pop(s, &(cpu.cs));
  rtl_pop(s, &(cpu.eflags));
  if((cpu.cs&0x3) == 3) {
    uint32_t gdt_addr = cpu.gdtr.base + cpu.tr;
    uint32_t base = vaddr_read(gdt_addr+2,2) 
                | (vaddr_read(gdt_addr+4,1)<<16) 
                | (vaddr_read(gdt_addr+7,1)<<24);
    rtl_pop(s,s1);
    rtl_pop(s,&cpu.ss);
    vaddr_write(base + 4, cpu.esp, 4);
    cpu.esp = *s1;
  }
  rtl_j(s,*s0);
  set_flag(F_IF);
  print_asm("iret");

#ifndef __DIFF_REF_NEMU__
  difftest_skip_ref();
#endif
}

static inline def_EHelper(in) {
  switch(id_dest->width)
	{
		case 1:
			*s0 = pio_read_b(*dsrc1);
			break;
		case 2:
		    *s0 = pio_read_w(*dsrc1);
			break;
		case 4:
			*s0 = pio_read_l(*dsrc1);
			break;
	}
	operand_write(s, id_dest, s0);
  print_asm_template2(in);
}

static inline def_EHelper(out) {
  switch (id_src1->width){
		case 1:
			pio_write_b(*ddest, *dsrc1);
			break;
		case 2:
			pio_write_w(*ddest, *dsrc1);
			break;
		case 4:
			pio_write_l(*ddest, *dsrc1);
			break;
  }
  print_asm_template2(out);
}
