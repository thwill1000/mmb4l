#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"
#include "cstring.h"
#include "utility.h"

static int is_prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

int cmdline_parse(int argc, const char *argv[], CmdLineArgs *result) {

    // TODO: should perhaps be rewritten to use getopt().
    // TODO: guard against string overflow.

    memset(result, 0, sizeof(CmdLineArgs));
    result->interactive = 255;

    // Looking for flags.
    int i = 1;
    for (; i < argc && argv[i][0] == '-'; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--directory") == 0) {
            if (i == argc - 1) {
                // Report an error.
                return -1;
            } else {
                strcpy(result->directory, argv[++i]);
            }
        } else if (is_prefix("-d=", argv[i])) {
            strcpy(result->directory, argv[i] + strlen("-d="));
        } else if (is_prefix("--directory=", argv[i])) {
            strcpy(result->directory, argv[i] + strlen("--directory="));
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            result->help = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            result->interactive = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            result->version = 1;
        } else {
            // Report an error.
            return -1;
        }
    }

    cstring_unquote(result->directory);

    // Any remaining arguments are the program to RUN.
    for (; i < argc; ++i) {
        if (result->run_cmd[0] == '\0') {
            cstring_cat(result->run_cmd, "RUN \"", sizeof(result->run_cmd));
            cstring_cat(result->run_cmd, argv[i], sizeof(result->run_cmd));
            cstring_cat(result->run_cmd, "\"", sizeof(result->run_cmd));
            if (i != argc - 1) cstring_cat(result->run_cmd, ",", sizeof(result->run_cmd));
        } else {
            cstring_cat(result->run_cmd, " ", sizeof(result->run_cmd));
            cstring_cat(result->run_cmd, argv[i], sizeof(result->run_cmd));
        }
    }

    if (result->interactive == 255) {
        result->interactive = (result->run_cmd[0] == '\0');
    }

    return 0;
}

void cmdline_print_usage() {
    fprintf(stderr, "Usage: mmbasic [OPTION]... [<file.bas>]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "If no <file.bas> is specified then starts the interpreter in interactive mode.\n");
    fprintf(stderr, "If <file.bas> is specified then starts the interpreter in batch mode, runs the\n");
    fprintf(stderr, "specified program and exits when the program ends or reports an error.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -d <path>          start in the specified directory.\n");
    fprintf(stderr, "  --directory <path>\n");
    fprintf(stderr, "  -h, --help         display this help text and exit.\n");
    fprintf(stderr, "  -i, --interactive  if started with a <file.bas> then return to the MMBasic\n");
    fprintf(stderr, "                     prompt when the program ends or reports an error instead\n");
    fprintf(stderr, "                     of automatically exiting.\n");
    fprintf(stderr, "  -v, --version      display version and copyright and exit.\n");
}
