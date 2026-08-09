#include "preproc.h"

void pre_adv(PreProc *p) {
  if (p->input->count > 0) {
    p->c = *(++p->input->items);
    p->input->count--;
  }
}

void pre_skip(PreProc *p, const char c) {
  if (p->c != c) {
    printf("ERROR: expected %c but got `%c`\n", c, p->c);
    exit(1);
  }
  pre_adv(p);
}

void pre_skip_space(PreProc *p) {
  while (isspace(p->c)) {
    pre_adv(p);
  }
}

void preproc_proc(PreProc *p, StringBuilder *sb) {
  while (p->c != '%' && p->input->count > 0) {
    da_push(sb, p->c);
    pre_adv(p);
  }
  if (p->input->count == 0)
    return;
  pre_skip(p, '%');
  StringBuilder dir = {0};
  while (isalnum(p->c)) {
    da_push(&dir, p->c);
    pre_adv(p);
  }
  da_push(&dir, '\0');
  if (strcmp(dir.items, "use") == 0) {
    pre_skip_space(p);
    pre_skip(p, '"');
    StringBuilder filename = {0};
    while (p->c != '"') {
      da_push(&filename, p->c);
      pre_adv(p);
    }
    da_push(&filename, '\0');

    char buf[4096];
    char *real_filepath = realpath(filename.items, buf);
    if (real_filepath == NULL) {
      perror("realpath");
      exit(1);
    }
    StringBuilder content = {0};
    if (!read_file(real_filepath, &content)) {
      log(ERROR, "could not read file: %s", real_filepath);
      exit(1);
    }
    pre_skip(p, '"');
    PreProc p2 = {
        .input = &content,
        .c = *content.items,
    };
    preproc_proc(&p2, sb);
  } else {
    log(ERROR, "unknown preproccessing directive `%s`", dir.items);
    exit(1);
  }
  preproc_proc(p, sb);
}
