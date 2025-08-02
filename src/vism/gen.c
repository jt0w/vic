#include "gen.h"
#include <inttypes.h>

Expr gen_consume(Gen *gen) {
  gen->current = gen->exprs.items[gen->pos++];
  return gen->current;
}

#define push(x) da_push(&p, (x))

#define ARG_REG()                                                              \
  do {                                                                         \
    char reg = gen->current.args.items[0].span.literal[0];                     \
    if (gen->current.args.count > 0)                                           \
      shift(gen->current.args.items, gen->current.args.count);                 \
    if (reg >= 'A' && reg <= 'Z')                                              \
      da_push(&p, reg - 'A');                                                  \
    else {                                                                     \
      fprintf(stderr, "error: `%c` is not a valid register\n", reg);           \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define ARG_NUM()                                                              \
  do {                                                                         \
    da_push(&p, strtoull(gen->current.args.items[0].span.literal, NULL, 10));  \
    if (gen->current.args.count > 0)                                           \
      shift(gen->current.args.items, gen->current.args.count);                 \
  } while (0)

// NOTE: We are currently using the Z register as temporary storage
#define OP_BIN(x)                                                              \
  do {                                                                         \
    if (gen->current.args.items[1].kind == TK_INT_LIT) {                       \
      push(OP_PUSH_REG);                                                       \
      push(REG_Z);                                                             \
      push(OP_LOAD);                                                           \
      push(REG_Z);                                                             \
      da_push(&p,                                                              \
              strtoull(gen->current.args.items[1].span.literal, NULL, 10));    \
      push((x));                                                               \
      ARG_REG();                                                               \
      push(REG_Z);                                                             \
      gen_consume(gen);                                                        \
      push(OP_POP_REG);                                                        \
      push(REG_Z);                                                             \
    } else {                                                                   \
      push((x));                                                               \
      ARG_REG();                                                               \
      ARG_REG();                                                               \
    }                                                                          \
  } while (0)

Program gen_parse_expr(Gen *gen) {
  Program p = {0};
  if (gen->pos == 0)
    gen_consume(gen);
  switch (gen->current.kind) {
  case EK_PUSH: {
    Token arg = gen->current.args.items[0];
    switch (arg.kind) {
    case TK_INT_LIT: {
      push(OP_PUSH);
      ARG_NUM();
      break;
    }
    case TK_LIT: {
      push(OP_PUSH_REG);
      ARG_REG();
      break;
    }
    default:
      todo("ek_push: arg");
    }
    break;
  }
  case EK_POP: {
    push(OP_POP_REG);
    ARG_REG();
    break;
  }
  case EK_LOAD: {
    Tokens args = gen->current.args;
    switch (args.items[1].kind) {
    case TK_INT_LIT: {
      push(OP_LOAD);
      ARG_REG();
      ARG_NUM();
      break;
    }
    case TK_LIT: {
      push(OP_LOAD_REG);
      ARG_REG();
      ARG_REG();
      break;
    }
    default:
      abort();
    }
    break;
  }
  case EK_ADD: {
    OP_BIN(OP_ADD);
    break;
  }
  case EK_SUB: {
    OP_BIN(OP_SUB);
    break;
  }
  case EK_MULT: {
    OP_BIN(OP_MULT);
    break;
  }
  case EK_DIV: {
    OP_BIN(OP_DIV);
    break;
  }
  case EK_CMP: {
    OP_BIN(OP_CMP);
    break;
  }
  case EK_JMP: {
    push(OP_JMP);
    ARG_NUM();
    break;
  }
  case EK_JE: {
    push(OP_JE);
    ARG_NUM();
    break;
  }
  case EK_JGE: {
    push(OP_JGE);
    ARG_NUM();
    break;
  }
  case EK_JG: {
    push(OP_JG);
    ARG_NUM();
    break;
  }
  case EK_JLE: {
    push(OP_JLE);
    ARG_NUM();
    break;
  }
  case EK_JL: {
    push(OP_JL);
    ARG_NUM();
    break;
  }
  case EK_SYSCALL: {
    push(OP_SYSCALL);
    break;
  }
  case EK_NOP: {
    push(OP_NOP);
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
