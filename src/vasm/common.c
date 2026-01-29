#include "common.h"

void translate_file(const char *input_file, Gen *gen, Program *p) {
  StringBuilder sb = {0};
  read_file((char *)input_file, &sb);
  Lexer lex = (Lexer) {
    .input = sb.items, .cpos = (Position){1, 1}, .file = input_file};
  Tokens tokens = {0};
  Token t = next_token(&lex);
  while (t.kind != TK_EOF) {
#ifdef DEBUG_MODE
    println("%s", print_token(input_file, t));
#endif
    da_push(&tokens, t);
    t = next_token(&lex);
  }

  Parser parser = {
    .tokens = tokens,
    .file = input_file,
  };
  Exprs exprs = {0};
  while (parser.pos <= parser.tokens.count) {
    Expr temp = parse_expr(&parser);
    da_push(&exprs, temp);
  }
  gen->file = input_file;
  gen->exprs = exprs;
  gen_generate(gen, p);
  da_free(sb);
  da_free(tokens);
}
