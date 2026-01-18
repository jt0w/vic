#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <debug.h>
#include <vm.h>


const char *result_to_str(Result e) {
  switch (e) {
  case RESULT_OK:
    return "OK";
  case RESULT_ERROR_STACK_OVERFLOW:
    return "ERROR_STACK_OVERFLOW";
  case RESULT_ERROR_STACK_UNDERFLOW:
    return "ERROR_STACK_UNDERFLOW";
  case RESULT_ERROR_ILLEGAL_INST:
    return "ERROR_ILLEGAL_INST";
  case RESULT_ERROR_ILLEGAL_INST_ACCESS:
    return "ERROR_ILLEGAL_INST_ACCESS";
  case RESULT_ERROR_MEMORY_OVERFLOW:
    return "RESULT_ERROR_MEMORY_OVERFLOW";
  case RESULT_ERROR_ILLEGAL_MEMORY_ACCESS:
    return "RESULT_ERROR_ILLEGAL_MEMORY_ACCESS";
  default:
    assert(0 && "UNREACHABLE: err_to_str");
  }
}

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

  println("Stack:");
  for (size_t i = 0; i < vm.stack.count; ++i) {
    println("  %ld", vm.stack.items[i]);
  }

  println("Memory:");
  for (size_t i = 0; i < vm.memory_pos; ++i) {
    println("  %u", vm.memory[i]);
  }
  return 0;
}
