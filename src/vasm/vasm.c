#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include "common.h"

#include <debug.h>
#include <vm.h>

#define VERSION "0.0.1"

int main(int argc, char *argv[]) {
  shift(argv, argc);
  Flags flags = {0};

  Flag output_file = parse_str_flag(flags,"-output", "-o", "out.bin", "Output File (Default: out.bin)");
  Flag input_file = parse_str_flag(flags, "-input", "-i", NULL, "Input file");

  if (!input_file.as.str) {
    print_flags_help(flags);
    log(ERROR, "Did not provide input file");
    return 0;
  }

  Gen gen = {0};
  Program program = {0};
  if (!translate_file(input_file.as.str, &gen, &program)) return 1;
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
  da_free(gen.exprs);
  da_free(gen.labels);
  da_free(gen.natives);
  da_free(gen.unresolved_jumps);
  da_free(gen.vars);
  return 0;
}
