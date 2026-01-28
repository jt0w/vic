#ifndef GEN_H
#define GEN_H
#include "parser.h"

typedef struct {
  const char *name;
  Word value;
} Var;

typedef struct {
  Var *items;
  size_t count;
  size_t cap;
} Vars;

typedef struct {
  Word pos;
  const char *name;
} Label;

typedef struct {
  Label *items;
  size_t count;
  size_t cap;
} Labels;

typedef struct {
  size_t expr_pos;
  size_t program_pos;
} UnresolvedJump;

typedef struct {
  UnresolvedJump *items;
  size_t count;
  size_t cap;
} UnresolvedJumps;

typedef struct {
  size_t pos;
  Exprs exprs;
  Expr current;
  VM_NativeNames natives;

  Labels labels;
  UnresolvedJumps unresolved_jumps;
  Vars vars;
} Gen;

Var find_var_by_name(Gen *gen, const char *name);
Expr gen_consume(Gen *gen);
Program gen_generate(Gen *gen);
#endif /* end of include guard: GEN_H */
