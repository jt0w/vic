#ifndef _VM_H
#define _VM_H
#include <stdio.h>
#include <stdint.h>

typedef enum {
  OP_NOP = 0x0, // 0
  OP_PUSH,      // 1
  OP_POP,       // 2
  OP_LOAD_REG,  // 3
  OP_PUSH_REG,  // 4
  OP_ADD_STACK, // 5
  OP_ADD_REG,   // 6
  OP_CMP_REG,   // 7
  OP_CMP_STACK, // 8
  OP_JMP,       // 9
  OP_JE,        // 10
  OP_JGE,       // 11
  OP_JG,        // 12
  OP_JLE,       // 13
  OP_JL,        // 14
} OpCode;

#define REG_COUNT 26
typedef enum { REG_A,REG_B,REG_C,REG_D,REG_E,REG_F,REG_G,REG_H,REG_I,REG_J,REG_K,REG_L,REG_M,REG_N,REG_O,REG_P,REG_Q,REG_R,REG_S,REG_T,REG_U,REG_V,REG_W,REG_X,REG_Y,REG_Z } Register;

#define FLAG_COUNT 2
typedef enum  { FLAG_CF, FLAG_ZF } Flag;

typedef struct {
  uint64_t *items;
  size_t count;
  size_t cap;
} Program;
#endif // _VM_H
