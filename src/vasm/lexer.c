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
    {TK_READ, "TK_READ"},
    {TK_CALL, "TK_CALL"},
    {TK_RET, "TK_RET"},
    {TK_NATIVE, "TK_NATIVE"},
    {TK_DEF, "TK_DEF"},
    {TK_SWAP, "TK_SWAP"},
    {TK_CHAR, "TK_CHAR"},
    {TK_STRING, "TK_STRING"},
    {TK_USE, "TK_USE"},


    {TK_EOF, "TK_EOF"},
};

TK_Map KeyWordMap[] = {
    {TK_PUSH, "push"},     {TK_POP, "pop"},     {TK_ADD, "add"},
    {TK_SUB, "sub"},       {TK_MULT, "mult"},   {TK_DIV, "div"},
    {TK_EQ, "eq"},         {TK_JMP, "jmp"},     {TK_NOP, "nop"},
    {TK_JNZ, "jnz"},       {TK_JZ, "jz"},       {TK_DUP, "dup"},
    {TK_WRITE, "write"},   {TK_ALLOC, "alloc"}, {TK_CALL, "call"},
    {TK_RET, "ret"},       {TK_SWAP, "swap"},   {TK_READ, "read"},
    {TK_NATIVE, "native"}, {TK_DEF, "def"}, {TK_USE, "use"},
};

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

  switch (lexer->current) {
  case '\0':
    da_push(&sb, lexer->current);
    return_and_set_span(TK_EOF);
  case ':':
    da_push(&sb, lexer->current);
    lex_consume(lexer);
    return_and_set_span(TK_COLON);
  case ';':
    da_push(&sb, lexer->current);
    lex_consume(lexer);
    return_and_set_span(TK_SEMICOLON);
  case '%':
    da_push(&sb, lexer->current);
    lex_consume(lexer);
    return_and_set_span(TK_PERCENT);
  case '"':
    lex_consume(lexer);
    while (true) {
      if (lexer->current == '"') break;
      if (lexer->current == '\n') {
        fprintln(stderr, "%s:%zu:%zu: error: line break in single line string literal",
            lexer->file, lexer->cpos.row, lexer->cpos.col);
        exit(1);
      }
      if (lexer->current == '\0') {
        fprintln(stderr, "%s:%zu:%zu: error: unterminated string literal",
            lexer->file, lexer->cpos.row, lexer->cpos.col);
        exit(1);
      }
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    lex_consume(lexer);
    return_and_set_span(TK_STRING);
  case '\'':
    lex_consume(lexer);
    da_push(&sb, '\'');
    if(lexer->current == '\\') {
      da_push(&sb, '\\');
      lex_consume(lexer);
      switch (lexer->current) {
      case 'n':
        t.as.chr = '\n';
        break;
      case 't':
        t.as.chr = '\t';
        break;
      case 'v':
        t.as.chr = '\v';
        break;
      case 'r':
        t.as.chr = '\r';
        break;
      case 'f':
        t.as.chr = '\f';
        break;
      case 'b':
        t.as.chr = '\b';
        break;
      case 'a':
        t.as.chr = '\a';
        break;
      case '\\':
        t.as.chr = '\\';
        break;
      case '"':
        t.as.chr = '"';
        break;
      case '\'':
        t.as.chr = '\'';
        break;
      case '?':
        t.as.chr = '\?';
        break;
      default:
        fprintln(stderr, "%s:%zu:%zu: error: unknown escape sequence `\\%c`",
            lexer->file, lexer->cpos.row, lexer->cpos.col, lexer->current);
        exit(1);
      }
      lex_consume(lexer);
    } else {
      t.as.chr = lexer->current;
      if (t.as.chr == '\'') {
        fprintln(stderr, "%s:%zu:%zu: errror: empty string literal",
            lexer->file, lexer->cpos.row, lexer->cpos.col);
        exit(1);
      }
      da_push(&sb, lexer->current);
      lex_consume(lexer);
    }
    if (lexer->current != '\'') {
      fprintln(stderr, "%s:%zu:%zu: error: unterminated string literal",
          lexer->file, lexer->cpos.row, lexer->cpos.col);
      exit(1);
    }
    lex_consume(lexer);
    da_push(&sb, '\'');
    return_and_set_span(TK_CHAR);
  }
  lex_consume(lexer);
  return_and_set_span(TK_ERR);
}
