#ifndef GEN_H
#define GEN_H
#include "parser.h"


typedef struct {
  size_t pos;
  Exprs exprs;
  Expr current;
} Gen;

Expr gen_consume(Gen *gen);
Program gen_parse_expr(Gen *gen);
#endif /* end of include guard: GEN_H */
