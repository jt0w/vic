#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"

typedef struct Stmt Stmt;
DA_STRUCT(Stmt, Stmts)
typedef struct Expr Expr;
DA_STRUCT(Expr, Exprs)
typedef struct Decl Decl;
typedef struct Funcall Funcall;
typedef struct FunctionDecl FunctionDecl;
DA_STRUCT(FunctionDecl, FunctionDecls)
typedef struct Scope Scope;

typedef enum {
  EK_U64,
  EK_CHAR,
} ExprKind;

struct Expr {
  ExprKind kind;
  union {
    uint64_t u64;
    char chr;
  };
};

typedef enum {
  SK_FUNCALL,
  SK_DECL,
} StmtKind;

struct Funcall {
  char *fun_name;
  Exprs args;
};

typedef enum {
  DK_FUNC,
} DeclKind;

struct FunctionDecl {
  const char *name;
  Scope *scope;
  Stmts body;
};

struct Decl {
  DeclKind kind;
  union {
    FunctionDecl fun;
  };
};

struct Stmt {
  StmtKind kind;
  union {
    Decl decl;
    Funcall funcall;
  };
};

struct Scope {
  // Vars var;
  FunctionDecls *functions;
};

typedef struct {
  Tokens tks;
  Token t;
} Parser;

FunctionDecl parse_fun(Parser *par);
Funcall parse_funcall(Parser *p);
Stmt parse_stmt(Parser *p);
#endif // endif PARSER_H
