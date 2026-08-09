#include <debug.h>
#include "lexer.h"

bool advance(Lexer *lex) {
  lex->c = *(++lex->input->items);
  if (lex->input->count <= 0 && lex->input->items != NULL)
    return false;
  lex->pos.col += 1;
  // TODO: other new lines
  if (lex->c == '\n') {
    lex->pos.row += 1;
    lex->pos.col = 1;
  }
  return true;
}

static const char *keywords[] = {"func"};
static const char keywords_size = sizeof(keywords) / sizeof(keywords[0]);
bool is_keyword(char *name) {
  for (size_t i = 0; i < keywords_size; ++i) {
    if (strcmp(name, keywords[i]) == 0)
      return true;
  }
  return false;
}

void error(Lexer *lex, const char *msg) {
  fprintln(stderr, "%s:%zu:%zu: errror: %s", lex->file, lex->pos.row, lex->pos.col, msg);
  exit(1);
}

#define adv()                                                                  \
  if (!advance(lex))                                                           \
    return (Token){.kind = TK_EOF, .pos = {}, .span = {}};

Token next_token(Lexer *lex) {
  if (lex->input->count <= 0 || lex->c == '\0')
    return (Token){.kind = TK_EOF, .pos = {}, .span = {}};

  while (isspace(lex->input->items[0])) {
    adv();
  }

  if (ispunct(lex->c)) {
    if (lex->c == '\"') {
      Token t = {0};
      t.kind = TK_STRING;
      t.pos = lex->pos;
      adv();
      StringBuilder sb = {0};
      while (lex->c != '\"') {
        da_push(&sb, lex->c);
        if (!advance(lex)) {
          error(lex, "unclosed string lit");
        }
      }
      adv();
      sb_push(&sb, '\0');
      t.span = (Span){
          .pos = t.pos,
          .literal = strdup(sb.items),
      };
      t.str = t.span.literal;
      return t;
    }
    if (lex->c == '\'') {
      Token t = {0};
      t.kind = TK_CHAR;
      t.pos = lex->pos;
      adv();
      t.span = (Span){
          .pos = t.pos,
          .literal = strdup(&lex->c),
      };
    if (lex->c == '\\') {
      adv();
      switch (lex->c) {
      case 'n':
        t.chr = '\n';
        break;
      case 't':
        t.chr = '\t';
        break;
      case 'v':
        t.chr = '\v';
        break;
      case 'r':
        t.chr = '\r';
        break;
      case 'f':
        t.chr = '\f';
        break;
      case 'b':
        t.chr = '\b';
        break;
      case 'a':
        t.chr = '\a';
        break;
      case '\\':
        t.chr = '\\';
        break;
      case '"':
        t.chr = '"';
        break;
      case '\'':
        t.chr = '\'';
        break;
      case '?':
        t.chr = '\?';
        break;
      default:
        error(lex, "unknown escape sequence");
      }
      adv();
      } else {
        t.chr = lex->c;
        adv();
      }

      if (lex->c != '\'') {
        error(lex, "unclosed char lit");
      }
      adv();
      return t;
    }
    Token t = {0};
    t.kind = TK_PUNCT;
    t.pos = lex->pos;
    t.span = (Span){
        .pos = t.pos,
        .literal = strdup(&lex->c),
    };
    t.chr = lex->c;
    t.str = t.span.literal;
    adv();
    return t;
  }
  if (isdigit(lex->c)) {
    Token t = {0};
    t.kind = TK_NUM;
    t.pos = lex->pos;
    t.span.pos = t.pos;
	t.u64 = 0;
    while (isdigit(lex->c)) {
      t.u64 = t.u64 * 10 + lex->c - '0';
      adv();
    }
    int size = snprintf(NULL,0, "%lu", t.u64) + 1;
    t.span.literal = malloc(size);
    snprintf(t.span.literal,size, "%lu", t.u64);
    return t;
  }
  if (isalnum(lex->c)) {
    Token t = {0};
    t.kind = TK_IDENT;
    t.pos = lex->pos;
    StringBuilder sb = {0};
    while (isalnum(lex->c)) {
      sb_push(&sb, lex->c);
      adv();
    }
    sb_push(&sb, '\0');
    t.span.pos = t.pos;
    t.span.literal = strdup(sb.items);
    t.str = t.span.literal;
    if (is_keyword(t.span.literal))
      t.kind = TK_KEYWORD;
    return t;
  }
  if (lex->c == '\0')
    return (Token){.kind = TK_EOF, .pos = lex->pos, .span = {}};
  println("UNREACHABLE: next_token (c = %c)", lex->c);
  abort();
}
