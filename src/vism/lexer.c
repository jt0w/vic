#include "lexer.h"

char *print_token(Token t, bool all_info) {
  for (size_t i = 0; i < sizeof(TK_MAP) / sizeof(TK_MAP[0]); ++i) {
    if (TK_MAP[i].kind == t.kind) {
      if (all_info)
        return temp_sprintf("%zu: %s = `%s`", t.span.pos.row, TK_MAP[i].id,
                          t.span.literal);
      else 
        return temp_sprintf("`%s`", TK_MAP[i].id);
    }
  }

  fprintf(stderr, "Tokenkind %d is not included in TK_MAP\n", t.kind);
  abort();
}

char lex_consume(Lexer *lexer) {
  lexer->current = lexer->input[lexer->pos++];
  return lexer->current;
}

#define return(k)                                                              \
  do {                                                                         \
    t.kind = (k);                                                              \
    da_push(&sb, '\0');                                                        \
    t.span.literal = malloc(sizeof(sb.items));                                 \
    strcpy(t.span.literal, sb.items);                                          \
    t.span.pos = lexer->cpos;                                                  \
    return t;                                                                  \
  } while (0)

Token next_token(Lexer *lexer) {
  Token t = {
      .span.pos = lexer->cpos,
  };
  if (lexer->pos == 0)
    lex_consume(lexer);
  StringBuilder sb = {0};
  if (isspace(lexer->current)) {
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
    da_push(&sb, lexer->current);
    lex_consume(lexer);
    return (TK_WHITESPACE);
  }
  if (isdigit(lexer->current)) {
    while (isdigit(lexer->current)) {
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    return (TK_INT_LIT);
  }
  if (isalpha(lexer->current)) {
    while (isalpha(lexer->current)) {
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    da_push(&sb, '\0');
    for (size_t i = 0; i < sizeof(KeyWordMap) / sizeof(KeyWordMap[0]); ++i) {
      if (strcmp(KeyWordMap[i].id, sb.items) == 0) {
        return (KeyWordMap[i].kind);
      }
    }
    return (TK_LIT);
  }
  da_push(&sb, lexer->current);
  lex_consume(lexer);
  return (TK_ERR);
}
