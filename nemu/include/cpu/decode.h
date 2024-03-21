#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include <isa.h>

#define OP_STR_SIZE 40
enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

typedef struct {
  uint32_t type;    // 类型：寄存器、内存、立即数
  int width;        // 操作数位宽
  union {
    uint32_t reg;   // 寄存器编号
    word_t imm;     // 立即数
    sword_t simm;   // 有符号立即数
  };
  rtlreg_t *preg;   // 操作数的值指针，指向操作数的值，如果是存在的寄存器，指向寄存器，否则指向val
  rtlreg_t val;     // 该操作数的值
  char str[OP_STR_SIZE];
} Operand;

typedef struct {
  uint32_t opcode;
  vaddr_t seq_pc;  // sequential pc
  uint32_t is_jmp;
  vaddr_t jmp_pc;
  Operand src1, dest, src2;
  int width;
  rtlreg_t tmp_reg[4];
  ISADecodeInfo isa;
} DecodeExecState;

#define def_DHelper(name) void concat(decode_, name) (DecodeExecState *s)

#ifdef DEBUG
#define print_Dop(...) snprintf(__VA_ARGS__)
#else
#define print_Dop(...)
#endif

#endif
