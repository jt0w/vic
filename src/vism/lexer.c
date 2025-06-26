#include "lexer.h"

char *print_token(Token t) {
  for (size_t i = 0;i < sizeof(TK_MAP) / sizeof(TK_MAP[0]);++i) {
    if (TK_MAP[i].kind == t.kind) {
      switch (TK_MAP[i].kind) {
        case TK_INT_LIT: {
          return temp_sprintf("%s = %s", TK_MAP[i].id, t.span.literal);
          break;
        }
        default: {
          return TK_MAP[i].id;
        }
      }
    }
  }
  //NOTE: Without `abort` there is a warning that this function doesn't return anything
  assert(!temp_sprintf("Tokenkind %d is not included in TK_MAP", t));
  abort();
}

char lex_consume(Lexer *lexer) {
  char c = lexer->input[lexer->pos++];
  lexer->current = c;
  return c;
}

#define return(k)                           \
do {                                        \
  t.kind = (k);                             \
  da_push(&sb, '\0');                       \
  t.span.literal = malloc(sizeof(sb.items));\
  t.span.pos = lexer->cpos;                 \
  strcpy(t.span.literal, sb.items);         \
  return t;                                 \
} while (0)

Token next_token(Lexer *lexer) {
  if (lexer->pos == 0) lex_consume(lexer);
  Token t = {
    .span.pos = lexer->cpos,
  };
  StringBuilder sb = {0};
  if (isspace(lexer->current)){
    switch (lexer->current) {
      case ' ': {
        lexer->cpos.col++;
        break;
      }
      case '\n': {
        lexer->cpos.row++;
        lexer->cpos.col = 1;
        break;
      }
      case '\t': {
        lexer->cpos.col += 8;
        break;
      }
      default: {
        assert(!"unknown space symbol");
      }
    }
    lex_consume(lexer);
    return(TK_WHITESPACE);
  }
  if (isdigit(lexer->current)) {
    while (isdigit(lexer->current)) {
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    return(TK_INT_LIT);
  }
  if (isalpha(lexer->current)) {
    while (isalpha(lexer->current)) {
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    da_push(&sb, '\0');
    for (size_t i = 0; i < sizeof(KeyWordMap) / sizeof(KeyWordMap[0]); ++i) {
      if (strcmp(KeyWordMap[i].id, sb.items) == 0) {
        return(KeyWordMap[i].kind);
      }
    }
    return(TK_LIT);
  }
  return(TK_ERR);
}

