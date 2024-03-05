#ifndef __X86_RTL_H__
#define __X86_RTL_H__

#include <rtl/rtl.h>
#include "reg.h"

/* RTL pseudo instructions */

static inline def_rtl(lr, rtlreg_t* dest, int r, int width) {
  switch (width) {
    case 4: rtl_mv(s, dest, &reg_l(r)); return;
    case 1: rtl_host_lm(s, dest, &reg_b(r), 1); return;
    case 2: rtl_host_lm(s, dest, &reg_w(r), 2); return;
    default: assert(0);
  }
}

static inline def_rtl(sr, int r, const rtlreg_t* src1, int width) {
  switch (width) {
    case 4: rtl_mv(s, &reg_l(r), src1); return;
    case 1: rtl_host_sm(s, &reg_b(r), src1, 1); return;
    case 2: rtl_host_sm(s, &reg_w(r), src1, 2); return;
    default: assert(0);
  }
}

static inline def_rtl(push, const rtlreg_t* src1) {
  // esp <- esp - 4
  // M[esp] <- src1
  reg_l(R_ESP) -= 4;
  rtl_sm(s, &reg_l(R_ESP), 0, src1, 4);
}

static inline def_rtl(pop, rtlreg_t* dest) {
  // dest <- M[esp]
  // esp <- esp + 4
  rtl_lm(s, dest, &reg_l(R_ESP), 0, 4);
  reg_l(R_ESP) += 4;
}

static inline def_rtl(is_sub_overflow, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
  // dest <- is_overflow(src1 - src2)
  bool is_overflow = false;
  if((sword_t)*src1 > 0 && (sword_t)*src2 < 0)
  {
    *t0 = ((*src1 - *src2) << (width * 8)) >> (width * 8);
    if((sword_t)*t0 <= (sword_t)*src1)
      is_overflow = true;
  }
  else if((sword_t)*src1 < 0 && (sword_t)*src2 > 0)
  {
    *t0 = ((*src1 - *src2) << (width * 8)) >> (width * 8);
    if((sword_t)*t0 > (sword_t)*src1)
      is_overflow = true;
  }
  if(is_overflow)
    *dest = 1;
  else
    *dest = 0;
}

static inline def_rtl(is_sub_carry, rtlreg_t* dest,
    const rtlreg_t* src1, const rtlreg_t* src2) {
  // dest <- is_carry(src1 - src2)
  *dest = *src1 < *src2;
}

static inline def_rtl(is_add_overflow, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
  // dest <- is_overflow(src1 + src2)
  bool is_overflow = false;
  if((sword_t)*src1 > 0 && (sword_t)*src2 > 0)
  {
    *t0 = *src1 + *src2;
    if(((sword_t)*t0 & (1 << (width * 8 - 1))) != 0)
      is_overflow = true;
  }
  else if((sword_t)*src1 < 0 && (sword_t)*src2 < 0)
  {
    *t0 = ((*src1 + *src2) << (width * 8)) >> (width * 8);
    if((sword_t)*t0 >= (sword_t)*src1 || (sword_t)*t0 >= (sword_t)*src2)
      is_overflow = true;
  }
  if(is_overflow)
    *dest = 1;
  else
    *dest = 0;
}

static inline def_rtl(is_add_carry, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) {
  // dest <- is_carry(src1 + src2)
  *t0 = *res + *src1;
  if(*t0 <  *res || *t0 < *src1)
    *dest = 1;
  else
    *dest = 0;
}

#define def_rtl_setget_eflags(f) \
  static inline def_rtl(concat(set_, f), const rtlreg_t* src) { \
    set_flag(concat(F_, f), *src); \
  } \
  static inline def_rtl(concat(get_, f), rtlreg_t* dest) { \
    *dest = get_flag(concat(F_, f)); \
  }

def_rtl_setget_eflags(CF)
def_rtl_setget_eflags(OF)
def_rtl_setget_eflags(ZF)
def_rtl_setget_eflags(SF)

static inline def_rtl(update_ZF, const rtlreg_t* result, int width) {
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  if(((*result << ((sizeof(word_t) - width) << 3))) == 0)
    set_flag(F_ZF, 1);
  else
    set_flag(F_ZF, 0);
}

static inline def_rtl(update_SF, const rtlreg_t* result, int width) {
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  if((*result & (1 << ((width << 3) - 1))) == 0)
    set_flag(F_SF, 0);
  else
    set_flag(F_SF, 1);
}

static inline def_rtl(update_ZFSF, const rtlreg_t* result, int width) {
  rtl_update_ZF(s, result, width);
  rtl_update_SF(s, result, width);
}

#endif
