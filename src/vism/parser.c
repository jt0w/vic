#include "parser.h"

bool par_raw_consume(Parser *parser) {
  parser->current = parser->tokens.items[parser->pos++];
  return parser->current.kind == TK_WHITESPACE;
  ;
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
  da_push_buf(&expected, "[");
  va_list args;
  va_start(args, parser);
  for (TokenKind tk = va_arg(args, TokenKind); tk != END;
  tk = va_arg(args, TokenKind)) {
    da_push_buf(&expected, print_token((Token){.kind = tk}));
    if (parser->current.kind == tk) { // Compare with the current token
      t = parser->current;
      par_consume(parser); // Consume the token if it matches
      va_end(args);
      return t;
    }
  }

  da_push_buf(&expected, "]");
  fprintf(stderr, "%s:%zu:%zu: error: expected '%s' but got '%s'\n",
          parser->file, parser->current.span.pos.row,
          parser->current.span.pos.col, expected.items,
          print_token(parser->current));
  exit(1);
}

#define BIN_OP()                                                             \
do {                                                                         \
  par_consume(parser);                                                       \
  Token lhs = par_expect(parser, TK_INT_LIT, TK_LIT, END);                   \
  Token rhs = par_expect(parser, TK_INT_LIT, TK_LIT, END);                   \
  da_push(&expr.args, lhs);                                                  \
  da_push(&expr.args, rhs);                                                  \
  break;                                                                     \
} while (0);

#define JUMP_OP()                                                            \
do {                                                                         \
  par_consume(parser);                                                       \
  Token arg = par_expect(parser, TK_INT_LIT, /*TK_LIT TODO: Labels*/ END);   \
  da_push(&expr.args, arg);                                                  \
  break;                                                                     \
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
      Token arg = par_expect(parser, TK_INT_LIT, TK_LIT, END);
      da_push(&expr.args, arg);
      break;
    }

    case TK_POP: {
      expr.kind = EK_POP;
      par_consume(parser);
      Token arg = par_expect(parser, TK_INT_LIT, TK_LIT, END);
      da_push(&expr.args, arg);
      break;
    }

    case TK_LOAD: {
      expr.kind = EK_LOAD;
      par_consume(parser);
      Token arg = par_expect(parser, TK_LIT, END);
      da_push(&expr.args, arg);
      arg = par_expect(parser, TK_INT_LIT, TK_LIT, END);
      da_push(&expr.args, arg);
      break;
    }
    case TK_ADD: {
      expr.kind = EK_ADD;
      BIN_OP()
    }
    case TK_SUB: {
      expr.kind = EK_SUB;
      BIN_OP()
    }
    case TK_MULT: {
      expr.kind = EK_MULT;
      BIN_OP()
    }
    case TK_DIV: {
      expr.kind = EK_DIV;
      BIN_OP()
    }
    case TK_CMP: {
      expr.kind = EK_CMP;
      par_consume(parser);
      Token lhs = par_expect(parser, TK_LIT, END);
      Token rhs = par_expect(parser, TK_LIT, END);
      da_push(&expr.args, lhs);
      da_push(&expr.args, rhs);
      break;
    }
    case TK_JMP: {
      expr.kind = EK_JMP;
      JUMP_OP()
    }
    case TK_JE: {
      expr.kind = EK_JE;
      JUMP_OP()
    }
    case TK_JG: {
      expr.kind = EK_JG;
      JUMP_OP()
    }
    case TK_JLE: {
      expr.kind = EK_JLE;
      JUMP_OP()
    }
    case TK_JL: {
      expr.kind = EK_JL;
      JUMP_OP()
    }
    case TK_JGE: {
      expr.kind = EK_JGE;
      JUMP_OP()
    }
    default: {
      todo("not implemented");
    }
  }
  return expr;
}
