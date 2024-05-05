/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_console.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <stdbool.h>

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/utility.h"

static void cmd_console_background(const char *p) {
    if (parse_is_end(p)) ERROR_SYNTAX;
    int colour = parse_colour(p, 0);
    if (colour == -1) ERROR_INVALID("background colour");
    console_background(colour);
}

static void cmd_console_bell(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    console_bell();
}

static void cmd_console_clear(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 0) ERROR_SYNTAX;
    console_clear();
}

static void cmd_console_foreground(const char *p) {
    if (parse_is_end(p)) ERROR_SYNTAX;
    int colour = parse_colour(p, 1);
    if (colour == -1) ERROR_INVALID("foreground colour");
    console_foreground(colour);
}

static void cmd_console_get_cursor(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    void *px = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARG_NOT_INTEGER(1);
    }

    void *py = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARG_NOT_INTEGER(2);
    }

    int x, y;
    if (FAILED(console_get_cursor_pos(&x, &y, 10000))) {
        ERROR_COULD_NOT("determine cursor position");
    }

    *((int64_t *) px) = x;
    *((int64_t *) py) = y;
}

static void cmd_console_get_size(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    void *pwidth = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARG_NOT_INTEGER(1);
    }

    void *pheight = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARG_NOT_INTEGER(2);
    }

    int width, height;
    if (FAILED(console_get_size(&width, &height, 0))) {
        ERROR_UNKNOWN_TERMINAL_SIZE;
    }

    *((int64_t *) pwidth) = width;
    *((int64_t *) pheight) = height;
}

static void cmd_console_hide_cursor(const char *p) {
    getargs(&p, 1, ",");
    bool hide = true;
    if (argc == 1) {
        hide = parse_bool(argv[0]);
    }
    console_show_cursor(!hide);
}

static void cmd_console_home(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    console_home_cursor();
}

static void cmd_console_invert(const char *p) {
    getargs(&p, 1, ",");
    int invert = 1;
    if (argc == 1) {
        invert = parse_bool(argv[0]);
    }
    console_invert(invert);
}

static void cmd_console_reset(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    console_reset();
}

#define MAX_CURSOR_X  1023
#define MAX_CURSOR_Y  1023

static void cmd_console_set_cursor(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    int x = getint(argv[0], 0, MAX_CURSOR_X);
    int y = getint(argv[2], 0, MAX_CURSOR_Y);
    console_set_cursor_pos(x, y);
}

static void cmd_console_set_size(const char *p) {
    const char *p2 = NULL;
    bool at_least = false;
    if ((p2 = parse_check_string(p, "ATLEAST"))) at_least = true;
    if (!p2) p2 = p;

    int width, height;
    { // getargs() should be first executable statement in a block.
        getargs(&p2, 3, ",");
        if (argc != 3) ERROR_ARGUMENT_COUNT;
        width = getint(argv[0], 0, MAX_CURSOR_X + 1);
        height = getint(argv[2], 0, MAX_CURSOR_Y + 1);
    }

    if (at_least) {
        int old_width = 0;
        int old_height = 0;
        if (FAILED(console_get_size(&old_width, &old_height, 0))) {
            ERROR_COULD_NOT("resize console");
        }
        width = max(width, old_width);
        height = max(height, old_height);
    }

    if (FAILED(console_set_size(width, height))) {
        ERROR_COULD_NOT("resize console");
    }
}

static void cmd_console_set_title(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    console_set_title(getCstring(argv[0]));
}

static void cmd_console_show_cursor(const char *p) {
    getargs(&p, 1, ",");
    bool show = 1;
    if (argc == 1) {
        show = parse_bool(argv[0]);
    }
    console_show_cursor(show);
}

void cmd_console(void) {
    const char *p;
    if ((p = parse_check_string(cmdline, "BACKGROUND"))) {
        cmd_console_background(p);
    } else if ((p = parse_check_string(cmdline, "BELL"))) {
        cmd_console_bell(p);
    } else if ((p = parse_check_string(cmdline, "CLEAR"))) {
        cmd_console_clear(p);
    } else if ((p = parse_check_string(cmdline, "GETCURSOR"))) {
        cmd_console_get_cursor(p);
    } else if ((p = parse_check_string(cmdline, "GETSIZE"))) {
        cmd_console_get_size(p);
    } else if ((p = parse_check_string(cmdline, "FOREGROUND"))) {
        cmd_console_foreground(p);
    } else if ((p = parse_check_string(cmdline, "HIDECURSOR"))) {
        cmd_console_hide_cursor(p);
    } else if ((p = parse_check_string(cmdline, "HOME"))) {
        cmd_console_home(p);
    } else if ((p = parse_check_string(cmdline, "INVERSE"))) {
        cmd_console_invert(p);
    } else if ((p = parse_check_string(cmdline, "INVERT"))) {
        cmd_console_invert(p);
    } else if ((p = parse_check_string(cmdline, "RESET"))) {
        cmd_console_reset(p);
    } else if ((p = parse_check_string(cmdline, "RESIZE"))) {
        cmd_console_set_size(p);
    } else if ((p = parse_check_string(cmdline, "SETCURSOR"))) {
        cmd_console_set_cursor(p);
    } else if ((p = parse_check_string(cmdline, "SETSIZE"))) {
        cmd_console_set_size(p);
    } else if ((p = parse_check_string(cmdline, "SETTITLE"))) {
        cmd_console_set_title(p);
    } else if ((p = parse_check_string(cmdline, "SHOWCURSOR"))) {
        cmd_console_show_cursor(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("CONSOLE");
    }
}
