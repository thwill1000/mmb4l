#include <sys/stat.h>
#include <unistd.h>

#include "../common/console.h"
#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/option.h"
#include "../common/parse.h"
#include "../common/utility.h"
#include "../common/version.h"

#define FONT_HEIGHT  12
#define FONT_WIDTH   8

extern char run_cmdline[STRINGSIZE];

static void mminfo_architecture(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, "x86_64");
    CtoM(g_string_rtn);
}

void get_mmcmdline(char *cmdline) {
    char *p = run_cmdline;
    skipspace(p);

    if (*p == 34) {
        do {
            p++;
        } while (*p != 34);
        p++;
        skipspace(p);
        if (*p == ',') {
            p++;
            skipspace(p);
        }
    }

    char *q;
    if (q = strchr(p, '|')) {
        q--;
        *q = 0;
    }

    strcpy(cmdline, p);
}

static void mminfo_cmdline(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    get_mmcmdline(g_string_rtn);
    CtoM(g_string_rtn);
}

static void mminfo_current(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, CurrentFile[0] == '\0' ? "NONE" : CurrentFile);
    CtoM(g_string_rtn);
}

void get_mmdevice(char *device) {
    strcpy(device, "Linux");
}

static void mminfo_device(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    get_mmdevice(g_string_rtn);
    CtoM(g_string_rtn);
}

static void mminfo_directory(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;

    g_rtn_type = T_STR;
    g_string_rtn = GetTempStrMemory();

    errno = 0;
    char *result = getcwd(g_string_rtn, STRINGSIZE);
    error_check();

    // Add a trailing '/' if one is not already present.
    // TODO: error handling if path too long.
    size_t len = strlen(g_string_rtn);
    if (g_string_rtn[len - 1] != '/') {
        g_string_rtn[len] = '/';
        g_string_rtn[len + 1] = '\0';
    }

    CtoM(g_string_rtn);
}

static void mminfo_envvar(char *p) {
    char *name = getCstring(p);
    char *value = getenv(name);
    if (!value) value = "";
    if (strlen(value) >= STRINGSIZE) {
        error("Environment variable value too long");
    }
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, value);
    CtoM(g_string_rtn);
}

static void mminfo_errmsg(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, MMErrMsg);
    CtoM(g_string_rtn);
}

static void mminfo_errno(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = MMerrno;
    g_rtn_type = T_INT;
}

static char *get_path(char *p) {
    char *path = getCstring(p);
    char tmp[STRINGSIZE];
    if (!munge_path(path, tmp, STRINGSIZE)) {
        error_check();
    }
    strcpy(path, tmp);
    return path;
}

static void mminfo_exists_dir(char *p) {
    char *path = get_path(p);
    struct stat st;
    g_integer_rtn = (stat(path, &st) == 0) && S_ISDIR(st.st_mode) ? 1 : 0;
    g_rtn_type = T_INT;
}

static void mminfo_exists_file(char *p) {
    char *path = get_path(p);
    struct stat st;
    g_integer_rtn = (stat(path, &st) == 0) && S_ISREG(st.st_mode) ? 1 : 0;
    g_rtn_type = T_INT;
}

static void mminfo_exists_symlink(char *p) {
    char *path = get_path(p);
    struct stat st;
    // Note use of lstat() rather than stat(), the latter would follow the symbolic link.
    g_integer_rtn = (lstat(path, &st) == 0) && S_ISLNK(st.st_mode) ? 1 : 0;
    g_rtn_type = T_INT;
}

static void mminfo_exists(char *p) {
    char *p2;
    if (p2 = checkstring(p, "DIR")) {
        mminfo_exists_dir(p2);
    } else if (p2 = checkstring(p, "FILE")) {
        mminfo_exists_file(p2);
    } else if (p2 = checkstring(p, "SYMLINK")) {
        mminfo_exists_symlink(p2);
    } else {
        char *path = get_path(p);
        struct stat st;
        g_integer_rtn = (stat(path, &st) == 0);
        g_rtn_type = T_INT;
    }
}

static void mminfo_filesize(char *p) {
    char *path = get_path(p);

    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            g_integer_rtn = st.st_size; // File.
        } else if (S_ISDIR(st.st_mode)) {
            g_integer_rtn = -2; // Directory.
        } else {
            error("Unexpected file type");
        }
    } else {
        g_integer_rtn = -1; // Does not exist.
    }

    g_rtn_type = T_INT;
}

static void mminfo_fontheight(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = FONT_HEIGHT;
    g_rtn_type = T_INT;
}

static void mminfo_fontwidth(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = FONT_WIDTH;
    g_rtn_type = T_INT;
}

void mminfo_hres(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int width, height;
    if (!console_get_size(&width, &height)) {
        ERROR_COULD_NOT("determine console size");
    }
    int scale = g_options.resolution == PIXEL ? FONT_WIDTH : 1;
    g_integer_rtn = width * scale;
    g_rtn_type = T_INT;
}

static void mminfo_hpos(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int x, y;
    if (!console_get_cursor_pos(&x, &y)) {
        ERROR_COULD_NOT("determine cursor position");
    }
    int scale = g_options.resolution == PIXEL ? FONT_WIDTH : 1;
    g_integer_rtn = x * scale;
    g_rtn_type = T_INT;
}

static void mminfo_option(char *p) {
    if (checkstring(p, "BASE")) {
        g_integer_rtn = OptionBase;
        g_rtn_type = T_INT;
    } else if (checkstring(p, "BREAK")) {
        g_integer_rtn = g_break_key;
        g_rtn_type = T_INT;
    } else if (checkstring(p, "CASE")) {
        g_string_rtn = GetTempStrMemory();
        option_list_case_to_string(g_options.Listcase, g_string_rtn);
        g_rtn_type = T_STR;
    } else if (checkstring(p, "CONSOLE")) {
        g_string_rtn = GetTempStrMemory();
        option_console_to_string(g_options.console, g_string_rtn);
        g_rtn_type = T_STR;
    } else if (checkstring(p, "DEFAULT")) {
        g_string_rtn = GetTempStrMemory();
        option_type_to_string(DefaultType, g_string_rtn);
        g_rtn_type = T_STR;
    } else if (checkstring(p, "EXPLICIT")) {
        g_string_rtn = GetTempStrMemory();
        option_explicit_to_string(OptionExplicit, g_string_rtn);
        g_rtn_type = T_STR;
    } else if (checkstring(p, "RESOLUTION")) {
        g_string_rtn = GetTempStrMemory();
        option_resolution_to_string(g_options.resolution, g_string_rtn);
        g_rtn_type = T_STR;
    } else if (checkstring(p, "TAB")) {
        g_integer_rtn = g_options.Tab;
        g_rtn_type = T_INT;
    } else {
        ERROR_UNRECOGNISED_OPTION;
    }

    if (g_rtn_type == T_STR) CtoM(g_string_rtn);
}

static void mminfo_path(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;

    if (CurrentFile[0] == '\0') {
        strcpy(g_string_rtn, "NONE");
    } else {
        if (!get_parent_path(CurrentFile, g_string_rtn, STRINGSIZE)) {
            ERROR_COULD_NOT("determine path");
        }
        // TODO: error handling if path too long.
        size_t len = strlen(g_string_rtn);
        g_string_rtn[len] = '/';
        g_string_rtn[len + 1] = '\0';
    }

    CtoM(g_string_rtn);
}

static void mminfo_version(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    char *endptr;
    g_float_rtn = (MMFLOAT) strtol(VERSION, &endptr, 10);
    g_float_rtn += (MMFLOAT) strtol(endptr + 1, &endptr, 10) / (MMFLOAT) 100.0;
    g_float_rtn += (MMFLOAT) strtol(endptr + 1, &endptr, 10) / (MMFLOAT) 10000.0;
    g_rtn_type = T_NBR;
}

void mminfo_vres(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int width, height;
    if (!console_get_size(&width, &height)) {
        ERROR_COULD_NOT("determine console size");
    }
    int scale = g_options.resolution == PIXEL ? FONT_HEIGHT : 1;
    g_integer_rtn = height * scale;
    g_rtn_type = T_INT;
}

static void mminfo_vpos(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int x, y;
    if (!console_get_cursor_pos(&x, &y)) {
        ERROR_COULD_NOT("determine cursor position");
    }
    int scale = g_options.resolution == PIXEL ? FONT_HEIGHT : 1;
    g_integer_rtn = y * scale;
    g_rtn_type = T_INT;
}

void fun_mminfo(void) {
    char *p;
    if (p = checkstring(ep, "ARCH")) {
        mminfo_architecture(p);
    } else if (p = checkstring(ep, "ARCHITECTURE")) {
        mminfo_architecture(p);
    } else if (p = checkstring(ep, "CMDLINE")) {
        mminfo_cmdline(p);
    } else if (p = checkstring(ep, "CURRENT")) {
        mminfo_current(p);
    } else if (p = checkstring(ep, "DEVICE")) {
        mminfo_device(p);
    } else if (p = checkstring(ep, "DIRECTORY")) {
        mminfo_directory(p);
    } else if (p = checkstring(ep, "ENVVAR")) {
        mminfo_envvar(p);
    } else if (p = checkstring(ep, "ERRMSG")) {
        mminfo_errmsg(p);
    } else if (p = checkstring(ep, "ERRNO")) {
        mminfo_errno(p);
    } else if (p = checkstring(ep, "EXISTS")) {
        mminfo_exists(p);
    } else if (p = checkstring(ep, "FILESIZE")) {
        mminfo_filesize(p);
    } else if (p = checkstring(ep, "FONTHEIGHT")) {
        mminfo_fontheight(p);
    } else if (p = checkstring(ep, "FONTWIDTH")) {
        mminfo_fontwidth(p);
    } else if (p = checkstring(ep, "HRES")) {
        mminfo_hres(p);
    } else if (p = checkstring(ep, "HPOS")) {
        mminfo_hpos(p);
    } else if (p = checkstring(ep, "OPTION")) {
        mminfo_option(p);
    } else if (p = checkstring(ep, "PATH")) {
        mminfo_path(p);
    } else if (p = checkstring(ep, "VER")) {
        mminfo_version(p);
    } else if (p = checkstring(ep, "VERSION")) {
        mminfo_version(p);
    } else if (p = checkstring(ep, "VRES")) {
        mminfo_vres(p);
    } else if (p = checkstring(ep, "VPOS")) {
        mminfo_vpos(p);
    } else {
        error("Unrecognised argument to Mm.Info()");
    }
}
