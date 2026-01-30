#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <debug.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <vm.h>

Result vm_next(VM *vm) {
  if (vm->pc >= vm->program.count) {
    return RESULT_ERROR_ILLEGAL_INST_ACCESS;
  }
  Inst inst = vm->program.items[vm->pc++];
  return INST_MAP[inst.opcode].exe(vm, inst);
}

void dump_vm(VM vm) {
  println("Stack:");
  for (size_t i = 0; i < vm.stack.count; ++i) {
    println("  %ld", vm.stack.items[i]);
  }

  println("Memory:");
  for (size_t i = 0; i < vm.memory_pos; ++i) {
    println("  %u", vm.memory[i]);
  }
}

int main(int argc, char *argv[]) {
  int result = EXIT_SUCCESS;
  const char *prog = shift(argv, argc);
  VM vm = {0};
  FILE *file = NULL;

  {
    if (argc != 1) {
      fprintln(stderr, "Usage: %s <file>", prog);
      log(ERROR, "Did not provide any args");
      goto fail;
    }

    char *file_name = shift(argv, argc);
    file = fopen(file_name, "rb");
    if (file == NULL) {
      log(ERROR, "File not found");
      goto fail;
    }

    size_t natives_count;
    if (fread(&natives_count, sizeof(natives_count), 1, file) != 1) {
      log(ERROR, "natives_count couldn't be read from file");
      goto fail;
    }
    for (size_t i = 0; i < natives_count; ++i) {
      size_t char_count;
      if (fread(&char_count, sizeof(char_count), 1, file) != 1) {
        log(ERROR, "char_count couldn't be read from file");
        goto fail;
      }
      char *buf = malloc(char_count + 1);
      if (fread(buf, sizeof(char), char_count, file) != char_count) {
        log(ERROR, "name of native couldn't be read from file");
        goto fail;
      }

      if (strcmp(buf, "write") == 0) {
        da_push(&vm.natives, native_write);
      } else {
        log(ERROR, "Unknown native: %s", buf);
        goto fail;
      }
    }

    Inst inst;
    while (fread(&inst, sizeof(Inst), 1, file) == 1) {
      da_push(&vm.program, inst);
    }

    Result res;
    while (vm.pc < vm.program.count) {
      res = vm_next(&vm);
#ifdef DEBUG_MODE
      dump_vm(vm);
      getchar();
#endif
      if (res != RESULT_OK) {
        log(ERROR, "%s", result_to_str(res));
        goto fail;
      }
    }

#ifdef DEBUG_MODE
    dump_vm(vm);
#endif
  }
end:
  if (file != NULL) fclose(file);
  return result;
fail:
  result = EXIT_FAILURE;
  goto end;
}
