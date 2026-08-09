#include "parser.h"
#include <debug.h>

void par_adv(Parser *p) {
  p->t = *(++p->tks.items);
  p->tks.count -= 1;
}

bool eq(Token t, const char *name) { return strcmp(t.span.literal, name) == 0; }

void par_skip(Parser *p, const char *name) {
  if (!eq(p->t, name)) {
    printf("ERROR: expected %s but got %s\n", name, p->t.str);
    exit(1);
  }
  par_adv(p);
}

char *skip_ident(Parser *p) {
  if (p->t.kind != TK_IDENT) {
    printf("ERROR: expected identifier but got %s\n", p->t.str);
    exit(1);
  }
  char *s = p->t.str;
  par_adv(p);
  return s;
}

char *skip_str(Parser *p) {
  if (p->t.kind != TK_STRING) {
    printf("ERROR: expected string but got %s\n", p->t.str);
    exit(1);
  }
  char *s = p->t.str;
  par_adv(p);
  return s;
}

Expr parse_expr(Parser *p) {
  Expr e = {0};
  switch (p->t.kind) {
  case TK_NUM: {
    e.kind = EK_U64;
    e.u64 = p->t.u64;
  } break;
  case TK_CHAR: {
    e.kind = EK_CHAR;
    e.chr = p->t.chr;
  } break;
  case TK_IDENT:
  case TK_PUNCT:
  case TK_KEYWORD:
  case TK_STRING:
  case TK_EOF:
  default:
    log(ERROR, "Expression cannot start with %s", p->t.span.literal);
    exit(1);
  };
  par_adv(p);
  return e;
}

FunctionDecl parse_fun(Parser *p) {
  FunctionDecl f = {0};
  par_skip(p, "func");
  f.name = strdup(skip_ident(p));
  par_skip(p, "(");
  par_skip(p, ")");
  par_skip(p, "{");
  while (!eq(p->t, "}")) {
    Stmt s = parse_stmt(p);
    da_push(&f.body, s);
  }
  par_skip(p, "}");

  return f;
}

Stmt parse_stmt(Parser *p) {
  Stmt s = {0};
  if (p->tks.items[1].kind != TK_EOF) {
    Token next = p->tks.items[1];
    if (eq(p->t, "func")) {
      s.kind = SK_DECL;
      s.decl.kind = DK_FUNC;
      s.decl.fun = parse_fun(p);
    } else if (eq(next, "(")) {
      s.kind = SK_FUNCALL;
      s.funcall = parse_funcall(p);
    } else {
      log(ERROR, "unexpected token `%s`", p->t.span.literal);
      exit(1);
    }
  }
  return s;
}

Funcall parse_funcall(Parser *p) {
  Funcall fc = {0};
  // NOTE: mem leak
  fc.fun_name = strdup(skip_ident(p));
  par_skip(p, "(");
  while (!eq(p->t, ")")) {
    da_push(&fc.args, parse_expr(p));
    if (eq(p->t, ","))
      par_adv(p);
    else
      break;
  }
  par_skip(p, ")");
  return fc;
}
