#include "helpers.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <wordexp.h>

FILE *maybe_fopen(const char *filename, const char *mode)
{
    if (strcmp(filename, "-") == 0) {
        if (*mode == 'w' || *mode == 'a') {
            return stdout;
        } else if (*mode == 'r') {
            return stdin;
        } else {
            errno = EINVAL;
            return NULL;
        }
    } else {
        return fopen(filename, mode);
    }
}

int maybe_set_stdout(const char *filename)
{
    if (strcmp(filename, "-") == 0) {
        return STDOUT_FILENO;
    } else {
        int new_stdout = open(filename, O_WRONLY | O_TRUNC);
        int saved_stdout;
        if (new_stdout < 0) {
            return -1;
        }

        saved_stdout = dup(STDOUT_FILENO);
        dup2(new_stdout, STDOUT_FILENO);
        return saved_stdout;
    }
}

void maybe_restore_stdout(int saved_stdout)
{
    if (saved_stdout != STDOUT_FILENO && saved_stdout > 0) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}

void maybe_fclose(FILE *file)
{
    if (file != stdout && file != stdin) {
        fclose(file);
    }
}
