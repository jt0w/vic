#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>
#include <stdbool.h>

#define build_dir "build/"
#define src_dir "src/"
#define c_std "c23"

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
  char *comp;
} Tool;

const Tool TOOLS[] = {
  {.name = "vl", .src = src_dir "vl/*.c", .out = build_dir "vl"},
  {.name = "vlc", .src = src_dir "vlc/*.c", .out = build_dir "vlc"},
  {.name = "vasm", .src = src_dir "vasm/*.c", .out = build_dir "vasm"},
  {
    .name = "devasm",
    .src = src_dir "devasm/*.c ",
    .out = build_dir "devasm",
  },
  {
    .name = "vam",
    .src = src_dir "vam/*.c ",
    .out = build_dir "vam",
  },
};

const Tool EXAMPLES[] = {
  {
    .name = "nice",
    .src = example_dir "vasm/nice.vasm",
    .out = example_dir "vasm/nice.vb",
  },
  {
    .name = "counter",
    .src = example_dir "vasm/counter.vasm",
    .out = example_dir "vasm/counter.vb",
  },
  {
    .name = "mem",
    .src = example_dir "vasm/mem.vasm",
    .out = example_dir "vasm/mem.vb",
  },
  {
    .name = "inc",
    .src = example_dir "vasm/inc.vasm",
    .out = example_dir "vasm/inc.vb",
  },
  {
    .name = "hello_world",
    .src = example_dir "vasm/hello_world.vasm",
    .out = example_dir "vasm/hello_world.vb",
  },
  {
    .name = "fib",
    .src = example_dir "vasm/fib.vasm",
    .out = example_dir "vasm/fib.vb",
  },
  {
	.name = "hello_world 2.0",
	.src = example_dir "vl/hello_world.vl",
	.out = example_dir "vl/hello_world.vb",
	.comp = "vlc",
  },
};

int build_tool(Tool tool) {
  Cmd cmd = {0};
  cmd_push(&cmd, "gcc", "-std=" c_std, "-I./src/common", "./src/common/vm.c");
  cmd_push(&cmd, "-Wall", "-Wextra", "-Wswitch-enum", "-pedantic", "-ggdb");
  if (MODE == MODE_DEBUG)
    cmd_push(&cmd, "-DDEBUG_MODE");
  cmd_push(&cmd, tool.src);
  cmd_push(&cmd, "-o", tool.out);
  return cmd_exec(&cmd);
}

int build_example(Tool example) {
  Cmd cmd = {0};
  if (example.comp == NULL) {
	cmd_push(&cmd, "build/vasm");
  }	else {
	cmd_push(&cmd, temp_sprintf("build/%s", example.comp));
  }
  cmd_push(&cmd, "-i", example.src);
  cmd_push(&cmd, "-o", example.out);
  return cmd_exec(&cmd);
}

int main(int argc, char **argv) {
  rebuild_file(argv, argc);
  create_dir(build_dir);
  shift(argv, argc);
  Flags flags = {0};
  Flag build_examples = parse_boolean_flag(flags, "-build_examples", "-be",
                                           false, "build examples");
  Flag mode_flag =
    parse_str_flag(flags, "-mode", "-m", "release", "compilation mode");

  Flag debugger_flag =
    parse_str_flag(flags, "-debugger", "-dbg", "gf2", "debugger");

  Flag debug_flag =
    parse_str_flag(flags, "-run-debug", "-rdbg", NULL, "run debugger");

  Flag build_flag = parse_str_flag(flags, "-build", "-b", NULL,
                                   "if set only that tool gets built");

  Flag help_flag =
    parse_boolean_flag(flags, "-help", "-h", false, "print help message");

  Flag ctags_flag =
    parse_boolean_flag(flags, "-ctags", "-ct", false,
                       "generate ctags (requires universal-ctags)");
  if (!flags_check(flags, argc)) {
    StringBuilder sb = {0};
    flags_err_str(flags, &sb, argv, argc);
    sb_push(&sb, '\0');
    log(ERROR, sb.items);
    da_free(sb);
    return 0;
  }
  if (help_flag.as.boolean) {
    print_flags_help(flags);
    return 0;
  }
  if (ctags_flag.as.boolean) {
    log(INFO, "Generating ctags ...");
    Cmd cmd = {0};
    cmd_push(&cmd, "ctags", "-eR", "*");
    if (!cmd_exec(&cmd)) {
      log(ERROR, "Failed to generate ctags");
      return 1;
    }
    log(INFO, "Generated ctags");
  }
  if (strcmp(mode_flag.as.str, "release") == 0) {
    MODE = MODE_RELEASE;
  } else if (strcmp(mode_flag.as.str, "debug") == 0) {
    MODE = MODE_DEBUG;
  } else {
    log(ERROR, "Uknown compilation mode `%s`", mode_flag.as.str);
    return 1;
  }

  bool failed = false;
  if (build_flag.as.str == NULL) {
    for (size_t i = 0; i < sizeof(TOOLS) / sizeof(TOOLS[0]); ++i) {
      log(CHIMERA_INFO, "Compiling %s", TOOLS[i].name);
      if (build_tool(TOOLS[i]) != 0)
        log(CHIMERA_INFO, "Compiled %s", TOOLS[i].name);
      else {
        log(CHIMERA_ERROR, "Error while compiling %s", TOOLS[i].name);
        failed = true;
      }
    }
  } else {
    bool found = false;
    for (size_t i = 0; i < sizeof(TOOLS) / sizeof(TOOLS[0]); ++i) {
      if (strcmp(TOOLS[i].name, build_flag.as.str) == 0) {
        found = true;
        log(CHIMERA_INFO, "Compiling %s", TOOLS[i].name);
        if (build_tool(TOOLS[i]) != 0)
          log(CHIMERA_INFO, "Compiled %s", TOOLS[i].name);
        else {
          log(CHIMERA_ERROR, "Error while compiling %s", TOOLS[i].name);
          failed = true;
        }
        break;
      }
    }
    if (!found) {
      log(ERROR, "Tool %s does not exist", build_flag.as.str);
      return 1;
    }
  }

  if (failed)
    return 1;

  if (build_examples.as.boolean) {
    for (size_t i = 0; i < sizeof(EXAMPLES) / sizeof(EXAMPLES[0]); ++i) {
      log(CHIMERA_INFO, "Compiling %s", EXAMPLES[i].name);
      if (build_example(EXAMPLES[i]) != 0)
        log(CHIMERA_INFO, "Compiled %s", EXAMPLES[i].name);
      else
        log(CHIMERA_ERROR, "Error while compiling %s", EXAMPLES[i].name);
    }
  }

  if (debug_flag.as.str != NULL) {
    Cmd c = {0};
    assert(debugger_flag.as.str != NULL);
    cmd_push(&c, debugger_flag.as.str);
    cmd_push(&c, debug_flag.as.str);
    if (!cmd_exec(&c))
      return 1;
  }
}
