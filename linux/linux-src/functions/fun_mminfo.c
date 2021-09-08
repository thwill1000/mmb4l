#include <sys/stat.h>

#include "../common/global_aliases.h"
#include "../common/utility.h"
#include "../common/version.h"

void mminfo_current(char *p) {
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;

    strcpy(g_string_rtn, CurrentFile);

    CtoM(g_string_rtn);
}

void mminfo_directory(char *p) {
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;

    if (*CurrentFile != '\0') {
        get_parent_path(CurrentFile, g_string_rtn, STRINGSIZE);
        size_t len = strlen(g_string_rtn);
        g_string_rtn[len] = '/';
        g_string_rtn[len + 1] = '\0';
    }

    CtoM(g_string_rtn);
}

void mminfo_filesize(char *p) {
    char *path = getCstring(p);

    char new_path[STRINGSIZE];
    munge_path(path, new_path, STRINGSIZE);

    struct stat st;
    if (stat(new_path, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            g_integer_rtn = st.st_size;
        } else {
            g_integer_rtn = -2;
        }
    } else {
        g_integer_rtn = -1; // File does not exist.
    }

    g_rtn_type = T_INT;
}

static void mminfo_fontheight(char *p) {
    g_integer_rtn = 12;
    g_rtn_type = T_INT;
}

static void mminfo_fontwidth(char *p) {
    g_integer_rtn = 8;
    g_rtn_type = T_INT;
}

void mminfo_option(char *p) {
    if (checkstring(p, "BASE")) {
        g_integer_rtn = (OptionBase == 1) ? 1 : 0;
        g_rtn_type = T_INT;
    } else if (checkstring(p, "BREAK")) {
        g_integer_rtn = g_break_key;
        g_rtn_type = T_INT;
    } else if (checkstring(p, "CONSOLE")) {
        g_string_rtn = GetTempStrMemory();
        strcpy(g_string_rtn, "Serial");
        CtoM(g_string_rtn);
        g_rtn_type = T_STR;
    } else {
        error("Syntax");
    }
}

void fun_mminfo(void) {
    char *p;
    if (p = checkstring(ep, "CURRENT")) {
        mminfo_current(p);
    } else if (p = checkstring(ep, "DIRECTORY")) {
        mminfo_directory(p);
    }else if (p = checkstring(ep, "FILESIZE")) {
        mminfo_filesize(p);
    } else if (p = checkstring(ep, "FONTHEIGHT")) {
        mminfo_fontheight(p);
    } else if (p = checkstring(ep, "FONTWIDTH")) {
        mminfo_fontwidth(p);
    } else if (p = checkstring(ep, "OPTION")) {
        mminfo_option(p);
    } else {
        error("Unrecognised argument to Mm.mminfo");
    }
}
