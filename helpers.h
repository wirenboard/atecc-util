#pragma once

#include <stdio.h>
#include <stdlib.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

FILE *maybe_fopen(const char *filename, const char *mode);
void maybe_fclose(FILE *file);

int maybe_set_stdout(const char *filename);
void maybe_restore_stdout(int saved_stdout);
