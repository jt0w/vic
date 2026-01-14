#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <vm.h>
#include <debug.h>

typedef enum {
  RESULT_OK,
  RESULT_ERROR_STACK_OVERFLOW,
  RESULT_ERROR_STACK_UNDERFLOW,
  RESULT_ERROR_ILLEGAL_INST,
  RESULT_ERROR_ILLEGAL_INST_ACCESS,
  RESULT_ERROR_MEMORY_OVERFLOW,
  RESULT_ERROR_ILLEGAL_MEMORY_ACCESS,
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
    da_push(&vm->stack,
            vm->stack.items[vm->stack.count - 1 - inst.operand.as_u64]);
    break;
  case OP_ADD:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2].as_u64 +=
        vm->stack.items[vm->stack.count - 1].as_u64;
    vm->stack.count--;
    break;
  case OP_SUB:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2].as_u64 -=
        vm->stack.items[vm->stack.count - 1].as_u64;
    vm->stack.count--;
    break;
  case OP_MULT:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2].as_u64 *=
        vm->stack.items[vm->stack.count - 1].as_u64;
    vm->stack.count--;
    break;
  case OP_DIV:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2].as_u64 /=
        vm->stack.items[vm->stack.count - 1].as_u64;
    vm->stack.count--;
    break;
  case OP_EQ:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
    vm->stack.items[vm->stack.count - 2].as_u64 =
        vm->stack.items[vm->stack.count - 1].as_u64 ==
        vm->stack.items[vm->stack.count - 2].as_u64;
    vm->stack.count--;
    break;
  case OP_JMP:
    vm->pc = inst.operand.as_u64;
    break;
  case OP_JNZ:
    if (vm->stack.items[vm->stack.count - 1].as_u64 != 0)
      vm->pc = inst.operand.as_u64;
    break;
  case OP_JZ:
    if (vm->stack.items[vm->stack.count - 1].as_u64 == 0)
      vm->pc = inst.operand.as_u64;
    break;
  case OP_ALLOC8:
    if (vm->memory_pos >= VM_MEMORY_CAP)
      return RESULT_ERROR_MEMORY_OVERFLOW;
    da_push(&vm->stack, WORD_U64((uint64_t)vm->memory_pos++));
    break;
  case OP_WRITE8:
    if (vm->stack.count < 2)
      return RESULT_ERROR_STACK_UNDERFLOW;
     uint64_t addr = vm->stack.items[vm->stack.count - 2].as_u64;
     if (addr >= VM_MEMORY_CAP)
       return RESULT_ERROR_ILLEGAL_MEMORY_ACCESS;
     uint8_t value = (uint8_t) vm->stack.items[vm->stack.count - 1].as_u64;
     vm->memory[addr] = value;
     vm->stack.count -= 2;
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
