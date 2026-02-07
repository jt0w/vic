#include "parser.h"
#include <debug.h>

Token par_consume(Parser *parser) {
  parser->current = parser->tokens.items[parser->pos++];
  return parser->current;
}

#define END -1

bool par_expect(Parser *parser, ...) {
  bool result = true;
  StringBuilder expected = {0};

  StringBuilder sb = {0};
  {
    da_push(&expected, '[');
    va_list args;
    va_start(args, parser);
    for (TokenKind tk = va_arg(args, TokenKind); (int)tk != END;
        tk = va_arg(args, TokenKind)) {
      da_push_buf(&expected,
          temp_sprintf("%s ,", token_name((Token){.kind = tk})));
      if (parser->current.kind == tk) {
        va_end(args);
        goto end;
      }
    }
    va_end(args);

    da_push(&expected, '\b');
    da_push(&expected, '\b');
    da_push(&expected, ']');
    da_push(&expected, '\0');

    if (parser->current.kind == TK_ERR)
      da_push_buf(&sb, "Nothing");
    else
      da_push_buf(&sb, token_name(parser->current));
    da_push(&sb, '\0');
    fprintf(stderr, "%s:%zu:%zu: error: expected (one of) '%s' but got %s \n",
        parser->file, parser->current.span.pos.row,
        parser->current.span.pos.col, expected.items, sb.items);
    goto fail;
  }

end:
  if (expected.items != NULL) da_free(expected);
  if (sb.items != NULL) da_free(sb);
  temp_reset();
  return result;
fail:
  breakpoint();
  result = false;
  goto end;
}

#define JUMP_OP()                                  \
  do {                                             \
    par_consume(parser);                           \
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, END);  \
    da_push(&expr->args, arg);                     \
  } while (0);


#define EXPECT(...)                                \
  if (!par_expect(parser, __VA_ARGS__)) goto fail; \
  par_consume(parser);                             \

#define EXPECT_CAPTURE(name, ...)                  \
  if (!par_expect(parser, __VA_ARGS__)) goto fail; \
  Token name = parser->current;                    \
  par_consume(parser);                             \

bool parse_expr(Parser *parser, Expr *expr) {
  bool result = true;
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
    goto fail;
  }

  case TK_INT_LIT: {
    fprintf(
        stderr,
        "%s:%zu:%zu: error: cannot start an expression with an int literal\n",
        parser->file, parser->current.span.pos.row,
        parser->current.span.pos.col);
    goto fail;
  }

  case TK_LIT: {
    da_push(&expr->args, parser->current);
    par_consume(parser);
    EXPECT(TK_COLON, END);
    expr->kind = EK_LABEL_DEF;
    break;
  }

  case TK_PUSH: {
    expr->kind = EK_PUSH;
    par_consume(parser);
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, TK_CHAR, END);
    da_push(&expr->args, arg);
    break;
  }

  case TK_POP: {
    expr->kind = EK_POP;
    par_consume(parser);
    break;
  }

  case TK_DUP: {
    expr->kind = EK_DUP;
    par_consume(parser);
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, END);
    da_push(&expr->args, arg);
    break;
  }

  case TK_SWAP: {
    expr->kind = EK_SWAP;
    par_consume(parser);
    EXPECT_CAPTURE(arg, TK_INT_LIT, END);
    da_push(&expr->args, arg);
    break;
  }

  case TK_ADD: {
    expr->kind = EK_ADD;
    par_consume(parser);
    break;
  }

  case TK_SUB: {
    expr->kind = EK_SUB;
    par_consume(parser);
    break;
  }
  case TK_MULT: {
    expr->kind = EK_MULT;
    par_consume(parser);
    break;
  }

  case TK_DIV: {
    expr->kind = EK_DIV;
    par_consume(parser);
    break;
  }
  case TK_EQ: {
    expr->kind = EK_EQ;
    par_consume(parser);
    break;
  }
  case TK_JMP: {
    expr->kind = EK_JMP;
    JUMP_OP();
    break;
  }
  case TK_JZ: {
    expr->kind = EK_JZ;
    JUMP_OP();
    break;
  }
  case TK_JNZ: {
    expr->kind = EK_JNZ;
    JUMP_OP();
    break;
  }
  case TK_NOP: {
    expr->kind = EK_NOP;
    par_consume(parser);
    break;
  }
  case TK_ALLOC: {
    par_consume(parser);
    expr->kind = EK_ALLOC;
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, END);
    da_push(&expr->args, arg);
    break;
  }
  case TK_WRITE: {
    par_consume(parser);
    expr->kind = EK_WRITE;
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, END);
    da_push(&expr->args, arg);
    break;
  }
  case TK_READ: {
    par_consume(parser);
    expr->kind = EK_READ;
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, END);
    da_push(&expr->args, arg);
    break;
  }
  case TK_CALL: {
    par_consume(parser);
    expr->kind = EK_CALL;
    EXPECT_CAPTURE(arg, TK_INT_LIT, TK_LIT, END);
    da_push(&expr->args, arg);
    break;
  }
  case TK_RET: {
    par_consume(parser);
    expr->kind = EK_RET;
    break;
  }
  case TK_NATIVE: {
    par_consume(parser);
    expr->kind = EK_NATIVE;
    Token name = parser->current;
    par_consume(parser);
    da_push(&expr->args, name);
    break;
  }
  case TK_PERCENT: {
    par_consume(parser);
    if (parser->current.kind == TK_DEF) {
      par_consume(parser);
      expr->kind = EK_VAR_DEF;
      EXPECT_CAPTURE(name, TK_LIT, END);
      EXPECT_CAPTURE(value, TK_INT_LIT, TK_LIT, END);
      da_push(&expr->args, name);
      da_push(&expr->args, value);
    } else if (parser->current.kind == TK_NATIVE) {
      par_consume(parser);
      expr->kind = EK_NATIVE_DEF;
      Token name = parser->current;
      par_consume(parser);
      da_push(&expr->args, name);
    } else if (parser->current.kind == TK_USE) {
      par_consume(parser);
      expr->kind = EK_USE;
      EXPECT_CAPTURE(filename, TK_STRING, END);
      da_push(&expr->args, filename);
    } else {
      fprintln(stderr,
          "%s:%zu:%zu: error: Unexpected token: `%s`",
          parser->file, parser->current.span.pos.row, parser->current.span.pos.col, parser->current.span.literal);
      goto fail;
    }
    break;
  }
  case TK_SEMICOLON: {
    size_t row = parser->current.span.pos.row;
    while (parser->current.span.pos.row == row &&
           parser->pos < parser->tokens.count) {
      par_consume(parser);
    }
    return parse_expr(parser, expr);
  }
  case TK_COLON: {
    fprintln(stderr,
            "%s:%zu:%zu: error: cannot start an expression with a colon",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col);
    goto fail;
  }
  case TK_USE:
  case TK_DEF: {
    fprintln(stderr,
            "%s:%zu:%zu: error: unexpected token `%s`",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col, parser->current.span.literal);
    goto fail;
  }
  case TK_CHAR: {
    fprintln(stderr,
            "%s:%zu:%zu: error: unexpected char literal",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col);
    goto fail;
  }
  case TK_STRING: {
    fprintln(stderr,
            "%s:%zu:%zu: error: unexpected string literal",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col);
    goto fail;
  }
  case TK_EOF: {

    fprintln(stderr,
            "%s:%zu:%zu: error: unexpected eof",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col);
    goto fail;
  }
  default: {
    fprintln(stderr,
            "%s:%zu:%zu: error: tokenkind `%s` not covered in parser",
            parser->file, parser->current.span.pos.row,
            parser->current.span.pos.col, token_name(parser->current));
    goto fail;
  }
  }
end:
  return result;
fail:
  result = false;
  goto end;
}
