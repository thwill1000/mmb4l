#include "../common/global_aliases.h"
#include "../common/version.h"

void option_base(char *p) {
    if (DimUsed) error("Must be before DIM or LOCAL");
    OptionBase = getint(p, 0, 1);
}

void option_break(char *p) {
    g_break_key = getinteger(p);
}

void option_case(char *p) {
    if (checkstring(p, "LOWER")) {
        Option.Listcase = CONFIG_LOWER;
        SaveOptions();
    } else if (checkstring(p, "UPPER")) {
        Option.Listcase = CONFIG_UPPER;
        SaveOptions();
    } else if (checkstring(p, "TITLE")) {
        Option.Listcase = CONFIG_TITLE;
        SaveOptions();
    } else {
        error("Option");
    }
}

void option_console(char *p) {
    // Allowed but ignored.
}

void option_default(char *p) {
    if (checkstring(p, "INTEGER")) {
        DefaultType = T_INT;
        return;
    } else if (checkstring(p, "FLOAT")) {
        DefaultType = T_NBR;
        return;
    } else if (checkstring(p, "STRING")) {
        DefaultType = T_STR;
        return;
    } else if (checkstring(p, "NONE")) {
        DefaultType = T_NOTYPE;
        return;
    } else {
        error("Option");
    }
}

void option_explicit(char *p) {
    OptionExplicit = true;
}

void option_tab(char *p) {
    if (checkstring(p, "2")) {
        Option.Tab = 2;
        SaveOptions();
    } else if (checkstring(p, "4")) {
        Option.Tab = 4;
        SaveOptions();
    } else if (checkstring(p, "8")) {
        Option.Tab = 8;
        SaveOptions();
    } else {
        error("Option");
    }
}

void cmd_option(void) {
    char *p;
    if (p = checkstring(cmdline, "BASE")) {
        option_base(p);
    } else if (p = checkstring(cmdline, "BREAK")) {
        option_break(p);
    } else if (p = checkstring(cmdline, "CASE")) {
        option_case(p);
    } else if (p = checkstring(cmdline, "CONSOLE")) {
        option_console(p);
    } else if (p = checkstring(cmdline, "DEFAULT")) {
        option_default(p);
    } else if (p = checkstring(cmdline, "EXPLICIT")) {
        option_explicit(p);
    } else if (p = checkstring(cmdline, "TAB")) {
        option_tab(p);
    } else {
        error("Option");
    }
}
