#include <stdio.h>
#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include "gen.h"
#include "lexer.h"
#include "parser.h"

#include <vm.h>

#define VERSION "0.0.1-alpha"

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
  while (lexer.pos < sb.count)
    da_push(&tokens, next_token(&lexer));
  Parser parser = {
      .tokens = tokens,
      .file = file,
  };
  Exprs exprs = {0};
  while (parser.pos <= parser.tokens.count) {
    Expr temp = parse_expr(&parser);
    da_push(&exprs, temp);
  }
  Gen gen = {.exprs = exprs};
  Program program = {0};
  while (gen.pos <= gen.exprs.count) {
    Program temp = gen_parse_expr(&gen);
    for (size_t i = 0; i < temp.count; ++i)
      da_push(&program, temp.items[i]);
  }
  FILE *bfile = fopen("data.bin", "wb");
  assert(bfile != NULL);
  fwrite(program.items, sizeof(uint64_t), program.count, bfile);
  fclose(bfile);
  println("Vism %s", );
  println("%zu bytes", program.count * sizeof(uint64_t));
  return 0;
}
