#include "gen.h"
#include "common.h"
#include <debug.h>
#include <inttypes.h>

Expr gen_consume(Gen *gen) {
  gen->current = gen->exprs.items[gen->pos++];
  return gen->current;
}

#define push(x) da_push(p, (x))

Var *find_var_by_name(Gen *gen, const char *name) {
  for (size_t i = 0; i < gen->vars.count; ++i) {
    if (strcmp(name, gen->vars.items[i].name) == 0) {
      return &gen->vars.items[i];
    }
  }

  fprintln(stderr, "error: Variable `%s` not found", name);
  exit(1);
}

int find_native_id_by_name(Gen *gen, const char *name) {
  for (size_t i = 0; i < gen->natives.count; ++i) {
    if (strcmp(name, gen->natives.items[i].items) == 0) {
      return i;
    }
  }

  fprintln(stderr, "error: Native `%s` not found", name);
  return -1;
}

bool gen_generate(Gen *gen, Program *p) {
  bool res = true;
  while (gen->pos <= gen->exprs.count) {
    if (gen->pos == 0)
      gen_consume(gen);
    switch (gen->current.kind) {
    case EK_USE: {
      char *file = gen->current.args.items[0].span.literal;
      Gen gen2 = {0};
      char *root = strdup(gen->file);
      // https://stackoverflow.com/questions/11270127/string-handling-in-ansi-c
      char *last_slash = strrchr(root, '/');
      if (last_slash != NULL) {
        *last_slash = '\0';
      }
      char *file_path = temp_sprintf("%s/%s", root, file);
      if (!translate_file(file_path, &gen2, p)) goto fail;

	  breakpoint();
      da_push_mult(&gen->labels, gen2.labels.items, gen2.labels.count);
      da_push_mult(&gen->natives, gen2.natives.items, gen2.natives.count);
      break;
    }
    case EK_LABEL_DEF: {
      da_push(&gen->labels, ((Label){
                                .pos = {p->count},
								// TODO: mem leak
                                .name = strdup(gen->current.args.items[0].as.str),
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
    case EK_NATIVE_DEF: {
      Token name = gen->current.args.items[0];
	  // TODO: mem leak
      da_push(&gen->natives, sb_from_string(strdup(name.span.literal)));
      break;
    }
    case EK_NATIVE: {
      Token name = gen->current.args.items[0];
      int id = {find_native_id_by_name(gen, name.span.literal)};
	  if (id < 0) goto fail;
      da_push(p, ((Inst){.opcode = OP_NATIVE, .operand = WORD_U64(id)}));
      break;
    }
    case EK_PUSH: {
      assert(gen->current.args.count == 1);
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_INT_LIT) {
        push(INST_PUSH(arg.as.num));
      } else if (arg.kind == TK_LIT) {
		Var *v = find_var_by_name(gen, arg.as.str);
		if (v == NULL) goto fail;
        push(INST_PUSH(v->value));
      } else if (arg.kind == TK_CHAR) {
        push(INST_PUSH(WORD_U64((int)arg.as.chr)));
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
    case EK_SWAP: {
      assert(gen->current.args.count == 1);
      Token arg = gen->current.args.items[0];
      assert(arg.kind == TK_INT_LIT);
      push(INST_SWAP(arg.as.num));
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
                                            .program_pos = p->count,
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
                                            .program_pos = p->count,
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
                                            .program_pos = p->count,
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
		Var *v = find_var_by_name(gen, arg.as.str);
		if (v == NULL) goto fail;
        push(INST_ALLOC(v->value));
      }
      break;
    }
    case EK_WRITE: {
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_INT_LIT) {
        push(INST_WRITE(arg.as.num));
      } else if (gen->current.args.items[0].kind == TK_LIT) {
		Var *v = find_var_by_name(gen, arg.as.str);
		if (v == NULL) goto fail;
        push(INST_WRITE(v->value));
      }
      break;
    }
    case EK_READ: {
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_INT_LIT) {
        push(INST_READ(arg.as.num));
      } else if (gen->current.args.items[0].kind == TK_LIT) {
		Var *v = find_var_by_name(gen, arg.as.str);
		if (v == NULL) goto fail;
        push(INST_READ(v->value));
      }
      break;
    }
    case EK_CALL: {
      push(INST_PUSH(WORD_U64(p->count + 1)));
      Token arg = gen->current.args.items[0];
      if (arg.kind == TK_LIT) {
        da_push(&gen->unresolved_jumps, ((UnresolvedJump){
                                            .expr_pos = gen->pos - 1,
                                            .program_pos = p->count,
                                        }));
        push(INST_JMP(WORD_U64(0)));
      } else if (arg.kind == TK_INT_LIT) {
        push(INST_JMP(arg.as.num));
      }
      break;
    }
    case EK_RET: {
      push(INST_RET);
      break;
    }

    case EK_INT:
    case EK_LIT: {
      fprintf(
          stderr,
          "FATAL ERROR: This should never happen please contact a maitainer "
          "(%s:%d)",
          __FILE__, __LINE__);
      goto fail;
    }
    }
    gen_consume(gen);
  }


  breakpoint();
  for (size_t i = 0; i < gen->unresolved_jumps.count; ++i) {
    UnresolvedJump jmp = gen->unresolved_jumps.items[i];
    Expr e = gen->exprs.items[jmp.expr_pos];
    assert(e.kind == EK_JMP || e.kind == EK_JNZ || e.kind == EK_JZ ||
           e.kind == EK_CALL);
    const char *name = e.args.items[0].as.str;
    bool found = false;
    for (size_t j = 0; j < gen->labels.count; ++j) {
      if (strcmp(name, gen->labels.items[j].name) == 0) {
        p->items[jmp.program_pos].operand = gen->labels.items[j].pos;
        found = true;
      }
    }
    if (!found) {
      log(ERROR, "Unresolved jmp %s", name);
      goto fail;
    }
  }

 end:
  return res;
 fail:
  res = false;
  goto end;
}
