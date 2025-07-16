#include <stdio.h>
#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include "lexer.h"
#include "parser.h"

#include <vm.h>

int main(int argc, char *argv[]) {
  shift(argv, argc);

  if (argc != 1) {
    fprintln(stderr, "Usage: vism <file>");
    fprintln(stderr, "ERROR: Did not provide any args");
    return 0;
  }
  char *file = shift(argv, argc);
  StringBuilder sb = {0};
  read_file(file, &sb);
  Lexer lexer = {.input = sb.items, .cpos = (Position){1, 1}, .file = file};
  Tokens tokens = {0};
  while (lexer.pos < sb.count) {
    da_push(&tokens, next_token(&lexer));
    println("%s", print_token(tokens.items[tokens.count - 1]));
  }
  Parser parser = {
      .tokens = tokens,
      .file = file,
  };
  Exprs exprs = {0};
  while (parser.pos < parser.tokens.count) {
    Expr temp = parse_expr(&parser);
    da_push(&exprs, temp);
  }
  da_free(sb);
  da_free(tokens);
  da_free(exprs);
  return 0;
}
