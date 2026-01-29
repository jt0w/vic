#define CHIMERA_STRIP_PREFIX
#define CHIMERA_IMPLEMENTATION
#include <chimera.h>

#include <vm.h>

char *print_type(Type t);

char *make_type_list(Type types[], size_t cap);

int main(int argc, char **argv) {
  const char *program = shift(argv, argc);
  if (argc != 1) {
    fprintln(stderr, "USAGE: %s <instruction>", program);
    log(ERROR, "Did not provide instruction");
    return 1;
  }

  const char *inst = shift(argv, argc);

  for (size_t i = 0; i < OPCODE_COUNT; ++i) {
    if (strcmp(inst, INST_MAP[i].name) == 0) {
      Instruction_Mapping m = INST_MAP[i];
      println("%s: ", m.name);
      println("   code: %d", m.code);
      println("   operand: %s", m.has_operand ? "true" : "false");
      println("   inputs: [%s]", make_type_list(m.inputs, INPUTS_CAP));
      println("   outputs: [%s]", make_type_list(m.outputs, OUTPUTS_CAP));
      return 0;
    }
  }

  log(ERROR, "`%s` is not a valid instruction!", inst);
  return 1;
}

char *print_type(Type t) {
  switch (t) {
  case TYPE_INT:
    return temp_sprintf("int");
  case TYPE_INST_POS:
    return temp_sprintf("inst_pos");
  case TYPE_MEM_ADDRESS:
    return temp_sprintf("mem_address");
  case TYPE_NOTHING:
  default:
    log(ERROR, "Unexpected Type");
    abort();
  }
}

char *make_type_list(Type types[], size_t cap) {
  StringBuilder sb = {0};
  for (size_t i = 0; i < cap; ++i) {
    if (types[i] == TYPE_NOTHING)
      break;
    if (i != 0)
      da_push_buf(&sb, ", ");
    char *t = print_type(types[i]);
    da_push_buf(&sb, t);
  }
  da_push(&sb, '\0');
  return sb.items;
}
