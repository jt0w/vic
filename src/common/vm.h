#ifndef _VM_H
#define _VM_H
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef union {
  uint64_t as_u64;
  void *as_pointer;
} Word;

#define WORD_U64(x) ((Word){.as_u64 = (x)})
#define WORD_POINTER(x) ((Word){.as_pointer = (x)})

typedef enum {
  OP_NOP = 0x0,
  OP_PUSH,
  OP_POP,
  OP_DUP,
  OP_SWAP,

  OP_ADD,
  OP_SUB,
  OP_MULT,
  OP_DIV,

  OP_EQ,

  OP_JMP,
  OP_JNZ,
  OP_JZ,

  OP_ALLOC,
  OP_WRITE,

  OP_RET,
  OPCODE_COUNT,
} OpCode;

typedef struct {
  OpCode opcode;
  Word operand;
} Inst;

#define INST_NOP      ((Inst){.opcode = OP_NOP                   })
#define INST_PUSH(x)  ((Inst){.opcode = OP_PUSH,   .operand = (x)})
#define INST_POP      ((Inst){.opcode = OP_POP                   })
#define INST_DUP(x)   ((Inst){.opcode = OP_DUP,    .operand = (x)})
#define INST_SWAP(x)  ((Inst){.opcode = OP_SWAP,   .operand = (x)})
#define INST_ADD      ((Inst){.opcode = OP_ADD                   })
#define INST_SUB      ((Inst){.opcode = OP_SUB                   })
#define INST_MULT     ((Inst){.opcode = OP_MULT                  })
#define INST_DIV      ((Inst){.opcode = OP_DIV                   })
#define INST_EQ       ((Inst){.opcode = OP_EQ                    })
#define INST_JMP(x)   ((Inst){.opcode = OP_JMP,    .operand = (x)})
#define INST_JNZ(x)   ((Inst){.opcode = OP_JNZ,    .operand = (x)})
#define INST_JZ(x)    ((Inst){.opcode = OP_JZ,     .operand = (x)})
#define INST_ALLOC(x) ((Inst){.opcode = OP_ALLOC, .operand = (x)})
#define INST_WRITE(x) ((Inst){.opcode = OP_WRITE, .operand = (x)})
#define INST_RET      ((Inst){.opcode = OP_RET                  })

typedef struct {
  Inst *items;
  size_t count;
  size_t cap;
} Program;

typedef struct {
  size_t count;
  size_t cap;
  Word *items;
} Stack;

#ifndef VM_MEMORY_CAP
#define VM_MEMORY_CAP 1024
#endif

typedef struct {
  Program program;
  size_t pc;

  Stack stack;
  // TODO: make this grow
  uint8_t memory[VM_MEMORY_CAP];
  size_t memory_pos;
} VM;


typedef enum {
  TYPE_INT,
  TYPE_MEM_ADRESS,
} Type;

typedef enum {
  RESULT_OK,
  RESULT_ERROR_STACK_OVERFLOW,
  RESULT_ERROR_STACK_UNDERFLOW,
  RESULT_ERROR_ILLEGAL_INST,
  RESULT_ERROR_ILLEGAL_INST_ACCESS,
  RESULT_ERROR_MEMORY_OVERFLOW,
  RESULT_ERROR_ILLEGAL_MEMORY_ACCESS,
} Result;

typedef Result (*ExecuteFun)(VM*, Inst);

#define INPUTS_CAP 2
#define OUTPUTS_CAP 1

typedef struct {
  OpCode code;
  const char *name;
  bool has_operand;
  Type inputs[INPUTS_CAP];
  Type outputs[OUTPUTS_CAP];
  ExecuteFun exe;
} Instruction_Mapping;

extern const Instruction_Mapping INST_MAP[OPCODE_COUNT];
#endif // _VM_H
