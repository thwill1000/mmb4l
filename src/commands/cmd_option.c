/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_option.c

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
#include <stdbool.h>
#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/flash.h"
#include "../common/graphics.h"
#include "../common/parse.h"
#include "../common/utility.h"

#define ERROR_INVALID_OPTION_BASE  error_throw_ex(kError, "Must be before DIM or LOCAL")

void cmd_option_list(const char *p) {
    bool all = false;
    const char *p2 = p;
    if ((p2 = checkstring(p2, "ALL")))  {
        all = true;
        p = p2;
    }
    if (!parse_is_end(p)) ERROR_SYNTAX;

    MmResult result;
    char buf[STRINGSIZE];
    size_t count = 0;

    for (OptionsDefinition *def = options_definitions; def->name; def++) {
        if (!all && options_has_default_value(&mmb_options, def->id)) continue;
        result = options_get_display_value(&mmb_options, def->id, buf);
        if (FAILED(result)) error_throw(result);
        console_puts("Option ");
        console_puts(def->name);
        console_puts(" ");
        console_puts(buf);
        console_puts("\r\n");
        count++;
    }

    if (count == 0) console_puts("All options at default values; try OPTION LIST ALL\r\n");

    console_puts("\r\n");
}

void cmd_option_load(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;

    const char *filename = getCstring(argv[0]);
    MmResult result = options_load(&mmb_options, filename, NULL);
    if FAILED(result) error_throw(result);
}

static MmResult cmd_option_reset_all(const char *p) {
    if (!parse_is_end(p)) return kSyntax;

    MmResult result = kInternalFault;

    for (const OptionsDefinition *def = options_definitions; def->name; def++) {
        if (!def->saved) continue;
        result = options_set_string_value(&mmb_options, def->id, def->default_value);
        if (FAILED(result)) break;
    }

    if (SUCCEEDED(result)) result = options_save(&mmb_options, OPTIONS_FILE_NAME);

    return result;
}

static MmResult cmd_option_reset_one(const char *p) {
    if (parse_is_end(p)) return kSyntax;

    MmResult result = kUnknownOption;
    const char *p2;

    for (const OptionsDefinition *def = options_definitions; def->name; def++) {
        if ((p2 = checkstring(p, (char *) def->name))) {
            if (parse_is_end(p2)) {
                result = def->saved
                        ? options_set_string_value(&mmb_options, def->id, def->default_value)
                        : kNotPersistent;
            } else {
                result = kSyntax;
            }
            break;
        }
    }

    if (SUCCEEDED(result)) result = options_save(&mmb_options, OPTIONS_FILE_NAME);

    return result;
}

void cmd_option_reset(const char *p) {
    MmResult result = kOk;
    const char *p2;

    if ((p2 = checkstring(p, "ALL")))  {
        result = cmd_option_reset_all(p2);
    } else {
        result = cmd_option_reset_one(p);
    }

    if (FAILED(result)) error_throw(result);
}

void cmd_option_save(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;

    const char *filename = getCstring(argv[0]);
    MmResult result = options_save(&mmb_options, filename);
    if FAILED(result) error_throw(result);
}

static MmResult cmd_option_set_integer(const char *p, const OptionsDefinition *def) {
    if (def->id == kOptionBase && DimUsed) ERROR_INVALID_OPTION_BASE;
    return options_set_integer_value(&mmb_options, def->id, getinteger(p));
}

static MmResult cmd_option_set_string(const char *p, const OptionsDefinition *def) {
    getargs(&p, 1, ",");

    // Some hacked behaviour.
    switch (def->id) {
        case kOptionExplicitType:
            mmb_options.explicit_type = (argc == 1 ? parse_bool(argv[0]) : true);
            return kOk;

        case kOptionConsole:
            // Allowed but ignored.
            return argc == 1 ? kOk : kSyntax;

        default:
            break;
    }

    if (argc != 1) ERROR_SYNTAX;

    // First try looking up unquoted token in the options enum map.
    const char *svalue = NULL;
    if (def->enum_map) {
        const char *p2;
        for (const NameOrdinalPair *entry = def->enum_map; entry->name; ++entry) {
            if ((p2 = checkstring(argv[0], (char *) entry->name))) {
                svalue = entry->name;
                break;
            }
        }
    }

    // If that fails expect a string literal or variable.
    if (!svalue) svalue = getCstring(argv[0]);

    return options_set_string_value(&mmb_options, def->id, svalue);
}

static void cmd_option_set(const char *p) {
    const OptionsDefinition *def = NULL;
    const char *p2;
    for (def = options_definitions; def->name; ++def) {
        if ((p2 = checkstring(p, (char *) def->name))) break;
    }
    if (!def->name) ERROR_UNKNOWN_OPTION;

    MmResult result = kOk;

    switch (def->type) {
        case kOptionTypeBoolean:
        case kOptionTypeFloat:
            result = kUnimplemented;
            break;
        case kOptionTypeInteger:
            result = cmd_option_set_integer(p2, def);
            break;
        case kOptionTypeString:
            result = cmd_option_set_string(p2, def);
            break;
        default:
            result = kInternalFault;
    }

    if (FAILED(result)) error_throw(result);

    if (def->saved) {
        result = options_save(&mmb_options, OPTIONS_FILE_NAME);
        if (FAILED(result)) {
            console_puts("Warning: failed to save options: ");
            console_puts(mmresult_to_string(result));
            console_puts("\r\n");
        }
    }

    if (def->id == kOptionSimulate) {
        switch (mmb_options.simulate) {
            case kSimulateGameMite:
                ERROR_ON_FAILURE(graphics_simulate_display(kSimulateGameMite, 0, 32, RGB_BLACK));
                ERROR_ON_FAILURE(flash_init());
                break;
            case kSimulatePicoMiteVga:
                ERROR_ON_FAILURE(flash_init());
                break;
            default:
                ERROR_ON_FAILURE(flash_term());
        }
    }
}

void cmd_option(void) {
    const char *p;
    if ((p = checkstring(cmdline, "LIST"))) {
        cmd_option_list(p);
    } else if ((p = checkstring(cmdline, "LOAD"))) {
        cmd_option_load(p);
    } else if ((p = checkstring(cmdline, "RESET"))) {
        cmd_option_reset(p);
    } else if ((p = checkstring(cmdline, "SAVE"))) {
        cmd_option_save(p);
    } else {
        cmd_option_set(cmdline);
    }
}
