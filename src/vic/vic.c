#define STC_IMPLEMENTATION
#define STC_STRIP_PREFIX
#include <stc.h> 

#include <stdio.h>
#include <stdbool.h>
#include <vm.h>

#define STACK_SIZE (8 * 1024)
 
typedef struct {
  Program program;
  uint64_t stack[STACK_SIZE];
  uint64_t regs[REG_COUNT];

  size_t sp;
  size_t pc;
} VM;

bool vm_push(VM *vm, uint64_t val) {
  vm->sp++;
  vm->stack[vm->sp] = val;
  if (vm->sp >= STACK_SIZE)
    return false;
  return true;
}

void vm_pop(VM *vm, Register reg) {
  if (vm->sp == 0) {
    vm->regs[reg] = vm->stack[vm->sp];
    vm->stack[vm->sp] = 0;
  }
  else {
    vm->regs[reg] = vm->stack[vm->sp];
    vm->stack[vm->sp] = 0;
    vm->sp--;
  }
}

inline void vm_load_reg(VM *vm, Register reg, uint64_t val) {
  vm->regs[reg] = val;
}

bool vm_push_reg(VM *vm, Register reg)  {
  return vm_push(vm, vm->regs[reg]);
}

// TODO: COMPARING
/*static uint64_t compare(uint64_t a, uint64_t b) {*/
/*  todo("error");*/
/*  if (a == b) return JE;*/
/*  if (a >= b) return JGE;*/
/*  if (a > b) return JG;*/
/*  if (a <= b) return JLE;*/
/*  if (a < b) return JL;*/
/*  fprintf(stderr, "UNREACHABLE");*/
/*  abort();*/
/*}*/
/**/
/*bool vm_cmp_reg(VM *vm, Register ra, Register rb) {*/
/*  uint64_t a = vm->regs[a];*/
/*  uint64_t b = vm->regs[b];*/
/*  return vm_push(vm, compare(a, b));*/
/*}*/
/**/
/*bool vm_cmp_stack(VM *vm) {*/
/*  uint64_t a = vm->stack[vm->sp];*/
/*  uint64_t b = vm->stack[vm->sp - 1];*/
/*  return vm_push(vm, compare(a, b));*/
/*}*/
/**/
/*bool vm_jmp(VM *vm, uint64_t dest) {*/
/*  if (dest > vm->program.count || dest < 0)*/
/*    return false;*/
/*  vm->pc = dest;*/
/*  return true;*/
/*}*/

inline static uint64_t vm_read(VM *vm) {
  uint64_t inst = vm->program.items[vm->pc];
  vm->pc++;
  return inst;
}

void vm_next(VM *vm) {
  uint64_t inst = vm_read(vm);
  uint64_t val;
  uint64_t reg;
  switch (inst) {
    case OP_NOP:
      break;
    case OP_PUSH:
      val = vm_read(vm);
      if (!vm_push(vm, val)){
        fprintln(stderr, "stack overflow");
        exit(1);
      }
      break;
    case OP_POP:
      reg = vm_read(vm);
      assert(reg >= A && val <= Z);
      vm_pop(vm, reg);
      break;
    case OP_LOAD_REG:
      reg = vm_read(vm);
      val = vm_read(vm);
      vm_load_reg(vm, reg, val);
      break;
    case OP_PUSH_REG:
      reg = vm_read(vm);
      vm_push_reg(vm, reg);
      break;
    default:
      todo("vm_next invalid opcode");
  }
}

int main(int argc, char *argv[]) {
  VM vm = {.sp = -1};

  da_push(&vm.program, OP_LOAD_REG);
  da_push(&vm.program, A);
  da_push(&vm.program, 0xFF);

  da_push(&vm.program, OP_PUSH_REG);
  da_push(&vm.program, A);

  da_push(&vm.program, OP_POP);
  da_push(&vm.program, B);


  while (vm.pc <= vm.program.count)
    vm_next(&vm);

  println("A %u", vm.regs[A]);
  println("B %u", vm.regs[B]);
  return 0;
}
