#include "lexer.h"

TK_Map TK_MAP[] = {
    {TK_ERR, "TK_ERR"},
    {TK_INT_LIT, "TK_INT_LIT"},
    {TK_LIT, "TK_LIT"},
    {TK_PUSH, "TK_PUSH"},
    {TK_POP, "TK_POP"},
    {TK_ADD, "TK_ADD"},
    {TK_SUB, "TK_SUB"},
    {TK_MULT, "TK_MULT"},
    {TK_DIV, "TK_DIV"},
    {TK_JMP, "TK_JMP"},
    {TK_JZ, "TK_JZ"},
    {TK_JNZ, "TK_JNZ"},
    {TK_NOP, "TK_NOP"},
    {TK_DUP, "TK_DUP"},
    {TK_COLON, "TK_COLON"},
    {TK_EQ, "TK_EQ"},
    {TK_SEMICOLON, "TK_SEMICOLON"},
    {TK_PERCENT, "TK_PERCENT"},
    {TK_ALLOC, "TK_ALLOC"},
    {TK_WRITE, "TK_WRITE"},
    {TK_WRITE, "TK_CALL"},
    {TK_RET, "TK_RET"},
};

TK_Map KeyWordMap[] = {{TK_PUSH, "push"},   {TK_POP, "pop"},    {TK_ADD, "add"},
                       {TK_SUB, "sub"},     {TK_MULT, "mult"},  {TK_DIV, "div"},
                       {TK_EQ, "eq"},       {TK_JMP, "jmp"},    {TK_NOP, "nop"},
                       {TK_JNZ, "jnz"},     {TK_JZ, "jz"},      {TK_DUP, "dup"},
                       {TK_WRITE, "write"}, {TK_ALLOC, "alloc"}, {TK_CALL, "call"}, {TK_RET, "ret"}};

char *token_name(Token t) {
  for (size_t i = 0; i < sizeof(TK_MAP) / sizeof(TK_MAP[0]); ++i) {
    if (TK_MAP[i].kind == t.kind) {
      return TK_MAP[i].id;
    }
  }

  fprintf(stderr, "Tokenkind %d is not included in TK_MAP\n", t.kind);
  abort();
}

char *print_token(const char *file, Token t) {
  for (size_t i = 0; i < sizeof(TK_MAP) / sizeof(TK_MAP[0]); ++i) {
    if (TK_MAP[i].kind == t.kind) {
      return temp_sprintf("%s:%zu:%zu: %s = `%s`", file, t.span.pos.row,
                          t.span.pos.col, TK_MAP[i].id, t.span.literal);
    }
  }

  fprintf(stderr, "Tokenkind %d is not included in TK_MAP\n", t.kind);
  abort();
}

char lex_consume(Lexer *lexer) {
  if (lexer->current == '\n') {
    lexer->cpos.row++;
    lexer->cpos.col = 0;
  }
  lexer->current = lexer->input[lexer->pos++];
  lexer->cpos.col++;
  return lexer->current;
}

#define return_and_set_span(k)                                                 \
  do {                                                                         \
    t.kind = (k);                                                              \
    da_push(&sb, '\0');                                                        \
    t.span.literal = malloc(sb.count);                                         \
    strcpy(t.span.literal, sb.items);                                          \
    if ((k) == TK_LIT)                                                         \
      t.as.str = t.span.literal;                                               \
    t.span.pos = lexer->cpos;                                                  \
    da_free(sb);                                                               \
    return t;                                                                  \
  } while (0)

Token next_token(Lexer *lexer) {
  Token t = {
      .span.pos = lexer->cpos,
  };
  if (lexer->pos == 0)
    lex_consume(lexer);
  StringBuilder sb = {0};
  while (isspace(lexer->current)) {
    lex_consume(lexer);
  }
  if (isdigit(lexer->current)) {
    while (isdigit(lexer->current)) {
      da_push(&sb, lexer->current);
      t.as.num.as_u64 = t.as.num.as_u64 * 10 + lexer->current - '0';
      lex_consume(lexer);
    }
    return_and_set_span(TK_INT_LIT);
  }

  if (isalnum(lexer->current)) {
    while (isalnum(lexer->current)) {
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    da_push(&sb, '\0');
    for (size_t i = 0; i < sizeof(KeyWordMap) / sizeof(KeyWordMap[0]); ++i) {
      if (strcmp(KeyWordMap[i].id, sb.items) == 0) {
        return_and_set_span(KeyWordMap[i].kind);
      }
    }
    return_and_set_span(TK_LIT);
  }

  da_push(&sb, lexer->current);
  switch (lexer->current) {
  case ':':
    lex_consume(lexer);
    return_and_set_span(TK_COLON);
  case ';':
    lex_consume(lexer);
    return_and_set_span(TK_SEMICOLON);
  case '%':
    lex_consume(lexer);
    return_and_set_span(TK_PERCENT);
  }
  lex_consume(lexer);
  return_and_set_span(TK_ERR);
}
