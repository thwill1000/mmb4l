#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/utility.h"

static void cmd_option_save() {
    MmResult result = options_save(&mmb_options, OPTIONS_FILE_NAME);
    if (FAILED(result)) {
        char buf[STRINGSIZE];
        sprintf(buf, "Warning: failed to save options: %s", mmresult_to_string(result));
        MMPrintString(buf);
    }
}

void cmd_option_list(char *p) {
    if (!parse_is_end(p)) ERROR_SYNTAX;

    MmResult result;
    char buf[STRINGSIZE];

    for (OptionsDefinition *def = options_definitions; def->name; def++) {
        result = options_get_display_value(&mmb_options, def->id, buf);
        if (FAILED(result)) error_system(result);
        MMPrintString("Option ");
        MMPrintString((char *) def->name);
        MMPrintString(" ");
        MMPrintString(buf);
        MMPrintString("\r\n");
    }

    MMPrintString("\r\n");
}

static MmResult cmd_option_set_integer(char *p, const OptionsDefinition *def) {
    if (def->id == kOptionBase && DimUsed) error("Must be before DIM or LOCAL");
    return options_set_integer_value(&mmb_options, def->id, getinteger(p));
}

static MmResult cmd_option_set_string(char *p, const OptionsDefinition *def) {
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
        char *p2;
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

static void cmd_option_set(char *p) {
    const OptionsDefinition *def = NULL;
    char *p2;
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

    if (result == kFileNotFound && def->id == kOptionSearchPath) ERROR_DIRECTORY_NOT_FOUND;
    if (FAILED(result)) error_system(result);
    if (def->saved) cmd_option_save();
}

void cmd_option(void) {
    char *p;
    if ((p = checkstring(cmdline, "LIST"))) {
        cmd_option_list(p);
    } else {
        cmd_option_set(cmdline);
    }
}
