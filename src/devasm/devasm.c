#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <vm.h>

int main(int argc, char **argv) {
  const char *prog_name = shift(argv, argc);
  if (argc != 1) {
    log(ERROR, "No input file provided");
    fprintln(stderr, "Usage: %s <input.vb>", prog_name);
    return 1;
  }
  char *file_name = shift(argv, argc);
  FILE *file = fopen(file_name, "rb");
  if (file == NULL) {
    log(ERROR, "File not found");
    return 1;
  }

  Inst inst;
  while (fread(&inst, sizeof(Inst), 1, file) == 1) {
    Instruction_Mapping mapping = INST_MAP[inst.opcode];
    printf("%s", mapping.name);
    if (mapping.has_operand) printf(" %"PRIu64, inst.operand.as_u64);
    printf("\n");
  }
  return 0;
}
