#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>
#include <stdbool.h>

#define build_dir "build/"
#define src_dir "src/"
#define c_std  "c23"

#define example_dir "examples/"

typedef struct {
  char *name;
  char *out;
  char *src;
} Tool;

const Tool TOOLS[] = {
    {
      .name = "vl",
      .src = src_dir "vl/vl.c",
      .out = build_dir "vl"
    },
    {
      .name = "vasm",
      .src = src_dir "vasm/vasm.c "
             src_dir "vasm/lexer.c "
             src_dir "vasm/parser.c "
             src_dir "vasm/gen.c ",
     .out = build_dir "vasm"
    },
};

const Tool EXAMPLES[] = {
  {
    .name = "nice",
    .src = example_dir "nice.vasm",
    .out = example_dir "nice.vb",
  },
  {
    .name = "counter",
    .src = example_dir "counter.vasm",
    .out = example_dir "counter.vb",
  },
};

int build_tool(Tool tool) {
  Cmd cmd = {0};
  cmd_push(&cmd, "gcc", "-std="c_std, "-I./src/common");
  cmd_push(&cmd, "-Wall","-Wextra","-Wswitch-enum", "-pedantic", "-ggdb");
  cmd_push(&cmd, "-o", tool.out);
  cmd_push(&cmd, tool.src);
  return cmd_exec(&cmd);
}

int build_example(Tool example) {
  Cmd cmd = {0};
  cmd_push(&cmd, "build/vasm");
  cmd_push(&cmd, "-i",  example.src);
  cmd_push(&cmd, "-o",  example.out);
  return cmd_exec(&cmd);
}

int main(int argc, char **argv) {
  rebuild_file(argv, argc);
  create_dir(build_dir);
  shift(argv, argc);
  Flag build_examples = parse_boolean_flag("-build_examples", "-be", false);

  for (size_t i = 0; i < sizeof(TOOLS) / sizeof(TOOLS[0]); ++i) {
    log(CHIMERA_INFO, "Compiling %s", TOOLS[i].name);
    if (build_tool(TOOLS[i]) != 0)
      log(CHIMERA_INFO, "Compiled %s", TOOLS[i].name);
    else
      log(CHIMERA_ERROR, "Error while compiling %s", TOOLS[i].name);
  }

  if (build_examples.as.boolean) {
    for (size_t i = 0; i < sizeof(EXAMPLES) / sizeof(EXAMPLES[0]); ++i) {
      log(CHIMERA_INFO, "Compiling %s", EXAMPLES[i].name);
      if (build_example(EXAMPLES[i]) != 0)
        log(CHIMERA_INFO, "Compiled %s", EXAMPLES[i].name);
      else
        log(CHIMERA_ERROR, "Error while compiling %s", EXAMPLES[i].name);
    }
  }
}
