#include "gen.h"
#include <debug.h>
#include <vm.h>

void gen_adv(Gen *g) {
  g->s = *(++g->stmts.items);
  g->stmts.count -= 1;
}

#define push(x) da_push(p, (x))

Program gen_gen(Gen *g) {
  Program p = {0};
  Stmt pre_gen[] = {
	{
      .kind = SK_FUNCALL,
      .funcall =
          {
            .fun_name = "main",
            .args = {0},
          },
	},
	{
	  .kind = SK_FUNCALL,
	  .funcall =
          {
            .fun_name = "exit",
			.args = {0},
		  },
	}
  };

  da_push(&pre_gen[1].funcall.args, ((Expr){.kind = EK_U64, .u64 = 0}));
  size_t pre_gen_c = sizeof(pre_gen) / sizeof(pre_gen[0]);

  for (size_t i = 0; i < pre_gen_c; ++i) {
	gen_stmt(g, &p, pre_gen[i]);
  }

  while (g->stmts.count >= 1) {
    gen_stmt(g, &p, g->s);
    gen_adv(g);
  }
 
  da_foreach(UnresolvedJmp, jmp, g->unresolvedJmps) {
  // jmp->funcall_name
  bool found = false;
  da_foreach(Function, f, g->funs) {
  if (strcmp(f->name, jmp->funcall_name) == 0) {
    found = true;
    assert(p.items[jmp->pos].opcode == OP_JMP);
    p.items[jmp->pos].operand = WORD_U64(f->pos + 1);
  }
}
  if (!found) {
    log(ERROR, "Could not find function %s", jmp->funcall_name);
    exit(1);
  }
}

  return p;
}

void gen_stmt(Gen *g, Program *p, Stmt s) {
  switch (s.kind) {
  case SK_DECL: {
    gen_decl(g, p, s);
  } break;
  case SK_FUNCALL: {
    gen_funcall(g, p, s);
  } break;
  case SK_IF_COND: {
	gen_if_cond(g, p, s);
  } break;
  default:
    log(ERROR, "unreachable: gen_stmt");
    abort();
  }
}

void gen_decl(Gen *g, Program *p, Stmt s) {
  switch (s.decl.kind) {
  case DK_FUNC: {
    Function f = {0};
    f.name = strdup(s.decl.fun.name);
    f.pos = p->count - 1;
    da_foreach(Stmt, bs, s.decl.fun.body) { gen_stmt(g, p, *bs); }
    push(INST_RET);
    da_push(&g->funs, f);
  } break;
  default:
    log(ERROR, "unreachable: gen_decl");
    abort();
  }
}

void gen_if_cond(Gen *g, Program *p, Stmt s) {
  gen_expr(g, p, (Expr){.kind = EK_COND, .cond = s.ifCond.cond});

  push(INST_JNZ(WORD_U64(p->count + 1)));
  size_t jmp_index = p->count;
  push(INST_JMP(WORD_U64(0)));
  da_foreach (Stmt, st, s.ifCond.body) {
    gen_stmt(g, p, *st);
  }
  p->items[jmp_index].operand = WORD_U64(p->count);
}

int find_native_ptr(Gen *g, VM_Internal n) {
  for (size_t i = 0; i < g->natives_c; ++i) {
    if (g->natives[i].ptr == n) {
      return i;
    }
  }
  return -1;
}


void gen_funcall(Gen *g, Program *p, Stmt s) {
  UnresolvedJmp j = {0};
  j.funcall_name = strdup(s.funcall.fun_name);

  da_foreach(Expr, e, s.funcall.args) { gen_expr(g, p, *e); }

  if (strcmp(j.funcall_name, "write") == 0) {
    int id = find_native_ptr(g, native_write);
    assert(id >= 0);
	push(INST_ALLOC(WORD_U64(1)));
	push(INST_DUP(WORD_U64(0)));
	push(INST_SWAP(WORD_U64(2)));
	push(INST_WRITE(WORD_U64(1)));
	push(INST_PUSH(WORD_U64(1)));
    push(INST_NATIVE(WORD_U64(id)));
  } else if (strcmp(j.funcall_name, "exit") == 0) {
    int id = find_native_ptr(g, native_exit);
    assert(id >= 0);
    push(INST_NATIVE(WORD_U64(id)));
  } else {
    if (p->count == 0)
      push(INST_PUSH(WORD_U64(2)));
    else
      push(INST_PUSH(WORD_U64(p->count+1)));

    push(INST_JMP(WORD_U64(0)));

    j.pos = p->count - 1;
    da_push(&g->unresolvedJmps, j);
  }
}

void gen_expr(Gen *g, Program *p, Expr e) {
  (void)g;
  switch (e.kind) {
  case EK_U64: {
    push(INST_PUSH(WORD_U64(e.u64)));
  } break;
  case EK_CHAR: {
    push(INST_PUSH(WORD_U64((int)e.chr)));
  } break;
  case EK_COND: {
	gen_expr(g, p, *e.cond.lhs);
	gen_expr(g, p, *e.cond.rhs);
	switch (e.cond.op) {
	case COND_OP_EQ: {
	  push(INST_EQ);
	} break;
	default:
	  log(ERROR, "unreachable: e.cond.op");
	  abort();
	}
  } break;
  default:
    log(ERROR, "unreachable: gen_expr");
    abort();
  }
}
