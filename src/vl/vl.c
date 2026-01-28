#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <debug.h>
#include <vm.h>

Result vm_next(VM *vm) {
  if (vm->pc >= vm->program.count) {
    return RESULT_ERROR_ILLEGAL_INST_ACCESS;
  }
  Inst inst = vm->program.items[vm->pc++];
  return INST_MAP[inst.opcode].exe(vm, inst);
}

int main(int argc, char *argv[]) {
  VM vm = {0};
  const char *prog = shift(argv, argc);

  if (argc != 1) {
    fprintln(stderr, "Usage: %s <file>", prog);
    log(ERROR, "Did not provide any args");
    return 1;
  }

  char *file_name = shift(argv, argc);
  FILE *file = fopen(file_name, "rb");
  if (file == NULL) {
    log(ERROR, "File not found");
    return 1;
  }

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

    if (strcmp(buf, "write") == 0) {
      da_push(&vm.natives, native_write);
    } else {
      log(ERROR, "Unknown native: %s", buf);
      return 1;
    }
  }

  Inst inst;
  while (fread(&inst, sizeof(Inst), 1, file) == 1) {
    da_push(&vm.program, inst);
  }

  Result res;
  while (vm.pc < vm.program.count) {
    res = vm_next(&vm);
    if (res != RESULT_OK) {
      log(ERROR, "%s", result_to_str(res));
      return 1;
    }
  }
  //
  // println("Stack:");
  // for (size_t i = 0; i < vm.stack.count; ++i) {
  //   println("  %ld", vm.stack.items[i]);
  // }
  //
  // println("Memory:");
  // for (size_t i = 0; i < vm.memory_pos; ++i) {
  //   println("  %u", vm.memory[i]);
  // }
  return 0;
}
