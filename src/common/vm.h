#ifndef _VM_H
#define _VM_H
#include <stdint.h>
#include <stdio.h>

typedef uint64_t Word;

typedef enum {
  OP_NOP = 0x0,
  OP_PUSH,
  OP_POP,
  OP_DUP,

  OP_ADD,
  OP_SUB,
  OP_MULT,
  OP_DIV,

  OP_EQ,

  OP_JMP,
  OP_JNZ,
  OP_JZ,
} OpCode;

typedef struct {
  OpCode opcode;
  uint64_t operand;
} Inst;

#define INST_NOP ((Inst){.opcode = OP_NOP})
#define INST_PUSH(x) ((Inst){.opcode = OP_PUSH, .operand = (x)})
#define INST_POP ((Inst){.opcode = OP_POP})
#define INST_DUP(x) ((Inst){.opcode = OP_DUP, .operand = (x)})
#define INST_ADD ((Inst){.opcode = OP_ADD})
#define INST_SUB ((Inst){.opcode = OP_SUB})
#define INST_MULT ((Inst){.opcode = OP_MULT})
#define INST_DIV ((Inst){.opcode = OP_DIV})
#define INST_EQ ((Inst){.opcode = OP_EQ})
#define INST_JMP(x) ((Inst){.opcode = OP_JMP, .operand = (x)})
#define INST_JNZ(x) ((Inst){.opcode = OP_JNZ, .operand = (x)})
#define INST_JZ(x) ((Inst){.opcode = OP_JZ, .operand = (x)})

typedef struct {
  Inst *items;
  size_t count;
  size_t cap;
} Program;
#endif // _VM_H
