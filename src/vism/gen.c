#include "gen.h"
#include <inttypes.h>

Expr gen_consume(Gen *gen) {
  gen->current = gen->exprs.items[gen->pos++];
  return gen->current;
}

#define push(x) da_push(&p, (x))

Program gen_parse_expr(Gen *gen) {
  Program p = {0};
  if (gen->pos == 0)
    gen_consume(gen);
  switch (gen->current.kind) {
  case EK_PUSH: {
    Token arg = gen->current.args.items[0];
    assert(arg.kind == TK_INT_LIT);
    push(INST_PUSH(arg.as.num));
    break;
  }
  case EK_POP: {
    push(INST_POP);
    break;
  }
  case EK_DUP: {
    Token arg = gen->current.args.items[0];
    assert(arg.kind == TK_INT_LIT);
    push(INST_DUP(arg.as.num));
    break;
  }
  case EK_ADD: {
    push(INST_ADD);
    break;
  }
  case EK_SUB: {
    push(INST_ADD);
    break;
  }
  case EK_MULT: {
    push(INST_MULT);
    break;
  }
  case EK_DIV: {
    push(INST_DIV);
    break;
  }
  case EK_EQ: {
    push(INST_EQ);
    break;
  }
  case EK_JMP: {
    Token arg = gen->current.args.items[0];
    assert(arg.kind == TK_INT_LIT);
    push(INST_JMP(arg.as.num));
    break;
  }
  case EK_JZ: {
    Token arg = gen->current.args.items[0];
    assert(arg.kind == TK_INT_LIT);
    push(INST_JZ(arg.as.num));
    break;
  }
  case EK_JNZ: {
    Token arg = gen->current.args.items[0];
    assert(arg.kind == TK_INT_LIT);
    push(INST_JNZ(arg.as.num));
    break;
  }
  case EK_NOP: {
    push(INST_NOP);
    break;
  }

  case EK_INT:
  case EK_LIT: {
    fprintf(stderr,
            "FATAL ERROR: This should never happen please contact a maitainer "
            "(%s:%d)",
            __FILE__, __LINE__);
    exit(1);
  }
  }
  gen_consume(gen);
  return p;
}
