#include <stdlib.h>
#include <string.h>

#include "cmdline.h"

static int is_prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

static void unquote(char *str) {
    if (str[0] == '\"' && str[strlen(str) - 1] == '\"') {
        int len = strlen(str);
        for (int i = 0; i < len - 2; ++i) {
            str[i] = str[i + 1];
        }
        str[len - 2] = '\0';
    }
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

    unquote(result->directory);

    // Any remaining arguments are the program to RUN.
    for (; i < argc; ++i) {
        if (result->run_cmd[0] == '\0') {
            strcat(result->run_cmd, "RUN \"");
            strcat(result->run_cmd, argv[i]);
            strcat(result->run_cmd, "\"");
            if (i != argc - 1) strcat(result->run_cmd, ",");
        } else {
            strcat(result->run_cmd, " ");
            strcat(result->run_cmd, argv[i]);
        }
    }

    if (result->interactive == 255) {
        result->interactive = (result->run_cmd[0] == '\0');
    }

    return 0;
}

void cmdline_print_usage(void (*println)(const char *)) {

}
