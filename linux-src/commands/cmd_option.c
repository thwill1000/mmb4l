#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/codepage.h"
#include "../common/error.h"
#include "../common/options.h"
#include "../common/parse.h"
#include "../common/utility.h"

static void cmd_option_save() {
    OptionsResult result = options_save(&mmb_options, OPTIONS_FILE_NAME);
    if (FAILED(result)) {
        char buf[STRINGSIZE];
        switch (result) {
            case kOtherIoError:
                sprintf(buf, "Warning: failed to save options: %s (%d)", strerror(errno), errno);
                break;
            default:
                sprintf(buf, "Warning: failed to save options: %d)", result);
                break;
        }
        MMPrintString(buf);
    }
}

static void option_base(char *p) {
    if (DimUsed) error("Must be before DIM or LOCAL");
    OptionBase = getint(p, 0, 1);
}

static void option_break(char *p) {
    g_break_key = getinteger(p);
}

static void option_case(char *p) {
    if (checkstring(p, "LOWER")) {
        mmb_options.list_case = CONFIG_LOWER;
    } else if (checkstring(p, "UPPER")) {
        mmb_options.list_case = CONFIG_UPPER;
    } else if (checkstring(p, "TITLE")) {
        mmb_options.list_case = CONFIG_TITLE;
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }

    cmd_option_save();
}

static void option_codepage(char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;
    char *codepage_name = getCstring(argv[0]);
    bool found = false;
    for (int i = 0; CODEPAGE_NAMES[i]; i++) {
        if (SUCCEEDED(codepage_set(codepage_name))) {
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
        DefaultType = T_INT;
    } else if (checkstring(p, "FLOAT")) {
        DefaultType = T_NBR;
    } else if (checkstring(p, "STRING")) {
        DefaultType = T_STR;
    } else if (checkstring(p, "NONE")) {
        DefaultType = T_NOTYPE;
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }
}

static void option_explicit(char *p) {
    getargs(&p, 1, ",");
    if (argc == 1) {
        OptionExplicit = parse_bool(argv[0]);
    } else {
        OptionExplicit = true;
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

    char buf[STRINGSIZE];

    sprintf(buf, "%d", OptionBase);
    option_list_item("Base", buf);

    sprintf(buf, "%d", g_break_key);
    option_list_item("Break", buf);

    options_list_case_to_string(mmb_options.list_case, buf);
    option_list_item("Case", buf);

    if (FAILED(codepage_to_string(codepage_current, buf))) ERROR_INTERNAL_FAULT;
    option_list_item("CodePage", buf);

    options_console_to_string(mmb_options.console, buf);
    option_list_item("Console", buf);

    options_type_to_string(DefaultType, buf);
    option_list_item("Default", buf);

    options_explicit_to_string(OptionExplicit, buf);
    option_list_item("Explicit", buf);

    options_resolution_to_string(mmb_options.resolution, buf);
    option_list_item("Resolution", buf);

    sprintf(buf, "%d", mmb_options.tab);
    option_list_item("Tab", buf);

    MMPrintString("\r\n");
}

static void option_resolution(char *p) {
    char *p2 = NULL;
    if ((p2 = checkstring(p, "CHARACTER"))) {
        mmb_options.resolution = CHARACTER;
    } else if ((p2 = checkstring(p, "PIXEL"))) {
        mmb_options.resolution = PIXEL;
    }

    if (!p2 || !parse_is_end(p2)) ERROR_UNRECOGNISED_OPTION;
}

static void option_tab(char *p) {
    if (checkstring(p, "2")) {
        mmb_options.tab = 2;
    } else if (checkstring(p, "4")) {
        mmb_options.tab = 4;
    } else if (checkstring(p, "8")) {
        mmb_options.tab = 8;
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }

    cmd_option_save();
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
    } else if ((p = checkstring(cmdline, "EXPLICIT"))) {
        option_explicit(p);
    } else if ((p = checkstring(cmdline, "LIST"))) {
        option_list(p);
    } else if ((p = checkstring(cmdline, "RESOLUTION"))) {
        option_resolution(p);
    } else if ((p = checkstring(cmdline, "TAB"))) {
        option_tab(p);
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }
}
