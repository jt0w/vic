#include <stdio.h>
#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include "gen.h"
#include "lexer.h"
#include "parser.h"

#include <debug.h>
#include <vm.h>

#define VERSION "0.0.1"

int main(int argc, char *argv[]) {
  shift(argv, argc);

  Flag output_file = parse_str_flag("-output", "-o", "out.bin");
  Flag input_file = parse_str_flag("-input", "-i", NULL);

  if (!input_file.as.str) {
    fprintln(stderr, "Usage: vasm -o <output.> -i <input.vasm>");
    log(ERROR, "Did not provide input file");
    return 0;
  }

  StringBuilder sb = {0};
  read_file((char *)input_file.as.str, &sb);
  Lexer lexer = {
      .input = sb.items, .cpos = (Position){1, 1}, .file = input_file.as.str};
  Tokens tokens = {0};
  Token t = next_token(&lexer);
  while (t.kind != TK_EOF) {
#ifdef DEBUG_MODE
    println("%s", print_token(input_file.as.str, t));
#endif
    da_push(&tokens, t);
    t = next_token(&lexer);
  }

  Parser parser = {
      .tokens = tokens,
      .file = input_file.as.str,
  };
  Exprs exprs = {0};
  while (parser.pos <= parser.tokens.count) {
    Expr temp = parse_expr(&parser);
    da_push(&exprs, temp);
  }
  Gen gen = {.exprs = exprs};
  Program program = gen_generate(&gen);
  FILE *bfile = fopen(output_file.as.str, "wb");
  assert(bfile != NULL);
  fwrite(&gen.natives.count, sizeof(gen.natives.count), 1, bfile);
  for (size_t i = 0; i < gen.natives.count; ++i) {
    fwrite(&gen.natives.items[i].count, sizeof(gen.natives.items[i].count), 1,
           bfile);
    fwrite(gen.natives.items[i].items, sizeof(*gen.natives.items[i].items),
           gen.natives.items[i].count, bfile);
  }
  fwrite(program.items, sizeof(*program.items), program.count, bfile);
  fclose(bfile);
  println("Vism %s", VERSION);
  return 0;
}
