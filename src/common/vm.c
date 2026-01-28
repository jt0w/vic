#include "vm.h"

#include "insts.c"

const Instruction_Mapping INST_MAP[OPCODE_COUNT] = {
    [OP_NOP] = {
      .code = OP_NOP,
      .name = "nop",
      .has_operand = false,
      .inputs = {},
      .outputs = {},
      .exe = vm_nop,
    },
    [OP_PUSH] = {
      .code = OP_PUSH,
      .name = "push",
      .has_operand = true,
      .inputs = {},
      .outputs = {},
      .exe = vm_push,
    },
    [OP_POP] = {
      .code = OP_POP,
      .name = "pop",
      .has_operand = true,
      .inputs = {},
      .outputs = {},
      .exe = vm_pop,
    },
    [OP_DUP] = {
      .code = OP_DUP,
      .name = "dup",
      .has_operand = true,
      .inputs = {TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_dup,
    },
    [OP_SWAP] = {
      .code = OP_SWAP,
      .name = "swap",
      .has_operand = true,
      .inputs = {TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_swap,
    },
    [OP_ADD] = {
      .code = OP_ADD,
      .name = "add",
      .has_operand = false,
      .inputs = {TYPE_INT, TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_add,
    },
    [OP_SUB] = {
      .code = OP_SUB,
      .name = "sub",
      .has_operand = false,
      .inputs = {TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_sub,
    },
    [OP_MULT] = {
      .code = OP_MULT,
      .name = "mult",
      .has_operand = false,
      .inputs = {TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_mult,
    },
    [OP_DIV] = {
      .code = OP_DIV,
      .name = "div",
      .has_operand = false,
      .inputs = {TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_div,
    },
    [OP_EQ] = {
      .code = OP_EQ,
      .name = "eq",
      .has_operand = false,
      .inputs = {TYPE_INT},
      .outputs = {TYPE_INT},
      .exe = vm_eq,
    },
    [OP_JMP] = {
      .code = OP_JMP,
      .name = "jmp",
      .has_operand = true,
      .inputs = {},
      .outputs = {},
      .exe = vm_jmp,
    },
    [OP_JNZ] = {
      .code = OP_JNZ,
      .name = "jnz",
      .has_operand = true,
      .inputs = {},
      .outputs = {},
      .exe = vm_jnz,
    },
    [OP_JZ] = {
      .code = OP_JZ,
      .name = "jz",
      .has_operand = true,
      .inputs = {},
      .outputs = {},
      .exe = vm_jz,
    },
    [OP_ALLOC] = {
      .code = OP_ALLOC,
      .name = "alloc",
      .has_operand = true,
      .inputs = {},
      .outputs = {TYPE_MEM_ADDRESS},
      .exe = vm_alloc,
    },
    [OP_WRITE] = {
      .code = OP_WRITE,
      .name = "write",
      .has_operand = true,
      .inputs = {TYPE_MEM_ADDRESS, TYPE_INT},
      .outputs = {},
      .exe = vm_write,
    },
    [OP_READ] = {
      .code = OP_READ,
      .name = "read",
      .has_operand = true,
      .inputs = {TYPE_MEM_ADDRESS},
      .outputs = {TYPE_INT},
      .exe = vm_read,
    },
    [OP_RET] = {
      .code = OP_RET,
      .name = "ret",
      .has_operand = false,
      .inputs = {TYPE_INST_POS},
      .outputs = {},
      .exe = vm_ret,
    },
    [OP_NATIVE] = {
      .code = OP_NATIVE,
      .name = "native",
      .has_operand = true,
      .exe = vm_native,
    },
};

Result native_write(VM *vm) {
  if (vm->stack.count < 2)
    return RESULT_ERROR_STACK_UNDERFLOW;

  uint64_t mem = vm->stack.items[vm->stack.count - 2].as_u64;
  uint64_t count = vm->stack.items[vm->stack.count - 1].as_u64;

  fwrite(&vm->memory[mem], sizeof(vm->memory[mem]), count, stdout);

  vm->stack.count -= 2;

  return RESULT_OK;
}

const char *result_to_str(Result r) {
  switch (r) {
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
  case RESULT_ERROR_ILLEGAL_NATIVE:
    return "RESULT_ERROR_ILLEGAL_NATIVE";
  default:
    assert(0 && "UNREACHABLE: result_to_str");
  }
}
