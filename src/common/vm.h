#ifndef _VM_H
#define _VM_H
#include <stdint.h>
#include <stdio.h>

typedef enum {
  OP_NOP = 0x0,
  OP_PUSH,
  OP_PUSH_REG,
  OP_POP_REG,

  OP_LOAD,
  OP_LOAD_REG,

  OP_ADD,
  OP_SUB,
  OP_MULT,
  OP_DIV,

  OP_CMP,

  OP_JMP,
  OP_JE,
  OP_JGE,
  OP_JG,
  OP_JLE,
  OP_JL,
} OpCode;

#define REG_COUNT 26
typedef enum {
  REG_A,
  REG_B,
  REG_C,
  REG_D,
  REG_E,
  REG_F,
  REG_G,
  REG_H,
  REG_I,
  REG_J,
  REG_K,
  REG_L,
  REG_M,
  REG_N,
  REG_O,
  REG_P,
  REG_Q,
  REG_R,
  REG_S,
  REG_T,
  REG_U,
  REG_V,
  REG_W,
  REG_X,
  REG_Y,
  REG_Z
} Register;

#define FLAG_COUNT 2
typedef enum { FLAG_CF, FLAG_ZF } Flag;

typedef struct {
  uint64_t *items;
  size_t count;
  size_t cap;
} Program;

#endif // _VM_H
