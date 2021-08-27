#include "../common/version.h"

int g_key_complete = 0;
char *g_key_interrupt = NULL;
int g_key_select = 0;

static void on_error_abort(char* p) {
    OptionErrorSkip = 0;
}

static void on_error_clear(char *p) {
    MMerrno = 0;
    *MMErrMsg = 0;
}

static void on_error_ignore(char *p) {
    MMerrno = 0;
    *MMErrMsg = 0;
    OptionErrorSkip = -1;
}

static void on_error_skip(char *p) {
    MMerrno = 0;
    *MMErrMsg = 0;
    OptionErrorSkip = (*p == 0 || *p == '\'') ? 2 : getint(p, 1, 10000) + 1;
}

static void on_error(char *p) {
    char *p2;
    if (p2 = checkstring(p, "ABORT")) {
        on_error_abort(p2);
    } else if (p2 = checkstring(p, "CLEAR")) {
        on_error_clear(p2);
    } else if (p2 = checkstring(p, "IGNORE")) {
        on_error_ignore(p2);
    } else if (p2 = checkstring(p, "SKIP")) {
        on_error_skip(p2);
    } else {
        error("Syntax");
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
            OnKeyGOSUB = NULL;  // the program wants to turn the interrupt off
        } else {
            OnKeyGOSUB = GetIntAddress(argv[0]);  // get a pointer to the interrupt routine
            InterruptUsed = true;
        }
    } else {
        g_key_select = getint(argv[0], 0, 255);
        if (g_key_select == 0) {
            g_key_interrupt = NULL;  // the program wants to turn the interrupt off
        } else {
            if (*argv[2] == '0' && !isdigit(*(argv[2] + 1))) {
                g_key_interrupt = NULL;  // the program wants to turn the interrupt off
            } else {
                g_key_interrupt = GetIntAddress(argv[2]);  // get a pointer to the interrupt routine
                InterruptUsed = true;
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
    if (p = checkstring(cmdline, "ERROR")) {
        on_error(p);
    } else if (p = checkstring(cmdline, "KEY")) {
        on_key(p);
    } else {
        on_number(p);
    }
}
