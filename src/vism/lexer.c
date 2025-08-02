#include "lexer.h"

TK_Map TK_MAP[] = {
    {TK_ERR, "TK_ERR"},         {TK_WHITESPACE, "TK_WHITESPACE"},

    {TK_INT_LIT, "TK_INT_LIT"}, {TK_LIT, "TK_LIT"},

    {TK_PUSH, "TK_PUSH"},       {TK_POP, "TK_POP"},

    {TK_LOAD, "TK_LOAD"},

    {TK_ADD, "TK_ADD"},         {TK_SUB, "TK_SUB"},
    {TK_MULT, "TK_MULT"},       {TK_DIV, "TK_DIV"},

    {TK_CMP, "TK_CMP"},

    {TK_JMP, "TK_JMP"},         {TK_JE, "TK_JE"},
    {TK_JGE, "TK_JGE"},         {TK_JG, "TK_JG"},
    {TK_JLE, "TK_JLE"},         {TK_JL, "TK_JL"},
    {TK_SYSCALL, "TK_SYSCALL"}, {TK_NOP, "TK_NOP"},
};

TK_Map KeyWordMap[] = {
    {TK_PUSH, "push"}, {TK_POP, "pop"}, {TK_LOAD, "load"},
    {TK_ADD, "add"},   {TK_SUB, "sub"}, {TK_MULT, "mult"},
    {TK_DIV, "div"},   {TK_CMP, "cmp"}, {TK_JMP, "jmp"},
    {TK_JE, "je"},     {TK_JGE, "jge"}, {TK_JG, "jg"},
    {TK_JLE, "jle"},   {TK_JL, "jl"},   {TK_SYSCALL, "syscall"},
    {TK_NOP, "nop"},
};

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
  exit(1);
}

char lex_consume(Lexer *lexer) {
  lexer->current = lexer->input[lexer->pos++];
  return lexer->current;
}

#define return(k)                                                              \
  do {                                                                         \
    t.kind = (k);                                                              \
    da_push(&sb, '\0');                                                        \
    t.span.literal = malloc(sb.count);                                         \
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
