#ifndef PREPROC_H
#define PREPROC_H

#include "lexer.h"

#include <unistd.h>
#include <limits.h>

typedef struct {
  StringBuilder *input;
  char c;
} PreProc;

void preproc_proc(PreProc *p, StringBuilder *sb);
#endif // endif PREPROC_H
