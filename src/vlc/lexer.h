#ifndef LEXER_H
#define LEXER_H

#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

typedef struct {
  size_t row;
  size_t col;
} Position;

#define ZERO_POS ((Position){1, 1})

typedef struct {
  Position pos;
  char *literal;
} Span;

typedef enum {
  TK_IDENT,
  TK_PUNCT,
  TK_KEYWORD,
  TK_NUM,
  TK_CHAR,
  TK_STRING,
  TK_EOF,
} TokenKind;

typedef struct {
  TokenKind kind;
  Position pos;
  Span span;
  union{
    char chr;
    char *str;
    uint64_t u64;
  };
} Token;

DA_STRUCT(Token, Tokens)

typedef struct {
  Position pos;
  char c;
  StringBuilder *input;
  const char *file;
} Lexer;

Token next_token(Lexer *lex);
#endif //  LEXER_H
