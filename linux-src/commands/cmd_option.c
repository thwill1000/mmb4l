#include <stdbool.h>

#include "../common/codepage.h"
#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/option.h"
#include "../common/parse.h"
#include "../common/utility.h"
#include "../common/version.h"

static void option_base(char *p) {
    if (DimUsed) error("Must be before DIM or LOCAL");
    OptionBase = getint(p, 0, 1);
}

static void option_break(char *p) {
    g_break_key = getinteger(p);
}

static void option_case(char *p) {
    if (checkstring(p, "LOWER")) {
        Option.Listcase = CONFIG_LOWER;
    } else if (checkstring(p, "UPPER")) {
        Option.Listcase = CONFIG_UPPER;
    } else if (checkstring(p, "TITLE")) {
        Option.Listcase = CONFIG_TITLE;
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }

    SaveOptions();
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

    option_list_case_to_string(g_options.Listcase, buf);
    option_list_item("Case", buf);

    if (FAILED(codepage_to_string(codepage_current, buf))) ERROR_INTERNAL_FAULT;
    option_list_item("CodePage", buf);

    option_console_to_string(g_options.console, buf);
    option_list_item("Console", buf);

    option_type_to_string(DefaultType, buf);
    option_list_item("Default", buf);

    option_explicit_to_string(OptionExplicit, buf);
    option_list_item("Explicit", buf);

    option_resolution_to_string(g_options.resolution, buf);
    option_list_item("Resolution", buf);

    sprintf(buf, "%d", g_options.Tab);
    option_list_item("Tab", buf);

    MMPrintString("\r\n");
}

static void option_resolution(char *p) {
    char *p2 = NULL;
    if ((p2 = checkstring(p, "CHARACTER"))) {
        g_options.resolution = CHARACTER;
    } else if ((p2 = checkstring(p, "PIXEL"))) {
        g_options.resolution = PIXEL;
    }

    if (!p2 || !parse_is_end(p2)) ERROR_UNRECOGNISED_OPTION;
}

static void option_tab(char *p) {
    if (checkstring(p, "2")) {
        Option.Tab = 2;
    } else if (checkstring(p, "4")) {
        Option.Tab = 4;
    } else if (checkstring(p, "8")) {
        Option.Tab = 8;
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }

    SaveOptions();
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
