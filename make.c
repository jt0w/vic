#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h> 
#include <stdbool.h> 

#define build_dir "build/"
#define src_dir   "src/"

typedef struct {
  char *name;
  char *out;
  char *src;
} Tool;

const Tool TOOLS[] = {
  {
    .name = "vic",
    .src = src_dir "vic/vic.c",
    .out = build_dir "vic"
  },
  {
    .name = "vism",
    .src = src_dir "vism/vism.c " src_dir "vism/lexer.c ",
    .out = build_dir "vism"
  },
};

int build_tool(Tool tool) {
  Cmd cmd = {0};
  cmd_push(&cmd, "gcc", "-std=c99", "-I./src/common");
  cmd_push(&cmd, "-Wall", /*"-pedantic"*/ "-ggdb");
  cmd_push(&cmd, "-o", tool.out);
  cmd_push(&cmd, tool.src);
  return cmd_exec(&cmd);
}

int main(int argc, char **argv) {
  rebuild_file(argv, argc);
  if (create_dir(build_dir) != 0) 
    return 1;

  shift(argv, argc);
  if (argc > 0)  {
    char *arg = shift(argv, argc);
    bool valid_arg = false;
    for (size_t i = 0; i < sizeof(TOOLS) / sizeof(TOOLS[0]); ++i) {
      if (strcmp(arg, TOOLS[i].name) == 0) {
        build_tool(TOOLS[i]);
        Cmd cmd = {0};
        cmd_push(&cmd, TOOLS[i].out);
        while (argc > 0)
          cmd_push(&cmd, shift(argv, argc));
        valid_arg = true;
        if (cmd_exec(&cmd) != 0)
          return 1;
        break;
      }
    }

    if (!valid_arg) {
      log(CHIMERA_ERROR, "Not a valid tool %s", arg);
      return 0;
    }
    
  } else {
    StringBuilder sb = {0};
    for (size_t i = 0; i < sizeof(TOOLS) / sizeof(TOOLS[0]); ++i) {
      log(CHIMERA_INFO, "Compiling %s", TOOLS[i].name);
      if (build_tool(TOOLS[i]) == 0) 
        log(CHIMERA_INFO, "Compiled %s", TOOLS[i].name);
      else 
        log(CHIMERA_ERROR, "Error while compiling %s", TOOLS[i].name);
    }
  }
}
