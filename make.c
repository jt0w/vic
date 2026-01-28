#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>
#include <stdbool.h>

#define build_dir "build/"
#define src_dir "src/"
#define c_std  "c23"

#define example_dir "examples/"

typedef enum {
  MODE_RELEASE,
  MODE_DEBUG,
} CompMode;

static CompMode MODE = {0};

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
    {
      .name = "devasm",
      .src = src_dir "devasm/devasm.c ",
      .out = build_dir "devasm",
    },
    {
      .name = "vam",
      .src = src_dir "vam/vam.c ",
      .out = build_dir "vam",
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
  {
    .name = "mem",
    .src = example_dir "mem.vasm",
    .out = example_dir "mem.vb",
  },
  {
    .name = "inc",
    .src = example_dir "inc.vasm",
    .out = example_dir "inc.vb",
  },
  {
    .name = "hello_world",
    .src = example_dir "hello_world.vasm",
    .out = example_dir "hello_world.vb",
  },
};

int build_tool(Tool tool) {
  Cmd cmd = {0};
  cmd_push(&cmd, "gcc", "-std="c_std, "-I./src/common", "./src/common/vm.c");
  cmd_push(&cmd, "-Wall","-Wextra","-Wswitch-enum", "-pedantic", "-ggdb");
  if (MODE == MODE_DEBUG) cmd_push(&cmd, "-DDEBUG_MODE");
  cmd_push(&cmd, tool.src);
  cmd_push(&cmd, "-o", tool.out);
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
  Flag mode_flag = parse_str_flag("-mode", "-m", "release");
  if (strcmp(mode_flag.as.str, "release") == 0) {
    MODE = MODE_RELEASE;
  } else if (strcmp(mode_flag.as.str, "debug") == 0) {
    MODE = MODE_DEBUG;
  } else {
    log(ERROR, "Uknown compilation mode `%s`", mode_flag.as.str);
    return 1;
  }

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
