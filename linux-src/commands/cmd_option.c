#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/codepage.h"
#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/options.h"
#include "../common/parse.h"
#include "../common/utility.h"

static void save_options() {
    MmResult result = options_save(&mmb_options, OPTIONS_FILE_NAME);
    if (FAILED(result)) {
        char buf[STRINGSIZE];
        sprintf(buf, "Warning: failed to save options: %s", mmresult_to_string(result));
        MMPrintString(buf);
    }
}

static void option_base(char *p) {
    if (DimUsed) error("Must be before DIM or LOCAL");
    mmb_options.base = getint(p, 0, 1);
}

static void option_break(char *p) {
    mmb_options.break_key = getinteger(p);
}

static void option_case(char *p) {
    if (checkstring(p, "TITLE")) {
        mmb_options.list_case = kTitle;
    } else if (checkstring(p, "LOWER")) {
        mmb_options.list_case = kLower;
    } else if (checkstring(p, "UPPER")) {
        mmb_options.list_case = kUpper;
    } else {
        ERROR_UNKNOWN_OPTION;
    }

    save_options();
}

static void option_codepage(char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;
    char *codepage_name = getCstring(argv[0]);
    bool found = false;
    for (int i = 0; CODEPAGE_NAMES[i]; i++) {
        if (SUCCEEDED(codepage_set(&mmb_options, codepage_name))) {
            found = true;
            break;
        }
    }
    if (!found) ERROR_INVALID("codepage");
}

static void option_console(char *p) {
    // Allowed but ignored.
}

static void option_default(char *p) {
    if (checkstring(p, "INTEGER")) {
        mmb_options.default_type = T_INT;
    } else if (checkstring(p, "FLOAT")) {
        mmb_options.default_type = T_NBR;
    } else if (checkstring(p, "STRING")) {
        mmb_options.default_type = T_STR;
    } else if (checkstring(p, "NONE")) {
        mmb_options.default_type = T_NOTYPE;
    } else {
        ERROR_UNKNOWN_OPTION;
    }
}

static void get_value(const char *pstart, char *value) {
    char *p = (char *) pstart;
    skipspace(p);
    while (*p && *p != ' ' && *p != ',' && *p != '\'' && *p != '(') {
        *value++ = *p++;
    }
    *value = '\0';
}

static void option_editor(char *p) {
    // Is the editor value one of the standard literals ?
    char value[STRINGSIZE];
    get_value(p, value);
    if (SUCCEEDED(options_set(&mmb_options, "editor", value))) {
        save_options();
        return;
    }

    // Is the editor value a string that evaluates to one of the standard literals ?
    char *s = getCstring(p);
    if (!s) ERROR_INVALID_OPTION_VALUE;
    if (SUCCEEDED(options_set(&mmb_options, "editor", s))) {
        save_options();
        return;
    }

    // Is the editor value a string to treat as a system command to launch the editor ?
    sprintf(value, "\"%s\"", s);
    if (SUCCEEDED(options_set(&mmb_options, "editor", value))) {
        save_options();
        return;
    }

    ERROR_INVALID_OPTION_VALUE;
}

static void option_explicit(char *p) {
    getargs(&p, 1, ",");
    if (argc == 1) {
        mmb_options.explicit_type = parse_bool(argv[0]);
    } else {
        mmb_options.explicit_type = true;
    }
}

/** Handle OPTION F1..12 value$ */
static void option_fn_key(char *p) {
    char buf[8];
    int f = 1;
    while (f != 13) {
        sprintf(buf, "F%d", f);
        if (checkstring(p, buf)) break;
        f++;
    }

    if (f == 13) ERROR_UNKNOWN_OPTION;

    // Skip the 'F' and the number.
    p += (f < 10) ? 2 : 3;
    char *value = getCstring(p);
    sprintf(buf, "f%d", f);

    MmResult result = options_set(&mmb_options, buf, value);
    if (SUCCEEDED(result)) {
        save_options();
    } else {
        error_system(result);
    }
}

static void option_list_item(char *name, char *value) {
    MMPrintString("Option ");
    MMPrintString(name);
    MMPrintString(" ");
    MMPrintString(value);
    MMPrintString("\r\n");
}

void option_list(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;

    char buf[STRINGSIZE + 2];

    sprintf(buf, "%d", mmb_options.base);
    option_list_item("Base", buf);

    sprintf(buf, "%d", mmb_options.break_key);
    option_list_item("Break", buf);

    options_list_case_to_string(mmb_options.list_case, buf);
    option_list_item("Case", buf);

    if (FAILED(codepage_to_string(mmb_options.codepage, buf))) ERROR_INTERNAL_FAULT;
    option_list_item("CodePage", buf);

    options_console_to_string(mmb_options.console, buf);
    option_list_item("Console", buf);

    options_type_to_string(mmb_options.default_type, buf);
    option_list_item("Default", buf);

    options_editor_to_string(mmb_options.editor, buf);
    option_list_item("Editor", mmb_options.editor);

    options_explicit_to_string(mmb_options.explicit_type, buf);
    option_list_item("Explicit", buf);

    for (int i = 0; i < OPTIONS_NUM_FN_KEYS; ++i) {
        char option[8];
        sprintf(option, "F%d", i + 1);
        options_fn_key_to_string(mmb_options.fn_keys[i], buf);
        option_list_item(option, buf);
    }

    options_resolution_to_string(mmb_options.resolution, buf);
    option_list_item("Resolution", buf);

    sprintf(buf, "\"%s\"", mmb_options.search_path);
    option_list_item("Search Path", buf);

    sprintf(buf, "%d", mmb_options.tab);
    option_list_item("Tab", buf);

    MMPrintString("\r\n");
}

static void option_resolution(char *p) {
    char *p2 = NULL;
    if ((p2 = checkstring(p, "CHARACTER"))) {
        mmb_options.resolution = kCharacter;
    } else if ((p2 = checkstring(p, "PIXEL"))) {
        mmb_options.resolution = kPixel;
    }

    if (!p2 || !parse_is_end(p2)) ERROR_UNKNOWN_OPTION;
}

static void option_search_path(char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;
    char *path = getCstring(argv[0]);

    MmResult result = options_set(&mmb_options, "search-path", path);
    switch (result) {
        case kOk:
            save_options();
            break;
        case kFileNotFound:
            ERROR_DIRECTORY_NOT_FOUND;
            break;
        default:
            ERROR_INVALID_OPTION_VALUE;
            break;
    }
}

static void option_tab(char *p) {
    if (checkstring(p, "2")) {
        mmb_options.tab = 2;
    } else if (checkstring(p, "4")) {
        mmb_options.tab = 4;
    } else if (checkstring(p, "8")) {
        mmb_options.tab = 8;
    } else {
        ERROR_UNKNOWN_OPTION;
    }

    save_options();
}

void cmd_option(void) {
    char *p;
    if ((p = checkstring(cmdline, "BASE"))) {
        option_base(p);
    } else if ((p = checkstring(cmdline, "BREAK"))) {
        option_break(p);
    } else if ((p = checkstring(cmdline, "CASE"))) {
        option_case(p);
    } else if ((p = checkstring(cmdline, "CODEPAGE"))) {
        option_codepage(p);
    } else if ((p = checkstring(cmdline, "CONSOLE"))) {
        option_console(p);
    } else if ((p = checkstring(cmdline, "DEFAULT"))) {
        option_default(p);
    } else if ((p = checkstring(cmdline, "EDITOR"))) {
        option_editor(p);
    } else if ((p = checkstring(cmdline, "EXPLICIT"))) {
        option_explicit(p);
    } else if ((p = checkstring(cmdline, "LIST"))) {
        option_list(p);
    } else if ((p = checkstring(cmdline, "RESOLUTION"))) {
        option_resolution(p);
    } else if ((p = checkstring(cmdline, "SEARCH PATH"))) {
        option_search_path(p);
    } else if ((p = checkstring(cmdline, "TAB"))) {
        option_tab(p);
    } else if (toupper(*cmdline) == 'F' && isdigit(*(cmdline + 1))) {
        option_fn_key(cmdline);
    } else {
        ERROR_UNKNOWN_OPTION;
    }
}
