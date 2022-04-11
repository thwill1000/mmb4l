#include <stdbool.h>

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/utility.h"

static void cmd_console_background(char *p) {
    if (parse_is_end(p)) ERROR_SYNTAX;
    int colour = parse_colour(p, 0);
    if (colour == -1) ERROR_INVALID("background colour");
    console_background(colour);
}

static void cmd_console_bell(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    console_bell();
}

static void cmd_console_clear(char *p) {
    getargs(&p, 1, ",");
    if (argc != 0) ERROR_SYNTAX;
    console_clear();
}

static void cmd_console_foreground(char *p) {
    if (parse_is_end(p)) ERROR_SYNTAX;
    int colour = parse_colour(p, 1);
    if (colour == -1) ERROR_INVALID("foreground colour");
    console_foreground(colour);
}

static void cmd_console_get_cursor(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    void *px = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARGUMENT_NOT_INTEGER("X");
    }

    void *py = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARGUMENT_NOT_INTEGER("Y");
    }

    int x, y;
    if (FAILED(console_get_cursor_pos(&x, &y, 10000))) {
        ERROR_COULD_NOT("determine cursor position");
    }

    *((int64_t *) px) = x;
    *((int64_t *) py) = y;
}

static void cmd_console_get_size(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    void *pwidth = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARGUMENT_NOT_INTEGER("WIDTH");
    }

    void *pheight = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)
            || vartbl[VarIndex].dims[0] != 0) {
        ERROR_ARGUMENT_NOT_INTEGER("HEIGHT");
    }

    int width, height;
    if (FAILED(console_get_size(&width, &height))) {
        ERROR_COULD_NOT("determine console size");
    }

    *((int64_t *) pwidth) = width;
    *((int64_t *) pheight) = height;
}

static void cmd_console_hide_cursor(char *p) {
    getargs(&p, 1, ",");
    int hide = 1;
    if (argc == 1) {
        hide = parse_bool(argv[0]);
    }
    console_show_cursor(hide == 1 ? 0 : 1);
}

static void cmd_console_home(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    console_home_cursor();
}

static void cmd_console_invert(char *p) {
    getargs(&p, 1, ",");
    int invert = 1;
    if (argc == 1) {
        invert = parse_bool(argv[0]);
    }
    console_invert(invert);
}

static void cmd_console_reset(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    console_reset();
}

#define MAX_CURSOR_X  1023
#define MAX_CURSOR_Y  1023

static void cmd_console_set_cursor(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    int x = getint(argv[0], 0, MAX_CURSOR_X);
    int y = getint(argv[2], 0, MAX_CURSOR_Y);
    console_set_cursor_pos(x, y);
}

static void cmd_console_set_size(char *p) {
    char *p2 = NULL;
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
        if (FAILED(console_get_size(&old_width, &old_height))) {
            ERROR_COULD_NOT("resize console");
        }
        width = max(width, old_width);
        height = max(height, old_height);
    }

    if (FAILED(console_set_size(width, height))) {
        ERROR_COULD_NOT("resize console");
    }
}

static void cmd_console_set_title(char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    console_set_title(getCstring(argv[0]));
}

static void cmd_console_show_cursor(char *p) {
    getargs(&p, 1, ",");
    int show = 1;
    if (argc == 1) {
        show = parse_bool(argv[0]);
    }
    console_show_cursor(show);
}

void cmd_console(void) {
    char *p;
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
        ERROR_UNKNOWN_SUBCOMMAND;
    }
}
