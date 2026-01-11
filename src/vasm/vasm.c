#include <stdio.h>
#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include "gen.h"
#include "lexer.h"
#include "parser.h"

#include <vm.h>
#include <debug.h>


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
  read_file((char*)input_file.as.str, &sb);
  Lexer lexer = {.input = sb.items, .cpos = (Position){1, 1}, .file = input_file.as.str};
  Tokens tokens = {0};
  while (lexer.pos < sb.count) {
    da_push(&tokens, next_token(&lexer));
#ifdef DEBUG_MODE
   println("%s", print_token(input_file.as.str, tokens.items [tokens.count - 1]));
#endif
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
  fwrite(program.items, sizeof(Inst), program.count, bfile);
  fclose(bfile);
  println("Vism %s", VERSION);
  println("%zu bytes", program.count * sizeof(Inst));
  return 0;
}
