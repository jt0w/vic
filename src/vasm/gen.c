#include "gen.h"
#include <inttypes.h>

Expr gen_consume(Gen *gen) {
  gen->current = gen->exprs.items[gen->pos++];
  return gen->current;
}

#define push(x) da_push(&p, (x))

Var find_var_by_name(Gen *gen, const char *name) {
  for (size_t i = 0; i < gen->vars.count; ++i) {
    if (strcmp(name, gen->vars.items[i].name) == 0) {
      return gen->vars.items[i];
    }
  }

  fprintln(stderr, "error: Variable `%s` not found", name);
  exit(1);
}

Program gen_generate(Gen *gen) {
  Program p = {0};
  while (gen->pos <= gen->exprs.count) {
    if (gen->pos == 0)
      gen_consume(gen);
    switch (gen->current.kind) {
    case EK_LABEL_DEF: {
      da_push(&gen->labels, ((Label) {
                                .pos = {p.count},
                                .name = gen->current.args.items[0].as.str,
                            }));
      break;
    }
    case EK_VAR_DEF: {
      assert(gen->current.args.count == 2);
      Token name = gen->current.args.items[0];
      Token value = gen->current.args.items[1];
      int found_index = -1;
      for (size_t i = 0; i < gen->vars.count; ++i) {
        if (strcmp(name.as.str, gen->vars.items[i].name) == 0) {
          found_index = (int)i;
        }
      }
      if (found_index != -1) {
        gen->vars.items[found_index].value = value.as.num;
      } else {
        da_push(&gen->vars,
                ((Var){.name = name.as.str, .value = value.as.num}));
      }
      break;
    }
    case EK_PUSH: {
      assert(gen->current.args.count == 1);
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_INT_LIT) {
        push(INST_PUSH(arg.as.num));
      } else if (arg.kind == TK_LIT) {
        push(INST_PUSH(find_var_by_name(gen, arg.as.str).value));
      } else {
        assert(!"invalid push operand");
      }
      break;
    }
    case EK_POP: {
      push(INST_POP);
      break;
    }
    case EK_DUP: {
      assert(gen->current.args.count == 1);
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
      assert(gen->current.args.count == 1);
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_LIT) {
        da_push(&gen->unresolved_jumps, ((UnresolvedJump){
                                            .expr_pos = gen->pos - 1,
                                            .program_pos = p.count,
                                        }));
        push(INST_JMP(WORD_U64(0)));
      } else if (arg.kind == TK_INT_LIT) {
        push(INST_JMP(arg.as.num));
      }
      break;
    }
    case EK_JZ: {
      assert(gen->current.args.count == 1);
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_LIT) {
        da_push(&gen->unresolved_jumps, ((UnresolvedJump){
                                            .expr_pos = gen->pos - 1,
                                            .program_pos = p.count,
                                        }));
        push(INST_JZ(WORD_U64(0)));
      } else if (arg.kind == TK_INT_LIT) {
        push(INST_JZ(arg.as.num));
      }

      break;
    }
    case EK_JNZ: {
      assert(gen->current.args.count == 1);
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_LIT) {
        da_push(&gen->unresolved_jumps, ((UnresolvedJump){
                                            .expr_pos = gen->pos - 1,
                                            .program_pos = p.count,
                                        }));
        push(INST_JNZ(WORD_U64(0)));
      } else if (arg.kind == TK_INT_LIT) {
        push(INST_JNZ(arg.as.num));
      }
      break;
    }
    case EK_NOP: {
      push(INST_NOP);
      break;
    }
    case EK_ALLOC: {
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_INT_LIT) {
        push(INST_ALLOC(arg.as.num));
      } else if (gen->current.args.items[0].kind == TK_LIT) {
        push(INST_ALLOC(find_var_by_name(gen, arg.as.str).value));
      }
      break;
    }
    case EK_WRITE: {
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_INT_LIT) {
        push(INST_WRITE(arg.as.num));
      } else if (gen->current.args.items[0].kind == TK_LIT) {
        push(INST_WRITE(find_var_by_name(gen, arg.as.str).value));
      }
      break;
    }

    case EK_INT:
    case EK_LIT: {
      fprintf(
          stderr,
          "FATAL ERROR: This should never happen please contact a maitainer "
          "(%s:%d)",
          __FILE__, __LINE__);
      exit(1);
    }
    }
    gen_consume(gen);
  }

  for (size_t i = 0; i < gen->unresolved_jumps.count; ++i) {
    UnresolvedJump jmp = gen->unresolved_jumps.items[i];
    Expr e = gen->exprs.items[jmp.expr_pos];
    assert(e.kind == EK_JMP || e.kind == EK_JNZ || e.kind == EK_JZ);
    const char *name = e.args.items[0].as.str;
    bool found = false;
    for (size_t j = 0; j < gen->labels.count; ++j) {
      if (strcmp(name, gen->labels.items[j].name) == 0) {
        p.items[jmp.program_pos].operand = gen->labels.items[j].pos;
        found = true;
      }
    }
    if (!found) {
      log(ERROR, "Unresolved jmp %s", name);
      exit(1);
    }
  }
  return p;
}
