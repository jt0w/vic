#ifndef GEN_H
#define GEN_H
#include "parser.h"
#include <vm.h>

typedef struct {
  VM_Internal ptr;
  StringBuilder name;
} Native;

typedef struct {
  const char *name;
  size_t pos;
} Function;

DA_STRUCT(Function, Functions)

typedef struct {
  size_t pos;
  union {
    const char *funcall_name;
  };
} UnresolvedJmp;

DA_STRUCT(UnresolvedJmp, UnresolvedJmps)

typedef struct {
  Stmts stmts;
  Stmt s;

  Functions funs;
  UnresolvedJmps unresolvedJmps;
  Native *natives;
  size_t natives_c;
} Gen;

Program gen_gen(Gen *g);

void gen_stmt(Gen *g, Program *p, Stmt s);
void gen_decl(Gen *g, Program *p, Stmt s);
void gen_if_cond(Gen *g, Program *p, Stmt s);
void gen_expr(Gen *g, Program *p, Expr e);
void gen_funcall(Gen *g, Program *p, Stmt s);
#endif // endif GEN_H
