#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/utility.h"

void cmd_option_list(char *p) {
    bool all = false;
    char *p2 = p;
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
        if (FAILED(result)) error_system(result);
        MMPrintString("Option ");
        MMPrintString((char *) def->name);
        MMPrintString(" ");
        MMPrintString(buf);
        MMPrintString("\r\n");
        count++;
    }

    if (count == 0) MMPrintString("All options at default values; try OPTION LIST ALL\r\n");

    MMPrintString("\r\n");
}

void cmd_option_load(char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;

    const char *filename = getCstring(argv[0]);
    MmResult result = options_load(&mmb_options, filename, NULL);
    if FAILED(result) error_system(result);
}

void cmd_option_save(char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_SYNTAX;

    const char *filename = getCstring(argv[0]);
    MmResult result = options_save(&mmb_options, filename);
    if FAILED(result) error_system(result);
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

    if (FAILED(result)) error_system(result);

    if (def->saved) {
        result = options_save(&mmb_options, OPTIONS_FILE_NAME);
        if (FAILED(result)) {
            MMPrintString("Warning: failed to save options: ");
            MMPrintString((char *) mmresult_to_string(result));
            MMPrintString("\r\n");
        }
    }
}

void cmd_option(void) {
    char *p;
    if ((p = checkstring(cmdline, "LIST"))) {
        cmd_option_list(p);
    } else if ((p = checkstring(cmdline, "LOAD"))) {
        cmd_option_load(p);
    } else if ((p = checkstring(cmdline, "SAVE"))) {
        cmd_option_save(p);
    } else {
        cmd_option_set(cmdline);
    }
}
