#include "parser.h"

Token par_consume(Parser *parser) {
  parser->current = parser->tokens.items[parser->pos++];
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
                temp_sprintf("%s ,", token_name((Token){.kind = tk})));
    if (parser->current.kind == tk) {
      t = parser->current;
      par_consume(parser);
      va_end(args);
      return t;
    }
  }
  va_end(args);

  da_push(&expected, '\b');
  da_push(&expected, '\b');
  da_push(&expected, ']');
  da_push(&expected, '\0');

  StringBuilder sb = {0};
  if (parser->current.kind == TK_ERR)
    da_push_buf(&sb, "Nothing");
  else
    da_push_buf(&sb, token_name(parser->current));
  da_push(&sb, '\0');
  fprintf(stderr, "%s:%zu:%zu: error: expected (one of) '%s' but got %s \n",
          parser->file, parser->current.span.pos.row,
          parser->current.span.pos.col, expected.items, sb.items);
  exit(1);
}

#define JUMP_OP()                                                              \
  do {                                                                         \
    par_consume(parser);                                                       \
    Token arg = par_expect(parser, TK_INT_LIT, TK_LIT, END);                   \
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
    da_push(&expr.args, parser->current);
    par_consume(parser);
    par_expect(parser, TK_COLON, END);
    expr.kind = EK_LABEL_DEF;
    break;
  }

  case TK_PUSH: {
    expr.kind = EK_PUSH;
    par_consume(parser);
    Token arg = par_expect(parser, TK_INT_LIT, TK_LIT, END);
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
    par_consume(parser);
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
  case TK_ALLOC8: {
    par_consume(parser);
    expr.kind = EK_ALLOC8;
    break;
  }
  case TK_WRITE8: {
    par_consume(parser);
    expr.kind = EK_WRITE8;
    break;
  }
  case TK_PERCENT: {
    par_consume(parser);
    Token t = par_expect(parser, TK_LIT, END);
    if (strcmp(t.as.str, "def") == 0) {
      expr.kind = EK_VAR_DEF;
      da_push(&expr.args, par_expect(parser, TK_LIT, END));
      da_push(&expr.args, par_expect(parser, TK_INT_LIT, END));
    } else {
      fprintln(stderr,
          "%s:%zu:%zu: error: Unexpected literal: `%s`",
          parser->file, t.span.pos.row, t.span.pos.col, t.span.literal);
      exit(1);
    }
    break;
  }
  case TK_SEMICOLON: {
    size_t row = parser->current.span.pos.row;
    while (parser->current.span.pos.row == row &&
           parser->pos < parser->tokens.count) {
      par_consume(parser);
    }
    return parse_expr(parser);
  }
  case TK_COLON: {
    fprintln(stderr,
            "%s:%zu:%zu: error: cannot start an expression with a colon",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col);
    exit(1);
  }
  default: {
    fprintln(stderr,
            "%s:%zu:%zu: error: tokenkind `%s` not covered in parser",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col, token_name(parser->current));
    abort();
  }
  }
  return expr;
}
