#ifndef _VM_H
#define _VM_H
#include <stdio.h>
#include <stdint.h>

#define JE 0
#define JGE 1
#define JG 2
#define JLE 3
#define JL 3

typedef enum {
  OP_NOP = 0x0,
  OP_PUSH,
  OP_POP,
  OP_LOAD_REG,
  OP_PUSH_REG,
  OP_CMP_REG,
  OP_CMP_STACK,
  OP_JMP,
  OP_JGE,
  OP_JG,
  OP_JLE,
  OP_JL,
} OpCode;

#define REG_COUNT 26
typedef enum { A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z } Register;

typedef struct {
  uint64_t *items;
  size_t count;
  size_t cap;
} Program;
#endif // _VM_H
