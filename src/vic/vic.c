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
  uint64_t flags[FLAG_COUNT];

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

uint64_t vm_pop(VM *vm, Register reg) {
  uint64_t res =   vm->stack[vm->sp];
  if (vm->sp == 0) {
    vm->regs[reg] = vm->stack[vm->sp];
    vm->stack[vm->sp] = 0;
  }
  else {
    vm->regs[reg] = vm->stack[vm->sp];
    vm->stack[vm->sp] = 0;
    vm->sp--;
  }
  return res;
}

inline void vm_load_reg(VM *vm, Register reg, uint64_t val) {
  vm->regs[reg] = val;
}

bool vm_push_reg(VM *vm, Register reg)  {
  return vm_push(vm, vm->regs[reg]);
}

// TODO: COMPARING
static inline void compare(VM *vm, uint64_t a, uint64_t b) {
  uint64_t res = a - b;
  if (res == 0) vm->flags[FLAG_ZF] = 0;
  if (res < 0) vm->flags[FLAG_CF] = 0;
  if (res > 0) vm->flags[FLAG_CF] = 1;
}

static inline void vm_cmp_reg(VM *vm, Register ra, Register rb) {
  uint64_t a = vm->regs[ra];
  uint64_t b = vm->regs[rb];
  compare(vm, a, b);
}

static inline void vm_cmp_stack(VM *vm) {
  uint64_t a = vm->stack[vm->sp];
  uint64_t b = vm->stack[vm->sp - 1];
  compare(vm, a, b);
}

bool vm_jmp(VM *vm, uint64_t dest) {
  if (dest > vm->program.count || dest < 0)
    return false;
  vm->pc = dest;
  return true;
}

bool vm_je(VM *vm, uint64_t dest) {
  if (vm->flags[FLAG_ZF] == 0)
    return vm_jmp(vm, dest);
  return true;
}

bool vm_jg(VM *vm, uint64_t dest) {
  if (vm->flags[FLAG_CF])
    return vm_jmp(vm, dest);
  return false;
}

bool vm_jge(VM *vm, uint64_t dest) {
  return vm_je(vm, dest) || vm_jg(vm, dest);
}

bool vm_jl(VM *vm, uint64_t dest) {
  if (!vm->flags[FLAG_CF])
    return vm_jmp(vm, dest);
  return false;
}

bool vm_jle(VM *vm, uint64_t dest) {
  return vm_je(vm, dest) || vm_jl(vm, dest);
}

inline static uint64_t vm_read(VM *vm) {
  uint64_t inst = vm->program.items[vm->pc];
  vm->pc++;
  return inst;
}

void vm_next(VM *vm) {
  uint64_t inst = vm_read(vm);
  /*println("%zu => %u", vm->pc-1, (unsigned int)inst);*/
  switch (inst) {
    case OP_NOP: break;
    case OP_PUSH: {
      uint64_t val = vm_read(vm);
      if (!vm_push(vm, val)){
        fprintln(stderr, "stack overflow");
        exit(1);
      }
      break;
    }
    case OP_POP: {
      uint64_t reg = vm_read(vm);
      assert(reg >= REG_A && reg <= REG_Z);
      vm_pop(vm, reg);
      break;
    }
    case OP_ADD_REG: {
      Register dest = vm_read(vm);
      Register src = vm_read(vm);
      assert(dest >= REG_A && dest <= REG_Z);
      assert(src >= REG_A && src <= REG_Z);
      vm->regs[dest] = vm->regs[dest] + vm->regs[src];
      break;
    }
    case OP_ADD_STACK: {
      assert(vm->sp > 0);
      uint64_t a = vm->stack[vm->sp];
      uint64_t b = vm->stack[vm->sp - 1];
      vm_push(vm, a + b);
      break;
    }
    case OP_LOAD_REG: {
      uint64_t reg = vm_read(vm);
      uint64_t val = vm_read(vm);
      vm_load_reg(vm, reg, val);
      break;
    }
    case OP_PUSH_REG: {
      uint64_t reg = vm_read(vm);
      vm_push_reg(vm, reg);
      break;
    }
    case OP_CMP_REG: {
      uint64_t a = vm_read(vm);
      uint64_t b = vm_read(vm);
      assert(a >= REG_A && a <= REG_Z);
      assert(b >= REG_A && b <= REG_Z);
      vm_cmp_reg(vm, a, b);
      break;
    }
    case OP_CMP_STACK:
      vm_cmp_stack(vm);
      break;
    case OP_JMP: {
      uint64_t dest = vm_read(vm);
      if (!vm_jmp(vm, dest)) {
        fprintln(stderr, "invalid jump dest");
        exit(1);
      }
      break;
    }
    case OP_JE: {
      uint64_t dest = vm_read(vm);
      if (!vm_je(vm, dest)) {
        fprintln(stderr, "invalid jump dest");
        exit(1);
      }
      break;
    }
    default: todo(temp_sprintf("vm_next invalid opcode %u %zu", (unsigned int)inst, vm->pc));
  }
}

int main(int argc, char *argv[]) {
  VM vm = {.sp = -1};
  // REALLY IMPORTANT
  vm.flags[FLAG_ZF] = -1;
  vm.flags[FLAG_CF] = -1;

  da_push(&vm.program, OP_LOAD_REG);
  da_push(&vm.program, REG_A);
  da_push(&vm.program, 0x1);

  da_push(&vm.program, OP_LOAD_REG);
  da_push(&vm.program, REG_B);
  da_push(&vm.program, 0x1);

  da_push(&vm.program, OP_LOAD_REG);
  da_push(&vm.program, REG_Z);
  da_push(&vm.program, 0xFFF);

  da_push(&vm.program, OP_ADD_REG);
  da_push(&vm.program, REG_A);
  da_push(&vm.program, REG_B);

  da_push(&vm.program, OP_CMP_REG);
  da_push(&vm.program, REG_A);
  da_push(&vm.program, REG_Z);

  // for ASM: jne could just be this (NOTE assembler would need to track the count of instructions)
  da_push(&vm.program, OP_JE);
  da_push(&vm.program, vm.program.count + 3);

  da_push(&vm.program, OP_JMP);
  da_push(&vm.program, 9);

  da_push(&vm.program, OP_NOP);

  while (vm.pc <= vm.program.count)
    vm_next(&vm);

  println("Did %u iterations", (unsigned int)vm.regs[REG_A]);

  return 0;
}
