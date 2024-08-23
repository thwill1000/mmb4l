/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_mminfo.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/cstring.h"
#include "../common/flash.h"
#include "../common/gamepad.h"
#include "../common/gpio.h"
#include "../common/graphics.h"
#include "../common/keyboard.h"
#include "../common/mmtime.h"
#include "../common/parse.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern char cmd_run_args[STRINGSIZE];

static void mminfo_architecture(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, MM_ARCH);
    CtoM(g_string_rtn);
}

static void mminfo_calldepth(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = LocalIndex;
    g_rtn_type = T_INT;
}

static void mminfo_cmdline(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, cmd_run_args);
    CtoM(g_string_rtn);
}

static void mminfo_cputime(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_INT;
    g_integer_rtn = mmtime_get_cputime_ns();
}

static void mminfo_current(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, CurrentFile[0] == '\0' ? "NONE" : CurrentFile);
    CtoM(g_string_rtn);
}

MmResult get_mmdevice(char *device) {
    if (mmb_options.simulate == kSimulateGameMite) {
        strcpy(device, "PicoMite");
    } else {
        MmResult result = options_get_string_value(&mmb_options, kOptionSimulate, device);
        if (FAILED(result)) return kUnknownDevice;
    }
    return kOk;
}

static void mminfo_device(const char *p) {
    const char *p2;
    g_rtn_type = T_STR;
    if ((p2 = checkstring(p, "X"))) {
        // With the 'X' flag we always return the real device, i.e. "MMB4L".
        if (!parse_is_end(p2)) error_throw(kUnexpectedText);
        g_string_rtn = GetTempStrMemory();
        strcpy(g_string_rtn, "MMB4L");
    } else {
        // Without the 'X' flag we can return a value set using OPTION SIMULATE.
        if (!parse_is_end(p)) error_throw(kUnexpectedText);
        g_string_rtn = GetTempStrMemory();
        MmResult result = get_mmdevice(g_string_rtn);
        if (FAILED(result)) {
            error_throw(result);
            return;
        }
    }
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

static void mminfo_flash_address(const char *p) {
    if (mmb_options.simulate != kSimulateGameMite && mmb_options.simulate != kSimulatePicoMiteVga) {
        ERROR_ON_FAILURE(kUnsupportedOnCurrentDevice);
    }
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    const int flash_index = getint(argv[0], 1, FLASH_NUM_SLOTS) - 1;
    g_rtn_type = T_INT;
    ERROR_ON_FAILURE(flash_get_addr(flash_index, (char **) &g_integer_rtn));
}

static void mminfo_fontheight(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = graphics_font_height(graphics_font);
    g_rtn_type = T_INT;
}

static void mminfo_fontwidth(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_integer_rtn = graphics_font_width(graphics_font);
    g_rtn_type = T_INT;
}

static void mminfo_gamepad(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    MMINTEGER id = getint(argv[0], 1, 4);
    g_string_rtn = GetTempStrMemory();
    MmResult result = gamepad_info(id, g_string_rtn);
    if (result == kGamepadNotFound) {
        strcpy(g_string_rtn, "");
    } else if (FAILED(result)) {
        error_throw(result);
    }
    g_rtn_type = T_STR;
    CtoM(g_string_rtn);
}

void mminfo_hres(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_INT;
    if (graphics_current) {
        g_integer_rtn = graphics_current->width;
    } else {
        int width, height;
        if (FAILED(console_get_size(&width, &height, 0))) {
            ERROR_UNKNOWN_TERMINAL_SIZE;
        }
        int scale = mmb_options.resolution == kPixel ? graphics_font_width(graphics_font) : 1;
        g_integer_rtn = width * scale;
    }
}

static void mminfo_hpos(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int x, y;
    if (FAILED(console_get_cursor_pos(&x, &y, 10000))) {
        ERROR_COULD_NOT("determine cursor position");
    }
    int scale = mmb_options.resolution == kPixel ? graphics_font_width(graphics_font) : 1;
    g_integer_rtn = x * scale;
    g_rtn_type = T_INT;
}

static void mminfo_line(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_STR;
    g_string_rtn = GetTempStrMemory();
    int line;
    char file[STRINGSIZE];
    error_get_line_and_file(&line, file);
    if (line == -1) {
        strcpy(g_string_rtn, "UNKNOWN");
    } else {
        sprintf(g_string_rtn, "%d,", line);
        if (FAILED(cstring_cat(g_string_rtn, file, STRINGSIZE))) ERROR_STRING_TOO_LONG;
    }
    CtoM(sret);
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
        if (FAILED(path_get_parent(CurrentFile, g_string_rtn, STRINGSIZE))) {
            ERROR_COULD_NOT("determine path");
        }
        // TODO: error handling if path too long.
        size_t len = strlen(g_string_rtn);
        g_string_rtn[len] = '/';
        g_string_rtn[len + 1] = '\0';
    }

    CtoM(g_string_rtn);
}

static void mminfo_pid(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_INT;
    g_integer_rtn = (MMINTEGER) getpid();
}

static void mminfo_pin_no(const char *p) {
    if (mmb_options.simulate != kSimulatePicoMiteVga
            && mmb_options.simulate != kSimulateGameMite) {
        error_throw(kUnsupportedOnCurrentDevice);
        return;
    }

    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;

    uint8_t pin_gp = 0;
    const char *tp = argv[0];
    // First try parsing arg as literal GPnn.
    MmResult result = parse_gp_pin(&tp, &pin_gp);
    if (result == kNotParsed) {
        // If that fails treat it as a string expression instead.
        const char *s = getCstring(tp);
        result = parse_gp_pin(&s, &pin_gp);
        if (SUCCEEDED(result)) tp = skipexpression(tp);
    }

    if (FAILED(result)) {
        error_throw(result);
        return;
    }

    if (!parse_is_end(tp)) {
        error_throw(kUnexpectedText);
        return;
    }

    uint8_t pin_num = 0;
    result = gpio_translate_from_pin_gp(pin_gp, &pin_num);
    if (FAILED(result)) {
        error_throw(result);
        return;
    }

    g_rtn_type = T_INT;
    g_integer_rtn = pin_num;
}

static void mminfo_platform(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_string_rtn = GetTempStrMemory();
    g_rtn_type = T_STR;
    if (mmb_options.simulate == kSimulateGameMite) {
        strcpy(g_string_rtn, "Game*Mite");
    } else {
        strcpy(g_string_rtn, "");
    }
    CtoM(g_string_rtn);
}

static void mminfo_ps2(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_INT;
    g_integer_rtn = keyboard_get_last_ps2_scancode();
}

static void mminfo_sdcard(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_STR;
    strcpy(g_string_rtn, "READY");
    CtoM(g_string_rtn);
}

static void mminfo_version(const char *p) {
    const char *p2;
    g_rtn_type = T_INT;
    if ((p2 = checkstring(p, "MAJOR"))) {
        if (!parse_is_end(p2)) ERROR_SYNTAX;
        g_integer_rtn = MM_MAJOR;
    } else if ((p2 = checkstring(p, "MINOR"))) {
        if (!parse_is_end(p2)) ERROR_SYNTAX;
        g_integer_rtn = MM_MINOR;
    } else if ((p2 = checkstring(p, "MICRO"))) {
        if (!parse_is_end(p2)) ERROR_SYNTAX;
        g_integer_rtn = MM_MICRO;
    } else if ((p2 = checkstring(p, "BUILD"))) {
        if (!parse_is_end(p2)) ERROR_SYNTAX;
        g_integer_rtn = BUILD_NUMBER;
    } else if (!parse_is_end(p)) {
        ERROR_SYNTAX;
    } else {
        g_integer_rtn = MM_VERSION;
    }
}

void mminfo_vres(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    g_rtn_type = T_INT;
    if (graphics_current) {
        g_integer_rtn = graphics_current->height;
    } else {
        int width, height;
        if (FAILED(console_get_size(&width, &height, 0))) {
            ERROR_UNKNOWN_TERMINAL_SIZE;
        }
        int scale = mmb_options.resolution == kPixel ? graphics_font_height(graphics_font) : 1;
        g_integer_rtn = height * scale;
    }
}

static void mminfo_vpos(const char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;
    int x, y;
    if (FAILED(console_get_cursor_pos(&x, &y, 10000))) {
        ERROR_COULD_NOT("determine cursor position");
    }
    int scale = mmb_options.resolution == kPixel ? graphics_font_height(graphics_font) : 1;
    g_integer_rtn = y * scale;
    g_rtn_type = T_INT;
}

static void mminfo_writebuff(const char *p) {
    error_throw_ex(kUnsupported, "Direct access to display WriteBuff unsupported by MMB4L");
}

void fun_mminfo(void) {
    const char *p;
    if ((p = checkstring(ep, "ARCH"))) {
        mminfo_architecture(p);
    } else if ((p = checkstring(ep, "CALLDEPTH"))) {
        mminfo_calldepth(p);
    } else if ((p = checkstring(ep, "CMDLINE"))) {
        mminfo_cmdline(p);
    } else if ((p = checkstring(ep, "CPUTIME"))) {
        mminfo_cputime(p);
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
    } else if ((p = checkstring(ep, "FLASH ADDRESS"))) {
        mminfo_flash_address(p);
    } else if ((p = checkstring(ep, "FONTHEIGHT"))) {
        mminfo_fontheight(p);
    } else if ((p = checkstring(ep, "FONTWIDTH"))) {
        mminfo_fontwidth(p);
    } else if ((p = checkstring(ep, "GAMEPAD"))) {
        mminfo_gamepad(p);
    } else if ((p = checkstring(ep, "HRES"))) {
        mminfo_hres(p);
    } else if ((p = checkstring(ep, "HPOS"))) {
        mminfo_hpos(p);
    } else if ((p = checkstring(ep, "LINE"))) {
        mminfo_line(p);
    } else if ((p = checkstring(ep, "OPTION"))) {
        mminfo_option(p);
    } else if ((p = checkstring(ep, "PATH"))) {
        mminfo_path(p);
    } else if ((p = checkstring(ep, "PID"))) {
        mminfo_pid(p);
    } else if ((p = checkstring(ep, "PINNO"))) {
        mminfo_pin_no(p);
    } else if ((p = checkstring(ep, "PLATFORM"))) {
        mminfo_platform(p);
    } else if ((p = checkstring(ep, "PS2"))) {
        mminfo_ps2(p);
    } else if ((p = checkstring(ep, "SDCARD"))) {
        mminfo_sdcard(p);
    } else if ((p = checkstring(ep, "VERSION"))) {
        mminfo_version(p);
    } else if ((p = checkstring(ep, "VRES"))) {
        mminfo_vres(p);
    } else if ((p = checkstring(ep, "VPOS"))) {
        mminfo_vpos(p);
    } else if ((p = checkstring(ep, "WRITEBUFF"))) {
        mminfo_writebuff(p);
    } else {
        ERROR_UNKNOWN_ARGUMENT;
    }
}
