/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_mminfo.c

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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/memory.h"
#include "../common/options.h"
#include "../common/parse.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"

#define FONT_HEIGHT  12
#define FONT_WIDTH   8

extern char run_cmdline[STRINGSIZE];

static void mminfo_architecture(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, MM_ARCH);
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
    if ((q = strchr(p, '|'))) {
        q--;
        *q = 0;
    }

    strcpy(cmdline, p);
}

static void mminfo_cmdline(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    get_mmcmdline(g_string_rtn);
    CtoM(g_string_rtn);
}

static void mminfo_current(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, CurrentFile[0] == '\0' ? "NONE" : CurrentFile);
    CtoM(g_string_rtn);
}

void get_mmdevice(char *device) {
    strcpy(device, MM_DEVICE);
}

static void mminfo_device(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    get_mmdevice(g_string_rtn);
    CtoM(g_string_rtn);
}

static void mminfo_directory(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;

    g_rtn_type = T_STR;
    g_string_rtn = GetTempStrMemory();

    errno = 0;
    if (!getcwd(g_string_rtn, STRINGSIZE)) error_throw(errno);

    // Add a trailing '/' if one is not already present.
    // TODO: error handling if path too long.
    size_t len = strlen(g_string_rtn);
    if (g_string_rtn[len - 1] != '/') {
        g_string_rtn[len] = '/';
        g_string_rtn[len + 1] = '\0';
    }

    CtoM(g_string_rtn);
}

static void mminfo_envvar(const char *p) {
    const char *name = getCstring(p);
    const char *value = getenv(name);
    if (!value) value = "";
    if (strlen(value) >= STRINGSIZE) ERROR_ENV_VAR_TOO_LONG;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, value);
    CtoM(g_string_rtn);
}

static void mminfo_errmsg(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, mmb_error_state_ptr->message);
    CtoM(g_string_rtn);
}

static void mminfo_errno(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = mmb_error_state_ptr->code;
    g_rtn_type = T_INT;
}

static char *get_path(const char *p) {
    char *path = GetTempStrMemory();
    MmResult result = path_munge(getCstring(p), path, STRINGSIZE);
    if (FAILED(result)) error_throw(result);
    return path;
}

static void mminfo_exists_dir(const char *p) {
    char *path = get_path(p);
    struct stat st;
    g_integer_rtn = (stat(path, &st) == 0) && S_ISDIR(st.st_mode) ? 1 : 0;
    g_rtn_type = T_INT;
}

static void mminfo_exists_file(const char *p) {
    char *path = get_path(p);
    struct stat st;
    g_integer_rtn = (stat(path, &st) == 0) && S_ISREG(st.st_mode) ? 1 : 0;
    g_rtn_type = T_INT;
}

static void mminfo_exists_symlink(const char *p) {
    char *path = get_path(p);
    struct stat st;
    // Note use of lstat() rather than stat(), the latter would follow the symbolic link.
    g_integer_rtn = (lstat(path, &st) == 0) && S_ISLNK(st.st_mode) ? 1 : 0;
    g_rtn_type = T_INT;
}

static void mminfo_exists(const char *p) {
    const char *p2;
    if ((p2 = checkstring(p, "DIR"))) {
        mminfo_exists_dir(p2);
    } else if ((p2 = checkstring(p, "FILE"))) {
        mminfo_exists_file(p2);
    } else if ((p2 = checkstring(p, "SYMLINK"))) {
        mminfo_exists_symlink(p2);
    } else {
        const char *path = get_path(p);
        struct stat st;
        g_integer_rtn = (stat(path, &st) == 0);
        g_rtn_type = T_INT;
    }
}

static void mminfo_exitcode(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_INT;
    g_integer_rtn = mmb_exit_code;
}

static void mminfo_filesize(const char *p) {
    char *path = get_path(p);

    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            g_integer_rtn = -2; // TODO: this matches CMM2, but probably better
                                // just to return st.st_size.
        } else {
            g_integer_rtn = st.st_size;
        }
    } else {
        g_integer_rtn = -1; // Does not exist.
    }

    g_rtn_type = T_INT;
}

static void mminfo_fontheight(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = FONT_HEIGHT;
    g_rtn_type = T_INT;
}

static void mminfo_fontwidth(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = FONT_WIDTH;
    g_rtn_type = T_INT;
}

void mminfo_hres(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int width, height;
    if (FAILED(console_get_size(&width, &height))) {
        ERROR_COULD_NOT("determine console size");
    }
    int scale = mmb_options.resolution == kPixel ? FONT_WIDTH : 1;
    g_integer_rtn = width * scale;
    g_rtn_type = T_INT;
}

static void mminfo_hpos(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int x, y;
    if (FAILED(console_get_cursor_pos(&x, &y, 10000))) {
        ERROR_COULD_NOT("determine cursor position");
    }
    int scale = mmb_options.resolution == kPixel ? FONT_WIDTH : 1;
    g_integer_rtn = x * scale;
    g_rtn_type = T_INT;
}

static void mminfo_option(const char *p) {
    OptionsDefinition *def = NULL;
    for (def = options_definitions; def->name; def++) {
        if (checkstring(p, (char *) def->name)) break;
    }

    if (!def->name) ERROR_UNKNOWN_OPTION;

    MmResult result = kInternalFault;

    switch (def->type) {
        case kOptionTypeFloat:
            g_rtn_type = T_NBR;
            result = options_get_float_value(&mmb_options, def->id, &g_float_rtn);
            break;

        case kOptionTypeInteger:
        case kOptionTypeBoolean:
            g_rtn_type = T_INT;
            result = options_get_integer_value(&mmb_options, def->id, &g_integer_rtn);
            break;

        case kOptionTypeString:
            g_rtn_type = T_STR;
            g_string_rtn = GetTempStrMemory();
            result = options_get_string_value(&mmb_options, def->id, g_string_rtn);
            if (SUCCEEDED(result)) CtoM(g_string_rtn);
            break;
    }

    if (FAILED(result)) error_throw(result);
}

static void mminfo_path(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;

    if (CurrentFile[0] == '\0') {
        strcpy(g_string_rtn, "NONE");
    } else {
        if (!path_get_parent(CurrentFile, g_string_rtn, STRINGSIZE)) {
            ERROR_COULD_NOT("determine path");
        }
        // TODO: error handling if path too long.
        size_t len = strlen(g_string_rtn);
        g_string_rtn[len] = '/';
        g_string_rtn[len + 1] = '\0';
    }

    CtoM(g_string_rtn);
}

static void mminfo_version(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    char *endptr;
    g_float_rtn = (MMFLOAT) strtol(VERSION, &endptr, 10);
    g_float_rtn += (MMFLOAT) strtol(endptr + 1, &endptr, 10) / (MMFLOAT) 100.0;
    g_float_rtn += (MMFLOAT) strtol(endptr + 1, &endptr, 10) / (MMFLOAT) 10000.0;
    g_rtn_type = T_NBR;
}

void mminfo_vres(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int width, height;
    if (FAILED(console_get_size(&width, &height))) {
        ERROR_COULD_NOT("determine console size");
    }
    int scale = mmb_options.resolution == kPixel ? FONT_HEIGHT : 1;
    g_integer_rtn = height * scale;
    g_rtn_type = T_INT;
}

static void mminfo_vpos(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int x, y;
    if (FAILED(console_get_cursor_pos(&x, &y, 10000))) {
        ERROR_COULD_NOT("determine cursor position");
    }
    int scale = mmb_options.resolution == kPixel ? FONT_HEIGHT : 1;
    g_integer_rtn = y * scale;
    g_rtn_type = T_INT;
}

void fun_mminfo(void) {
    const char *p;
    if ((p = checkstring(ep, "ARCH"))) {
        mminfo_architecture(p);
    } else if ((p = checkstring(ep, "CMDLINE"))) {
        mminfo_cmdline(p);
    } else if ((p = checkstring(ep, "CURRENT"))) {
        mminfo_current(p);
    } else if ((p = checkstring(ep, "DEVICE"))) {
        mminfo_device(p);
    } else if ((p = checkstring(ep, "DIRECTORY"))) {
        mminfo_directory(p);
    } else if ((p = checkstring(ep, "ENVVAR"))) {
        mminfo_envvar(p);
    } else if ((p = checkstring(ep, "ERRMSG"))) {
        mminfo_errmsg(p);
    } else if ((p = checkstring(ep, "ERRNO"))) {
        mminfo_errno(p);
    } else if ((p = checkstring(ep, "EXISTS"))) {
        mminfo_exists(p);
    } else if ((p = checkstring(ep, "EXITCODE"))) {
        mminfo_exitcode(p);
    } else if ((p = checkstring(ep, "FILESIZE"))) {
        mminfo_filesize(p);
    } else if ((p = checkstring(ep, "FONTHEIGHT"))) {
        mminfo_fontheight(p);
    } else if ((p = checkstring(ep, "FONTWIDTH"))) {
        mminfo_fontwidth(p);
    } else if ((p = checkstring(ep, "HRES"))) {
        mminfo_hres(p);
    } else if ((p = checkstring(ep, "HPOS"))) {
        mminfo_hpos(p);
    } else if ((p = checkstring(ep, "OPTION"))) {
        mminfo_option(p);
    } else if ((p = checkstring(ep, "PATH"))) {
        mminfo_path(p);
    } else if ((p = checkstring(ep, "VERSION"))) {
        mminfo_version(p);
    } else if ((p = checkstring(ep, "VRES"))) {
        mminfo_vres(p);
    } else if ((p = checkstring(ep, "VPOS"))) {
        mminfo_vpos(p);
    } else {
        ERROR_UNKNOWN_ARGUMENT;
    }
}
