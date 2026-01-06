#include "parser.h"

bool par_raw_consume(Parser *parser) {
  parser->current = parser->tokens.items[parser->pos++];
  return parser->current.kind == TK_WHITESPACE;
}

Token par_consume(Parser *parser) {
  while (par_raw_consume(parser)) {
  }
  return parser->current;
}

#define END -1

Token par_expect(Parser *parser, ...) {
  Token t;
  StringBuilder expected = {0};
  da_push(&expected, '[');
  va_list args;
  va_start(args, parser);
  for (TokenKind tk = va_arg(args, TokenKind); (int)tk != END;
       tk = va_arg(args, TokenKind)) {
    da_push_buf(&expected,
                temp_sprintf("%s ,", print_token((Token){.kind = tk}, false)));
    if (parser->current.kind == tk) {
      t = parser->current;
      par_consume(parser);
      va_end(args);
      return t;
    }
  }

  da_push(&expected, '\b');
  da_push(&expected, '\b');
  da_push(&expected, ']');
  da_push(&expected, '\0');

  StringBuilder sb = {0};
  if (parser->current.kind == TK_ERR)
    da_push_buf(&sb, "Nothing");
  else
    da_push_buf(&sb, print_token(parser->current, false));
  fprintf(stderr, "%s:%zu:%zu: error: expected (one of) '%s' but got %s \n",
          parser->file, parser->current.span.pos.row,
          parser->current.span.pos.col, expected.items, sb.items);
  exit(1);
}

#define BIN_OP()                                                               \
  do {                                                                         \
    par_consume(parser);                                                       \
    Token lhs = par_expect(parser, TK_INT_LIT, TK_LIT, END);                   \
    Token rhs = par_expect(parser, TK_INT_LIT, TK_LIT, END);                   \
    da_push(&expr.args, lhs);                                                  \
    da_push(&expr.args, rhs);                                                  \
  } while (0);

#define JUMP_OP()                                                              \
  do {                                                                         \
    par_consume(parser);                                                       \
    Token arg = par_expect(parser, TK_INT_LIT, /*TK_LIT TODO: Labels*/ END);   \
    da_push(&expr.args, arg);                                                  \
  } while (0);

Expr parse_expr(Parser *parser) {
  Expr expr = {0};
  if (parser->pos == 0)
    par_consume(parser);
  switch (parser->current.kind) {
  case (TK_ERR): {
    if (parser->current.span.literal == NULL)
      fprintf(stderr, "%s:%zu:%zu: error: expected token\n", parser->file,
              parser->current.span.pos.row, parser->current.span.pos.col);
    else
      fprintf(stderr, "%s:%zu:%zu: error: unexpected token `%s`\n",
              parser->file, parser->current.span.pos.row,
              parser->current.span.pos.col, parser->current.span.literal);
    exit(1);
  }

  case TK_INT_LIT: {
    fprintf(
        stderr,
        "%s:%zu:%zu: error: cannot start an expression with an int literal\n",
        parser->file, parser->current.span.pos.row,
        parser->current.span.pos.col);
    exit(1);
  }

  case TK_LIT: {
    fprintf(stderr,
            "%s:%zu:%zu: error: cannot start an expression with a literal\n",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col);
    exit(1);
  }

  case TK_PUSH: {
    expr.kind = EK_PUSH;
    par_consume(parser);
    Token arg = par_expect(parser, TK_INT_LIT,  END);
    da_push(&expr.args, arg);
    break;
  }

  case TK_POP: {
    expr.kind = EK_POP;
    par_consume(parser);
    break;
  }

  case TK_DUP: {
    expr.kind = EK_DUP;
    par_consume(parser);
    Token arg = par_expect(parser, TK_INT_LIT, END);
    da_push(&expr.args, arg);
    break;
  }

  case TK_ADD: {
    expr.kind = EK_ADD;
    par_consume(parser);
    break;
  }

  case TK_SUB: {
    expr.kind = EK_SUB;
    par_consume(parser);
    break;
  }
  case TK_MULT: {
    expr.kind = EK_MULT;
    par_consume(parser);
    break;
  }

  case TK_DIV: {
    expr.kind = EK_DIV;
    par_consume(parser);
    break;
  }
  case TK_EQ: {
    expr.kind = EK_EQ;
    break;
  }
  case TK_JMP: {
    expr.kind = EK_JMP;
    JUMP_OP();
    break;
  }
  case TK_JZ: {
    expr.kind = EK_JZ;
    JUMP_OP();
    break;
  }
  case TK_JNZ: {
    expr.kind = EK_JNZ;
    JUMP_OP();
    break;
  }
  case TK_NOP: {
    expr.kind = EK_NOP;
    par_consume(parser);
    break;
  }
  case TK_WHITESPACE: {
    // Should be filtered out by consume
    abort();
  }
  }
  return expr;
}
