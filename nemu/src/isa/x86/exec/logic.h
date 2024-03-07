#include "cc.h"

static inline def_EHelper(test) {
  rtl_and(s, s0, ddest, dsrc1);
  *s1 = 0;
  rtl_set_CF(s, s1);
  rtl_set_OF(s, s1);
  rtl_update_ZFSF(s, s0, id_dest->width);
  print_asm_template2(test);
}

static inline def_EHelper(and) {
  rtl_and(s, ddest, ddest, dsrc1);
  operand_write(s, id_dest, ddest);
  *s0 = 0;
  rtl_set_CF(s, s0);
  rtl_set_OF(s, s0);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  print_asm_template2(and);
}

static inline def_EHelper(xor) {
  rtl_xor(s, ddest, ddest, dsrc1);
  operand_write(s, id_dest, ddest);
  *s0 = 0;
  rtl_set_CF(s, s0);
  rtl_set_OF(s, s0);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  print_asm_template2(xor);
}

static inline def_EHelper(or) {
  rtl_or(s, ddest, ddest, dsrc1);
  operand_write(s, id_dest, ddest);
  *s0 = 0;
  rtl_set_CF(s, s0);
  rtl_set_OF(s, s0);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  print_asm_template2(or);
}

static inline def_EHelper(not) {
  rtl_not(s, ddest, ddest);
  operand_write(s, id_dest, ddest);
  print_asm_template1(not);
}

static inline def_EHelper(sar) {
  rtl_sar(s, ddest, ddest, dsrc1);
  operand_write(s, id_dest, ddest);
  rtl_update_ZF(s, ddest, id_dest->width);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(sar);
}

static inline def_EHelper(shl) {
  rtl_shl(s, ddest, ddest, dsrc1);
  operand_write(s, id_dest, ddest);
  rtl_update_ZF(s, ddest, id_dest->width);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shl);
}

static inline def_EHelper(shr) {
  rtl_shr(s, ddest, ddest, dsrc1);
  operand_write(s, id_dest, ddest);
  rtl_update_ZF(s, ddest, id_dest->width);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shr);
}

static inline def_EHelper(rol) {
  rtl_mv(s,s0,dsrc1);
  while(*s0 != 0)
  {
    if(s->isa.is_operand_size_16)
    {
      rtl_msb(s,s1,ddest,2);
      *ddest = *ddest * 2 +*s1;
      rtl_andi(s,ddest,ddest,0xffff);
    }
    else{
      rtl_msb(s,s1,ddest,4);
      *ddest = *ddest * 2 +*s1;
    }
    rtl_subi(s,s0,s0,1);
  }
}

static inline def_EHelper(ror) {
  rtl_mv(s,s0,dsrc1);
  while(*s0 != 0)
  {
    if(s->isa.is_operand_size_16)
    {
      rtl_andi(s,s1,ddest,1);
      *ddest = *ddest / 2 +((*s1)<<15);
      rtl_andi(s,ddest,ddest,0xffff);
    }
    else{
      rtl_andi(s,s1,ddest,1);
      *ddest = *ddest / 2 +(*s1 << 31);
    }
    rtl_subi(s,s0,s0,1);
  }
}

static inline def_EHelper(bsr) {
  if(*dsrc1 == 0)
  {
    rtl_set_ZF(s, dsrc1);
  }
  else{
    uint32_t pos = (s->isa.is_operand_size_16 ? 16 : 32) - 1;
    *s1 = 1;
    rtl_set_ZF(s, s1);
    uint32_t loop = 0;
    while((*dsrc1 & (1 << pos)) == 0 )
    {
      pos -= 1;
      loop++;
    }
    *ddest = pos;
  }
  operand_write(s, id_dest, ddest);
  print_asm_template2(bsr);
}

static inline def_EHelper(setcc) {
  uint32_t cc = s->opcode & 0xf;
  rtl_setcc(s, ddest, cc);
  operand_write(s, id_dest, ddest);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}
