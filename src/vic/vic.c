#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <vm.h>

#define ERROR(x)                                                               \
  {                                                                            \
    fprintln(stderr, x);                                                       \
    abort();                                                                   \
  }

typedef struct {
  size_t count;
  size_t cap;
  uint64_t *items;
} Stack;

typedef struct {
  Program program;
  Stack stack;
  uint64_t regs[REG_COUNT];
  uint64_t flags[FLAG_COUNT];

  size_t pc;
} VM;

bool vm_push(VM *vm, uint64_t val) {
  vm->regs[REG_SP]++;
  da_push(&vm->stack, val);
  return true;
}

uint64_t vm_pop(VM *vm, Register reg) {
  if (vm->regs[REG_SP] == 0) {
    ERROR("vic: error: stack underflow");
  }
  uint64_t res = vm->stack.items[vm->regs[REG_SP - 1]];
  vm->regs[reg] = res;
  vm->stack.items[vm->regs[REG_SP]] = 0;
  vm->regs[REG_SP]--;
  vm->stack.count--;
  return res;
}

inline void vm_load(VM *vm, Register reg, uint64_t val) { vm->regs[reg] = val; }

inline void vm_load_reg(VM *vm, Register dest, Register src) {
  vm->regs[dest] = vm->regs[src];
}
bool vm_push_reg(VM *vm, Register reg) { return vm_push(vm, vm->regs[reg]); }

static inline void compare(VM *vm, uint64_t a, int64_t b) {
  uint64_t res = a - b;
  if (res == 0)
    vm->flags[FLAG_ZF] = 0;
  if (res < 0)
    vm->flags[FLAG_CF] = 0;
  if (res > 0)
    vm->flags[FLAG_CF] = 1;
}

static inline void vm_cmp_reg(VM *vm, Register ra, Register rb) {
  uint64_t a = vm->regs[ra];
  uint64_t b = vm->regs[rb];
  compare(vm, a, b);
}

static inline void vm_cmp_stack(VM *vm) {
  uint64_t a = vm->stack.items[vm->regs[REG_SP]];
  uint64_t b = vm->stack.items[vm->regs[REG_SP] - 1];
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
  switch (inst) {
  case OP_NOP:
    break;
  case OP_PUSH: {
    uint64_t val = vm_read(vm);
    if (!vm_push(vm, val)) {
      ERROR("stack overflow");
    }
    break;
  }
  case OP_POP_REG: {
    uint64_t reg = vm_read(vm);
    assert(reg >= REG_A && reg <= REG_Z);
    vm_pop(vm, reg);
    break;
  }
  case OP_ADD: {
    Register dest = vm_read(vm);
    Register src = vm_read(vm);
    assert(dest >= REG_A && dest <= REG_Z);
    assert(src >= REG_A && src <= REG_Z);
    vm->regs[dest] = vm->regs[dest] + vm->regs[src];
    break;
  }
  case OP_SUB: {
    Register dest = vm_read(vm);
    Register src = vm_read(vm);
    assert(dest >= REG_A && dest <= REG_Z);
    assert(src >= REG_A && src <= REG_Z);
    vm->regs[dest] = vm->regs[dest] - vm->regs[src];
    break;
  }
  case OP_MULT: {
    Register dest = vm_read(vm);
    Register src = vm_read(vm);
    assert(dest >= REG_A && dest <= REG_Z);
    assert(src >= REG_A && src <= REG_Z);
    vm->regs[dest] = vm->regs[dest] * vm->regs[src];
    break;
  }
  case OP_DIV: {
    Register dest = vm_read(vm);
    Register src = vm_read(vm);
    assert(dest >= REG_A && dest <= REG_Z);
    assert(src >= REG_A && src <= REG_Z);
    vm->regs[dest] = vm->regs[dest] / vm->regs[src];
    break;
  }
  case OP_LOAD: {
    uint64_t reg = vm_read(vm);
    uint64_t val = vm_read(vm);
    vm_load(vm, reg, val);
    break;
  }
  case OP_LOAD_REG: {
    uint64_t dest = vm_read(vm);
    uint64_t src = vm_read(vm);
    vm_load_reg(vm, dest, src);
    break;
  }
  case OP_PUSH_REG: {
    uint64_t reg = vm_read(vm);
    vm_push_reg(vm, reg);
    break;
  }
  case OP_CMP: {
    uint64_t a = vm_read(vm);
    uint64_t b = vm_read(vm);
    assert(a >= REG_A && a <= REG_Z);
    assert(b >= REG_A && b <= REG_Z);
    vm_cmp_reg(vm, a, b);
    break;
  }
  case OP_JMP: {
    uint64_t dest = vm_read(vm);
    if (!vm_jmp(vm, dest))
      ERROR("vic: error: invalid jump dest");
    break;
  }
  case OP_JE: {
    uint64_t dest = vm_read(vm);
    if (!vm_je(vm, dest))
      ERROR("vic: error: invalid jump dest");
    break;
  }
  case OP_SYSCALL: {
    uint64_t sys_code = vm->regs[REG_A];
    switch (sys_code) {
    // sys read
    case 0: {
      todo("sys read");
      break;
    }
    // sys write
    case 1: {
      uint64_t fd = vm->regs[REG_B];
      uint64_t c = vm->regs[REG_C];
      if (!write(fd, &c, 1)) {
        ERROR("vic: error: write failed");
      }
      break;
    }
    // sys open
    case 2: {
      todo("sys open");
      break;
    }
    // sys close
    case 3: {
      todo("sys close");
      break;
    }
    case 4: {
      uint64_t exit_code = vm->regs[REG_B];
      exit(exit_code);
      break;
    }
    default:
      ERROR("vic: error: unsupported syscall");
    }
    break;
  }
  default:
    todo(temp_sprintf("vm_next invalid opcode %u %zu", (unsigned int)inst,
                      vm->pc));
  }
}

int main(int argc, char *argv[]) {
  VM vm = {};

  vm.flags[FLAG_ZF] = -1;
  vm.flags[FLAG_CF] = -1;

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

  uint64_t value;

  while (fread(&value, sizeof(uint64_t), 1, file) == 1) {
    da_push(&vm.program, value);
  }

  while (vm.pc <= vm.program.count)
    vm_next(&vm);

  return 0;
}
