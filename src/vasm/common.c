#include "common.h"

bool translate_file(const char *input_file, Gen *gen, Program *p) {
  bool result = true;
  StringBuilder sb = {0};
  Lexer lex = {0};
  Tokens tokens = {0};
  Exprs exprs = {0};

  if (!read_file((char *)input_file, &sb)) {
    log(ERROR, "could not read `%s`", input_file);
    goto fail;
  }

  {
    lex = (Lexer) {
      .input = sb.items, .cpos = (Position){1, 1}, .file = input_file};
    Token t = {0};
    if (!next_token(&lex, &t)) goto fail;
    while (t.kind != TK_EOF) {
#ifdef DEBUG_MODE
      println("%s", print_token(input_file, t));
#endif
      da_push(&tokens, t);
      if (!next_token(&lex, &t)) goto fail;
    }

    Parser parser = {
      .tokens = tokens,
      .file = input_file,
    };
    while (parser.pos <= parser.tokens.count) {
      Expr temp = {0};
      if (!parse_expr(&parser, &temp)) goto fail;
      da_push(&exprs, temp);
    }
    gen->file = input_file;
    gen->exprs = exprs;
    if (!gen_generate(gen, p)) goto fail;
  }
end:
  if (sb.items != NULL) da_free(sb);
  for (size_t i = 0; i < tokens.count; ++i) {
    free_token(tokens.items[i]);
  }
  if (tokens.items != NULL) da_free(tokens);
  return result;
fail:
  result = false;
  goto end;;
}
