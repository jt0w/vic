#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include "preproc.h"
#include <debug.h>

#include "gen.h"
#include "lexer.h"
#include "parser.h"

#define VERSION "0.0.1-beta"
int main(int argc, char **argv) {
  shift(argv, argc);
  Flags flags = {0};

  Flag help_flag = parse_boolean_flag(flags, "-help", "-h", false,
                                      "Wether to print help message");
  Flag output_file = parse_str_flag(flags, "-output", "-o", "out.bin",
                                    "Output File (Default: out.bin)");
  (void)output_file;
  Flag input_file = parse_str_flag(flags, "-input", "-i", NULL, "Input file");

  if (!flags_check(flags, argc)) {
    StringBuilder sb = {0};
    flags_err_str(flags, &sb, argv, argc);
    sb_push(&sb, '\0');
    return 1;
  }

  if (help_flag.as.boolean) {
    print_flags_help(flags);
  }

  if (!input_file.as.str) {
    print_flags_help(flags);
    log(ERROR, "Did not provide input file");
    return 0;
  }

  int res = 0;
  StringBuilder input = {0};
  StringBuilder processed_input = {0};
  PreProc pre = {0};
  Lexer lex = {0};
  Tokens tokens = {0};
  Parser parser = {0};
  Stmts stmts = {0};
  Gen gen = {0};
  Program program = {0};
  // __asm__ __volatile__("" ::"m"(program));


  Native natives[] = {
    {native_write, sb_from_string("write")},
    {native_exit, sb_from_string("exit")},
  };
  static const size_t natives_c = sizeof(natives) / sizeof(natives[0]);
  gen.natives = natives;
  gen.natives_c = natives_c;


  {
    if (!read_file((char *)input_file.as.str, &input)) {
      log(ERROR, "could not read file '%s'", input_file.as.str);
      goto fail;
    }
	sb_push(&input, '\0');

    pre = (PreProc){
        .input = &input,
        .c = *input.items,
    };
    preproc_proc(&pre, &processed_input);

    lex = (Lexer){
        .file = input_file.as.str,
        .input = &processed_input,
        .c = processed_input.items[0],
        .pos = ZERO_POS,
    };
    for (;;) {
      Token t = next_token(&lex);
      da_push(&tokens, t);
      if (t.kind == TK_EOF)
        break;
    }
    parser = (Parser){
        .tks = tokens,
        .t = tokens.items[0],
    };
    while (parser.tks.count > 1) {
      Stmt s = {0};
      s = parse_stmt(&parser);
      da_push(&stmts, s);
    }

 gen.stmts = stmts;
    gen.s = stmts.items[0];
    while (gen.stmts.count > 0) {
      Program p = gen_gen(&gen);
      da_push_mult(&program, p.items, p.count);
    }

  }

  {
    FILE *bfile = fopen(output_file.as.str, "wb");
    assert(bfile != NULL);
    fwrite(&natives_c, sizeof(natives_c), 1, bfile);
    for (size_t i = 0; i < natives_c; ++i) {
      fwrite(&natives[i].name.count, sizeof(gen.natives[i].name.count), 1,
             bfile);
      fwrite(gen.natives[i].name.items, sizeof(*gen.natives[i].name.items),
             gen.natives[i].name.count, bfile);
    }
    fwrite(program.items, sizeof(*program.items), program.count, bfile);
    fclose(bfile);
  }

  println("vlc %s", VERSION);
done:
  return res;
fail:
  res = 1;
  goto done;
}
