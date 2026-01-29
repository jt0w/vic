#ifndef COMMON_H
#define COMMON_H
#include <vm.h>

#include "lexer.h"
#include "parser.h"
#include "gen.h"

void translate_file(const char *input_file, Gen *gen, Program *p);
#endif // endif COMMON_H
