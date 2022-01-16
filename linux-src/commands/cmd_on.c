#include <ctype.h>

#include "../common/error.h"
#include "../common/interrupt.h"
#include "../common/version.h"

int g_key_complete = 0;

static void on_error_abort(char* p) {
    OptionErrorSkip = 0;
}

static void on_error_clear(char *p) {
    MMerrno = 0;
    *MMErrMsg = 0;
}

static void on_error_ignore(char *p) {
    on_error_clear(p);
    OptionErrorSkip = -1;
}

static void on_error_skip(char *p) {
    on_error_clear(p);
    OptionErrorSkip = (*p == 0 || *p == '\'') ? 2 : getint(p, 1, 10000) + 1;
}

static void on_error(char *p) {
    char *p2;
    if ((p2 = checkstring(p, "ABORT"))) {
        on_error_abort(p2);
    } else if ((p2 = checkstring(p, "CLEAR"))) {
        on_error_clear(p2);
    } else if ((p2 = checkstring(p, "IGNORE"))) {
        on_error_ignore(p2);
    } else if ((p2 = checkstring(p, "SKIP"))) {
        on_error_skip(p2);
    } else {
        ERROR_SYNTAX;
    }
}

/**
 * ON KEY target
 * ON KEY ASCIIcode, target
 */
static void on_key(char *p) {
    getargs(&p, 3, ",");
    if (argc == 1) {
        if (*argv[0] == '0' && !isdigit(*(argv[0] + 1))) {
            interrupt_disable_any_key();
        } else {
            interrupt_enable_any_key(GetIntAddress(argv[0]));
        }
    } else {
        int key = getint(argv[0], 0, 255);
        if (key == 0) {
            interrupt_disable_specific_key();
        } else {
            if (*argv[2] == '0' && !isdigit(*(argv[2] + 1))) {
                interrupt_disable_specific_key();
            } else {
                interrupt_enable_specific_key(key, GetIntAddress(argv[2]));
            }
        }
    }
}

/** ON nbr GOTO | GOSUB target[,target, target,...] */
static void on_number(char *p) {
    int r;
    char ss[4] = {tokenGOTO, tokenGOSUB, ',', 0};
    {  // start a new block
        getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ss);  // getargs macro must be the first executable stmt in a block
        if (argc < 3 || !(*argv[1] == ss[0] || *argv[1] == ss[1])) error("Syntax");
        if (argc % 2 == 0) error("Syntax");

        r = getint(argv[0], 0, 255);  // evaluate the expression controlling the statement
        if (r == 0 || r > argc / 2) return;  // microsoft say that we just go on to the next line

        if (*argv[1] == ss[1]) {
            // this is a GOSUB, same as a GOTO but we need to first push the
            // return pointer
            if (gosubindex >= MAXGOSUB) error("Too many nested GOSUB");
            errorstack[gosubindex] = CurrentLinePtr;
            gosubstack[gosubindex++] = nextstmt;
            LocalIndex++;
        }

        if (isnamestart(*argv[r * 2]))
            nextstmt = findlabel(argv[r * 2]);  // must be a label
        else
            nextstmt = findline(getinteger(argv[r * 2]), true);  // try for a line number
    }
    IgnorePIN = false;
}

void cmd_on(void) {
    char *p;
    if ((p = checkstring(cmdline, "ERROR"))) {
        on_error(p);
    } else if ((p = checkstring(cmdline, "KEY"))) {
        on_key(p);
    } else {
        on_number(p);
    }
}
