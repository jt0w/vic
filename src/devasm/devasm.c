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

  VM_NativeNames natives = {0};
  size_t natives_count;
  if (fread(&natives_count, sizeof(natives_count), 1, file) != 1) {
    log(ERROR, "natives_count couldn't be read from file");
    return 1;
  }
  for (size_t i = 0; i < natives_count; ++i) {
    size_t char_count;
    if (fread(&char_count, sizeof(char_count), 1, file) != 1)  {
      log(ERROR, "char_count couldn't be read from file");
      return 1;
    }
    char *buf = malloc(char_count + 1);
    if (fread(buf, sizeof(char), char_count, file) != char_count) {
      log(ERROR, "name of native couldn't be read from file");
      return 1;
    }
    da_push(&natives, sb_from_string(buf));
  }

  Inst inst;
  while (fread(&inst, sizeof(Inst), 1, file) == 1) {
    Instruction_Mapping mapping = INST_MAP[inst.opcode];
    printf("%s", mapping.name);
    if (mapping.code == OP_NATIVE) printf(" %s", natives.items[inst.operand.as_u64].items);
    else if (mapping.has_operand) printf(" %"PRIu64, inst.operand.as_u64);
    printf("\n");
  }
  return 0;
}
