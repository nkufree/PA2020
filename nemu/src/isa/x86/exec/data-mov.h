static inline def_EHelper(mov) {
  operand_write(s, id_dest, dsrc1);
  print_asm_template2(mov);
}

static inline def_EHelper(push) {
  rtl_push(s, ddest);
  print_asm_template1(push);
}

static inline def_EHelper(pop) {
  rtl_pop(s, ddest);
  print_asm_template1(pop);
}

static inline def_EHelper(pusha) {
  assert(id_dest->width == 4);
  // TODO: width = 2
  *s0 = reg_l(R_ESP);
  printf("eax: %d, edx : %d\n", reg_l(R_EAX), reg_l(R_EDX));
  rtl_push(s, &reg_l(R_EAX));
  rtl_push(s, &reg_l(R_ECX));
  rtl_push(s, &reg_l(R_EDX));
  rtl_push(s, &reg_l(R_EBX));
  rtl_push(s, s0);
  rtl_push(s, &reg_l(R_EBP));
  rtl_push(s, &reg_l(R_ESI));
  rtl_push(s, &reg_l(R_EDI));
  print_asm("pusha");
}

static inline def_EHelper(popa) {
  assert(id_dest->width == 4);
  // TODO: width = 2
  rtl_pop(s, &reg_l(R_EDI));
  rtl_pop(s, &reg_l(R_ESI));
  rtl_pop(s, &reg_l(R_EBP));
  rtl_pop(s, s0);
  rtl_pop(s, &reg_l(R_EBX));
  rtl_pop(s, &reg_l(R_EDX));
  rtl_pop(s, &reg_l(R_ECX));
  rtl_pop(s, &reg_l(R_EAX));
  print_asm("popa");
}

static inline def_EHelper(leave) {
  operand_write(s, id_dest, dsrc1);
  rtl_pop(s, dsrc1);
  print_asm("leave");
}

static inline def_EHelper(cltd) {
  if (s->isa.is_operand_size_16) {
    if((int16_t)reg_w(R_AX) < 0)
      reg_w(R_DX) = 0xffff;
    else
      reg_w(R_DX) = 0;
  }
  else {
    if((int32_t)reg_l(R_EAX) < 0)
      reg_l(R_EDX) = 0xffffffff;
    else
      reg_l(R_EDX) = 0;
  }
  print_asm(s->isa.is_operand_size_16 ? "cwtl" : "cltd");
}

static inline def_EHelper(cwtl) {
  if (s->isa.is_operand_size_16) {
    reg_w(R_AX)=*(int8_t*)(&reg_b(R_AL));
  }
  else {
    reg_l(R_EAX)=*(int16_t*)(&reg_w(R_AX));
  }
  print_asm(s->isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

static inline def_EHelper(movsx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(s, ddest, dsrc1, id_src1->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(movsx);
}

static inline def_EHelper(movzx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  operand_write(s, id_dest, dsrc1);
  print_asm_template2(movzx);
}

static inline def_EHelper(lea) {
  rtl_addi(s, ddest, s->isa.mbase, s->isa.moff);
  operand_write(s, id_dest, ddest);
  print_asm_template2(lea);
}

static inline def_EHelper(xchg) {
  *s0 = *ddest;
  operand_write(s, id_dest, dsrc1);
  operand_write(s, id_src1, s0);
  print_asm_template2(xchg);
}