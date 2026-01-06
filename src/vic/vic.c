#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <vm.h>

typedef struct {
  size_t count;
  size_t cap;
  Word *items;
} Stack;

typedef struct {
  Program program;
  Stack stack;
  size_t pc;
} VM;

typedef enum {
  RESULT_OK,
  RESULT_ERROR_STACK_OVERFLOW,
  RESULT_ERROR_STACK_UNDERFLOW,
  RESULT_ERROR_ILLEGAL_INST,
  RESULT_ERROR_ILLEGAL_INST_ACCESS,
} Result;

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
  default:
    assert(0 && "UNREACHABLE: err_to_str");
  }
}

Result vm_next(VM *vm) {
  if (vm->pc >= vm->program.count) {
    return RESULT_ERROR_ILLEGAL_INST_ACCESS;
  }
  Inst inst = vm->program.items[vm->pc++];
  switch (inst.opcode) {
  case OP_NOP:
    break;
  case OP_PUSH:
    da_push(&vm->stack, inst.operand);
    break;
  case OP_POP:
    if (vm->stack.count == 0)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.count--;
    break;
  case OP_DUP:
    if (vm->stack.count < 1)
      return RESULT_ERROR_STACK_UNDERFLOW;
    da_push(&vm->stack, vm->stack.items[vm->stack.count - 1 - inst.operand]);
    break;
  case OP_ADD:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2] +=
        vm->stack.items[vm->stack.count - 1];
    vm->stack.count--;
    break;
  case OP_SUB:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2] -=
        vm->stack.items[vm->stack.count - 1];
    vm->stack.count--;
    break;
  case OP_MULT:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2] *=
        vm->stack.items[vm->stack.count - 1];
    vm->stack.count--;
    break;
  case OP_DIV:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2] /=
        vm->stack.items[vm->stack.count - 1];
    vm->stack.count--;
    break;
  case OP_EQ:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2] =
        vm->stack.items[vm->stack.count - 1] ==
        vm->stack.items[vm->stack.count - 2];
    vm->stack.count--;
    break;
  case OP_JMP:
    vm->pc = inst.operand;
    break;
  case OP_JNZ:
    if (vm->stack.items[vm->stack.count - 1] != 0)
      vm->pc = inst.operand;
    break;
  case OP_JZ:
    if (vm->stack.items[vm->stack.count - 1] == 0)
      vm->pc = inst.operand;
    break;
  default:
    return RESULT_ERROR_ILLEGAL_INST;
  }
  return RESULT_OK;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  VM vm = {0};
  shift(argv, argc);

  if (argc != 1) {
    fprintln(stderr, "Usage: vic <file>");
    fprintln(stderr, "ERROR: Did not provide any args");
    return 1;
  }

  char *file_name = shift(argv, argc);
  FILE *file = fopen(file_name, "rb");
  if (file == NULL) {
    fprintln(stderr, "ERROR: File not found");
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
  return 0;
}
