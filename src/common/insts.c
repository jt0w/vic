#include "vm.h"

Result vm_nop(VM *vm, Inst inst) {
  (void)vm;
  (void)inst;
  return RESULT_OK;
}

Result vm_push(VM *vm, Inst inst) {
  da_push(&vm->stack, inst.operand);
  return RESULT_OK;
}

Result vm_pop(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 1)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_dup(VM *vm, Inst inst) {
  if (vm->stack.count < 1)
    return RESULT_ERROR_STACK_UNDERFLOW;
  if ((int64_t)vm->stack.count - (int64_t)1 - (int64_t)inst.operand.as_u64 < (int64_t)0)
    return RESULT_ERROR_STACK_UNDERFLOW;
  da_push(&vm->stack,
          vm->stack.items[vm->stack.count - 1 - inst.operand.as_u64]);
  return RESULT_OK;
}

Result vm_swap(VM *vm, Inst inst) {
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;
  if ((int64_t)vm->stack.count - (int64_t)1 - (int64_t)inst.operand.as_u64 < (int64_t)0)
    return RESULT_ERROR_STACK_UNDERFLOW;
  Word top = vm->stack.items[vm->stack.count - 1];
  Word swap = vm->stack.items[vm->stack.count - 1 - inst.operand.as_u64];
  vm->stack.items[vm->stack.count - 1] = swap;
  vm->stack.items[vm->stack.count - 1 - inst.operand.as_u64] = top;
  return RESULT_OK;
}

Result vm_add(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->stack.items[vm->stack.count - 2].as_u64 +=
      vm->stack.items[vm->stack.count - 1].as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_sub(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->stack.items[vm->stack.count - 2].as_u64 -=
      vm->stack.items[vm->stack.count - 1].as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_mult(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->stack.items[vm->stack.count - 2].as_u64 *=
      vm->stack.items[vm->stack.count - 1].as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_div(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->stack.items[vm->stack.count - 2].as_u64 /=
      vm->stack.items[vm->stack.count - 1].as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_eq(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->stack.items[vm->stack.count - 2].as_u64 =
      vm->stack.items[vm->stack.count - 1].as_u64 ==
      vm->stack.items[vm->stack.count - 2].as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_jmp(VM *vm, Inst inst) {
  vm->pc = inst.operand.as_u64;
  return RESULT_OK;
}

Result vm_jnz(VM *vm, Inst inst) {
  if (vm->stack.count < 1)
    return RESULT_ERROR_STACK_UNDERFLOW;
  if (vm->stack.items[vm->stack.count - 1].as_u64 != 0)
    vm->pc = inst.operand.as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_jz(VM *vm, Inst inst) {
  if (vm->stack.count < 1)
    return RESULT_ERROR_STACK_UNDERFLOW;
  if (vm->stack.items[vm->stack.count - 1].as_u64 == 0)
    vm->pc = inst.operand.as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_alloc(VM *vm, Inst inst) {
  if (vm->memory_pos >= VM_MEMORY_CAP)
    return RESULT_ERROR_MEMORY_OVERFLOW;
  Word operand = inst.operand;
  da_push(&vm->stack, WORD_U64((uint64_t)vm->memory_pos));
  vm->memory_pos += operand.as_u64;
  return RESULT_OK;
}

Result vm_write(VM *vm, Inst inst) {
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;

  uint64_t addr = vm->stack.items[vm->stack.count - 2].as_u64;
  if (addr >= VM_MEMORY_CAP)
    return RESULT_ERROR_ILLEGAL_MEMORY_ACCESS;

  uint64_t count = inst.operand.as_u64;
  uint64_t value = vm->stack.items[vm->stack.count - 1].as_u64;
  memset(&vm->memory[addr], value, count);

  vm->stack.count -= 2;
  return RESULT_OK;
}

Result vm_read(VM *vm, Inst inst) {
  if (inst.operand.as_u64 > VM_MEMORY_CAP)
    return RESULT_ERROR_ILLEGAL_MEMORY_ACCESS;
  if (vm->stack.count < 1)
    return RESULT_ERROR_STACK_UNDERFLOW;
  uint64_t value = 0;
  uint64_t count = inst.operand.as_u64;
  memcpy(&value, &vm->memory[vm->stack.items[vm->stack.count - 0].as_u64], count);
  da_push(&vm->stack, WORD_U64(value));
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_ret(VM *vm, Inst inst) {
  (void)inst;
  if (vm->stack.count < 1)
    return RESULT_ERROR_STACK_UNDERFLOW;
  vm->pc = vm->stack.items[vm->stack.count - 1].as_u64;
  vm->stack.count--;
  return RESULT_OK;
}

Result vm_native(VM *vm, Inst inst) {
  uint64_t id = inst.operand.as_u64;
  if (id >= vm->natives.count)
    return RESULT_ERROR_ILLEGAL_NATIVE;
  Result err = vm->natives.items[id](vm);
  if (err != RESULT_OK)
    return err;
  return RESULT_OK;
}
