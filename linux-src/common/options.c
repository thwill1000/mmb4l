#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "mmb4l.h"
#include "codepage.h"
#include "cstring.h"
#include "options.h"
#include "path.h"
#include "utility.h"

#define INVALID_VALUE  "???"

/**
 * Note that user specification of 'code' and 'default' are always changed to
 * and stored as 'VSCode' and 'Nano' respectively, hence their command field
 * should be unused.
 */
OptionsEditor options_editors[] = {
    { "atom",    "Atom",    "atom ${file}:${line}",             false },
    { "code",    "VSCode",  "-- never used --",                 false },
    { "default", "Nano",    "-- never used --",                 true  },
    { "geany",   "Geany",   "geany --line=${line} ${file} &",   false },
    { "gedit",   "Gedit",   "gedit ${file} +${line} &",         false },
    { "leafpad", "Leafpad", "leafpad --jump=${line} ${file} &", false },
    { "nano",    "Nano",    "nano +${line} ${file}",            true  },
    { "sublime", "Sublime", "subl ${file}:${line}",             false },
    { "vi",      "Vi",      "vi +${line} ${file}",              true  },
    { "vim",     "Vim",     "vim +${line} ${file}",             true  },
    { "vscode",  "VSCode",  "code -g ${file}:${line}",          false },
    { "xed",     "Xed",     "xed +${line} ${file} &",           false },
    { NULL, NULL, NULL }
};

static const NameOrdinalPair options_console_map[] = {
    { "Both",    kBoth },
    { "Screen",  kScreen },
    { "Serial" , kSerial },
    { NULL,      -1 }
};

static const NameOrdinalPair options_default_type_map[] = {
    { "None",    T_NOTYPE },
    { "Float",   T_NBR },
    { "String",  T_STR },
    { "Integer", T_INT },
    { NULL,      -1 }
};

// Note that at the moment the ordinal is unused in this map.
static const NameOrdinalPair options_editor_map[] = {
    { "Atom",    0 },
    { "Code",    0 },
    { "Default", 0 },
    { "Geany",   0 },
    { "Gedit",   0 },
    { "Leafpad", 0 },
    { "Nano",    0 },
    { "Sublime", 0 },
    { "Vi",      0 },
    { "Vim",     0 },
    { "VSCode",  0 },
    { "Xed",     0 },
    { NULL,      -1 }
};

static const NameOrdinalPair options_list_case_map[] = {
    { "Title", kTitle },
    { "Lower", kLower },
    { "Upper", kUpper },
    { NULL,    -1 }
};

static const NameOrdinalPair options_resolution_map[] = {
    { "Character", kCharacter },
    { "Pixel",     kPixel },
    { NULL,    -1 }
};

OptionsDefinition options_definitions[] = {
    { "Base",        kOptionBase,         kOptionTypeInteger, false, "0",                       NULL },
    { "Break",       kOptionBreakKey,     kOptionTypeInteger, false, "3" /* Ctrl-C */,          NULL },
    { "Case",        kOptionListCase,     kOptionTypeString,  true,  "Title",                   options_list_case_map },
    { "CodePage",    kOptionCodePage,     kOptionTypeString,  false, "None",                    NULL },
    { "Console",     kOptionConsole,      kOptionTypeString,  false, "Serial",                  options_console_map },
    { "Default",     kOptionDefaultType,  kOptionTypeString,  false, "Float",                   options_default_type_map },
    { "Editor",      kOptionEditor,       kOptionTypeString,  true,  "Default",                 options_editor_map },
    { "Explicit",    kOptionExplicitType, kOptionTypeString,  false, "Off",                     NULL },
    { "F1",          kOptionF1,           kOptionTypeString,  true,  "FILES\r\n",               NULL },
    { "F2",          kOptionF2,           kOptionTypeString,  true,  "RUN\r\n",                 NULL },
    { "F3",          kOptionF3,           kOptionTypeString,  true,  "LIST\r\n",                NULL },
    { "F4",          kOptionF4,           kOptionTypeString,  true,  "EDIT\r\n",                NULL },
    { "F5",          kOptionF5,           kOptionTypeString,  true,  "AUTOSAVE \"\"\202",       NULL },
    { "F6",          kOptionF6,           kOptionTypeString,  true,  "XMODEM RECEIVE \"\"\202", NULL },
    { "F7",          kOptionF7,           kOptionTypeString,  true,  "XMODEM SEND \"\"\202",    NULL },
    { "F8",          kOptionF8,           kOptionTypeString,  true,  "EDIT \"\"\202",           NULL },
    { "F9",          kOptionF9,           kOptionTypeString,  true,  "LIST \"\"\202",           NULL },
    { "F10",         kOptionF10,          kOptionTypeString,  true,  "RUN \"\"\202",            NULL },
    { "F11",         kOptionF11,          kOptionTypeString,  true,  "",                        NULL },
    { "F12",         kOptionF12,          kOptionTypeString,  true,  "",                        NULL },
    { "Resolution",  kOptionResolution,   kOptionTypeString,  false, "Character",               options_resolution_map },
    { "Search Path", kOptionSearchPath,   kOptionTypeString,  true,  "",                        NULL },
    { "Tab",         kOptionTab,          kOptionTypeInteger, true,  "4",                       NULL },
#if defined(OPTION_TESTS)
    { "ZBoolean",    kOptionZBoolean,     kOptionTypeBoolean, true,  "true",                    NULL },
    { "ZFloat",      kOptionZFloat,       kOptionTypeFloat,   true,  "2.71828",                 NULL },
    { "ZInteger",    kOptionZInteger,     kOptionTypeInteger, true,  "1945",                    NULL },
    { "ZString",     kOptionZString,      kOptionTypeString,  true,  "wombat",                  NULL },
#endif
    { NULL, -1, -1, false, "" }
};

void options_init(Options *options) {
    memset(options, 0, sizeof(Options));

    // TODO: Do these even belong in options?
    options->autorun = 0;
    options->height = 0;
    options->prog_flash_size = PROG_FLASH_SIZE;
    options->width = 0;

    for (OptionsDefinition *def = options_definitions; def->name; def++) {
        MmResult result = options_set_string_value(options, def->id, def->default_value);
        if (FAILED(result)) {
            fprintf(stderr, "%s\n", mmresult_to_string(result));
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * Parses a single line expecting "name = value" format.
 *
 * If the line is empty, or whitespace only then returns success
 * and an empty name.
 *
 * @param[in]  line   line to parse.
 * @param[out] name   buffer to output the name in.
 * @param[out] value  buffer to output the value in.
 * @return TODO
 */
static MmResult options_parse(const char *line, char *name, char *value) {
    // Check for empty or whitespace only line.
    char *p = (char *) line;
    while (isspace(*p)) p++;
    if (!*p || *p == '#' || *p == ';') {
        *name = '\0';
        return 0;
    }

    char *pos = strchr(line, '=');
    if (!pos) return kInvalidFormat;

    // Extract name.
    {
        char *dst = name;
        p = (char *) line;
        while (isspace(*p)) p++; // Trim leading whitespace.
        while (p < pos) {
            *dst++ = (*p == '-' ? ' ' : *p);
            p++;
        }
        *dst = '\0';
        while (isspace(*--dst)) *dst = '\0'; // Trim trailing whitespace.
    }

    // Extract value.
    {
        char *dst = value;
        p = pos + 1;
        while (isspace(*p)) p++; // Trim leading whitespace.
        while (*p && *p != '#' && *p != ';') *dst++ = *p++;
        *dst = '\0';
        while(isspace(*--dst)) *dst = '\0'; // Trim trailing whitespace.
        cstring_unquote(value);
        char unencoded[STRINGSIZE];
        MmResult result = options_decode_string(value, unencoded);
        if (FAILED(result)) return result;
        strcpy(value, unencoded);
    }

    return kOk;
}

static MmResult options_parse_boolean(const char *value, bool *out) {
    if (strcasecmp(value, "0") == 0
            || strcasecmp(value, "false") == 0
            || strcasecmp(value, "off") == 0) {
        *out = false;
        return kOk;
    } else if (strcasecmp(value, "1") == 0
            || strcasecmp(value, "true") == 0
            || strcasecmp(value, "on") == 0) {
        *out = true;
        return kOk;
    } else {
        return kInvalidValue;
    }
}

static MmResult options_parse_integer(const char *value, int *out) {
    char *endptr;
    int i = strtol(value, &endptr, 10);
    if (*endptr) {
        return kInvalidValue;
    } else {
        *out = i;
        return kOk;
    }
}

static MmResult options_parse_float(const char *value, MMFLOAT *out) {
    char *endptr;
    MMFLOAT f = strtod(value, &endptr);
    if (*endptr) {
        return kInvalidValue;
    } else {
        *out = f;
        return kOk;
    }
}

static void options_report_warning(int line_num, char *name, MmResult result, OPTIONS_WARNING_CB warning_cb) {
    char buf[256];
    switch (result) {
        case kFileNotFound:
            sprintf(buf, "line %d: file or directory not found for option '%s'.", line_num, name);
            break;
        case kInvalidFormat:
            sprintf(buf, "line %d: invalid option format.", line_num);
            break;
        case kUnknownOption:
            sprintf(buf, "line %d: unknown option '%s'.", line_num, name);
            break;
        case kInvalidValue:
            sprintf(buf, "line %d: invalid value for option '%s'.", line_num, name);
            break;
        case kInvalidString:
            sprintf(buf, "line %d: invalid string value for option '%s'.", line_num, name);
            break;
        default:
            sprintf(buf, "line %d: unknown error for option '%s'.", line_num, name);
            break;
    }
    warning_cb(buf);
}

MmResult options_get_definition(const char *name, OptionsDefinition **definition) {
    for (OptionsDefinition *def = options_definitions; def->name; def++) {
        if (strcasecmp(def->name, name) == 0) {
            *definition = def;
            return kOk;
        }
    }
    *definition = NULL;
    return kUnknownOption;
}

MmResult options_load(Options *options, const char *filename, OPTIONS_WARNING_CB warning_cb) {
    char path[STRINGSIZE];
    if (!path_munge(filename, path, STRINGSIZE)) return errno;

    errno = 0;
    FILE *f = fopen(path, "r");
    if (!f) return errno;

    char line[256];
    char name[128];
    char value[128];
    MmResult result = kOk;
    int line_num = 0;
    while (!feof(f) && fgets(line, 256, f)) {
        line_num++;
        result = options_parse(line, name, value);
        if (SUCCEEDED(result)) {
            if (!*name) continue; // Skip empty lines.
            OptionsDefinition *def = NULL;
            result = options_get_definition(name, &def);
            if (SUCCEEDED(result)) result = options_set_string_value(options, def->id, value);
        }
        if (!SUCCEEDED(result)) {
            if (warning_cb) options_report_warning(line_num, name, result, warning_cb);
        }
    }
    fclose(f);
    return kOk;
}

/** Creates parent directory of 'filename' if it does not exist. */
static MmResult options_create_parent_directory(const char *path) {
    char dir[STRINGSIZE];
    if (!path_get_parent(path, dir, STRINGSIZE)) return errno;

    struct stat st = {0};
    if (FAILED(stat(dir, &st))) {
        if (FAILED(mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) return errno;
    }

    return kOk;
}

static void options_get_save_name(const OptionsDefinition *def, char *svalue) {
    const char *src = def->name;
    char *dst = svalue;
    while (*src) {
        if (*src == ' ') {
            *dst++ = '-';
        } else {
            *dst++ = tolower(*src);
        }
        src++;
    }
    *dst = '\0';
}

static MmResult options_get_save_value(const Options *options, OptionsId id, char *svalue) {
    if (options_definitions[id].type == kOptionTypeBoolean) {
        MMINTEGER ivalue = 0;
        MmResult result = options_get_integer_value(options, id, &ivalue);
        if (SUCCEEDED(result)) strcpy(svalue, ivalue ? "true" : "false");
        return result;
    }

    char tmp[STRINGSIZE];
    MmResult result = options_get_string_value(options, id, tmp);
    if (FAILED(result)) return result;
    result = options_encode_string(tmp, svalue);
    if (FAILED(result)) return result;
    if (options_definitions[id].type == kOptionTypeString) {
        result = SUCCEEDED(cstring_enquote(svalue, STRINGSIZE)) ? kOk : kStringTooLong;
    }
    return result;
}

MmResult options_save(const Options *options, const char *filename) {
    char path[STRINGSIZE];
    if (!path_munge(filename, path, STRINGSIZE)) return errno;

    MmResult result = options_create_parent_directory(path);
    if (FAILED(result)) return result;

    errno = 0;
    FILE *f = fopen(path, "w");
    if (!f) return errno;

    char tmp[STRINGSIZE];
    for (OptionsDefinition *def = options_definitions; def->name; def++) {
        if (!def->saved) continue;
        options_get_save_name(def, tmp);
        fprintf(f, "%s = ", tmp);
        result = options_get_save_value(options, def->id, tmp);
        if (FAILED(result)) break;
        fprintf(f, "%s\n", tmp);
    }
    fclose(f);
    return result;
}

MmResult options_decode_string(const char *encoded, char *decoded) {
    const char *src = encoded;
    char *dst = decoded;
    while (*src) {
        if (dst - decoded >= STRINGSIZE - 1) return kStringTooLong;
        if (*src == '\\') {
            switch (*(src + 1)) {
                case '\0':
                    return kInvalidString;
                case '\\':
                    *dst++ = '\\';
                    src += 2;
                    break;
                case '\"':
                    *dst++ = '\"';
                    src += 2;
                    break;
                case 'r':
                    *dst++ = '\r';
                    src += 2;
                    break;
                case 'n':
                    *dst++ = '\n';
                    src += 2;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    if (!isdigit(*(src + 2)) || !isdigit(*(src + 3))) return kInvalidString;
                    *dst++ = 64 * (*(src + 1) - '0') + 8 * (*(src + 2) - '0') + (*(src + 3) - '0');
                    src += 4;
                    break;
                }
                default:
                    return kInvalidString;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    return kOk;
}

MmResult options_encode_string(const char *unencoded, char *encoded) {
    const char *src = unencoded;
    char *dst = encoded;
    while (*src) {
        switch (*src) {
            case '\\':
                if (dst - encoded >= STRINGSIZE - 2) return kStringTooLong;
                sprintf(dst, "\\\\");
                dst += 2;
                break;
            case '"':
                if (dst - encoded >= STRINGSIZE - 2) return kStringTooLong;
                sprintf(dst, "\\\"");
                dst += 2;
                break;
            case '\r':
                if (dst - encoded >= STRINGSIZE - 2) return kStringTooLong;
                sprintf(dst, "\\r");
                dst += 2;
                break;
            case '\n':
                if (dst - encoded >= STRINGSIZE - 2) return kStringTooLong;
                sprintf(dst, "\\n");
                dst += 2;
                break;
            default:
                if (*src < 32 || *src > 126) {
                    if (dst - encoded >= STRINGSIZE - 4) return kStringTooLong;
                    sprintf(dst, "\\%03o", *src);
                    dst += 4;
                } else {
                    if (dst - encoded >= STRINGSIZE - 1) return kStringTooLong;
                    *dst++ = *src;
                }
                break;
        }
        src++;
    }
    *dst = '\0';
    return kOk;
}

MmResult options_get_display_value(const Options *options, OptionsId id, char *svalue) {
    char tmp[STRINGSIZE];
    MmResult result = options_get_string_value(options, id, tmp);
    if (FAILED(result)) return result;

    // 'end' points at the last non-space character.
    char *end = tmp + strlen(tmp) - 1;
    while (end > tmp && *end == ' ') end--;

    char *src = tmp;
    char *dst = svalue;
    char append[8];
    while (*src) {
        switch (*src) {
            case ' ':
               if (src > end) {
                    sprintf(append, "<%02x>", *src);
               } else {
                    sprintf(append, "%c", *src);
               }
               break;
            case '\n':
                strcpy(append, "<lf>");
                break;
            case '\r':
                if (*(src + 1)) {
                    strcpy(append, "<crlf>");
                    src++;
                } else {
                    strcpy(append, "<cr>");
                }
                break;
            case 0x82:
                strcpy(append, "<left>");
                break;
            default:
                if (*src < 32 || *src > 126) {
                    sprintf(append, "<%02x>", *src);
                } else {
                    sprintf(append, "%c", *src);
                }
                break;
        }
        src++;

        // If the string will be too long then truncate it rather than reporting an error.
        if (dst - svalue + strlen(append) >= STRINGSIZE) break;

        strcpy(dst, append);
        dst += strlen(append);
    }
    *dst = '\0';

    if (svalue[0] == '\0') strcpy(svalue, "<unset>");

    return result;
}

MmResult options_get_float_value(const Options *options, OptionsId id, MMFLOAT *fvalue) {
    MmResult result = kOk;
    switch (id) {
#if defined(OPTION_TESTS)
        case kOptionZFloat:
            *fvalue = options->zfloat;
            break;
#endif
        default:
            result = kInternalFault;
            break;
    }
    return result;
}

MmResult options_get_integer_value(const Options *options, OptionsId id, MMINTEGER *ivalue) {
    MmResult result = kOk;
    switch (id) {
        case kOptionBase:
            *ivalue = options->base;
            break;
        case kOptionBreakKey:
            *ivalue = options->break_key;
            break;
        case kOptionTab:
            *ivalue = options->tab;
            break;
#if defined(OPTION_TESTS)
        case kOptionZBoolean:
            *ivalue = options->zboolean;
            break;
        case kOptionZInteger:
            *ivalue = options->zinteger;
            break;
#endif
        default:
            result = kInternalFault;
            break;
    }
    return result;
}

/**
 * @brief Gets the {@param name} corresponding to an {@param ordinal}
 *        by lookup in a NameOrdinalPair array/map.
 */
static void options_ordinal_to_name(const NameOrdinalPair *map, int ordinal, char *name) {
    if (map[ordinal].ordinal == ordinal) {
        // The simple case, the ordinals are equal to the array indexes.
        strcpy(name, map[ordinal].name);
        return;
    } else {
        // The complex case, the ordinals are different to the array indexes.
        for (const NameOrdinalPair *entry = map; entry->name; entry++) {
            if (entry->ordinal == ordinal) {
                strcpy(name, entry->name);
                return;
            }
        }
    }
    strcpy(name, INVALID_VALUE);
}

MmResult options_get_string_value(const Options *options, OptionsId id, char *svalue) {
    MmResult result = kOk;

    // Note there is no protection for overrunning 'svalue';
    // it is assumed to be STRINGSIZE and all the stored string values are
    // assumed to have already been validated that size or less.

    switch (options_definitions[id].type) {
        case kOptionTypeBoolean: {
            MMINTEGER ivalue;
            result = options_get_integer_value(options, id, &ivalue);
            if (SUCCEEDED(result)) sprintf(svalue, "%s", ivalue ? "On" : "Off");
            return result;
        }

        case kOptionTypeInteger: {
            MMINTEGER ivalue;
            result = options_get_integer_value(options, id, &ivalue);
            if (SUCCEEDED(result)) sprintf(svalue, "%ld", ivalue);
            return result;
        }

        case kOptionTypeFloat: {
            MMFLOAT fvalue;
            result = options_get_float_value(options, id, &fvalue);
            if (SUCCEEDED(result)) sprintf(svalue, "%g", fvalue);
            return result;
        }

        default:
            // Include default clause to keep clang happy.
            break;
    }

    switch (id) {

        case kOptionCodePage:
            if (FAILED(codepage_to_string(options->codepage, svalue))) {
                strcpy(svalue, INVALID_VALUE);
            }
            break;

        case kOptionConsole:
            assert(options->console >= kBoth && options->console <= kSerial);
            options_ordinal_to_name(
                    options_definitions[kOptionConsole].enum_map,
                    options->console,
                    svalue);
            break;

        case kOptionDefaultType:
            assert(options->default_type == T_INT
                    || options->default_type == T_NBR
                    || options->default_type == T_STR
                    || options->default_type == T_NOTYPE);
            options_ordinal_to_name(
                    options_definitions[kOptionDefaultType].enum_map,
                    options->default_type,
                    svalue);
            break;

        case kOptionEditor:
            strcpy(svalue, options->editor);
            break;

        case kOptionExplicitType:
            strcpy(svalue, options->explicit_type ? "On" : "Off");
            break;

        case kOptionF1:
        case kOptionF2:
        case kOptionF3:
        case kOptionF4:
        case kOptionF5:
        case kOptionF6:
        case kOptionF7:
        case kOptionF8:
        case kOptionF9:
        case kOptionF10:
        case kOptionF11:
        case kOptionF12:
            strcpy(svalue, options->fn_keys[id - kOptionF1]);
            break;

        case kOptionListCase:
            assert(options->list_case >= kTitle && options->list_case <= kUpper);
            options_ordinal_to_name(
                    options_definitions[kOptionListCase].enum_map,
                    options->list_case,
                    svalue);
            break;

        case kOptionResolution:
            assert(options->resolution >= kCharacter && options->resolution <= kPixel);
            options_ordinal_to_name(
                    options_definitions[kOptionResolution].enum_map,
                    options->resolution,
                    svalue);
            break;

        case kOptionSearchPath:
            strcpy(svalue, options->search_path);
            break;

#if defined(OPTION_TESTS)
        case kOptionZString:
            strcpy(svalue, options->zstring);
            break;
#endif

        default:
            result = kInternalFault;
    }
    return result;
}

static MmResult options_set_base(Options *options, int ivalue) {
    if (ivalue == 0 || ivalue == 1) {
        options->base = ivalue;
        return kOk;
    } else {
        return kInvalidValue;
    }
}

static MmResult options_set_break_key(Options *options, int ivalue) {
    if (ivalue > 0 && ivalue < 256) {
        options->break_key = ivalue;
        return kOk;
    } else {
        return kInvalidValue;
    }
}

static MmResult options_set_codepage(Options *options, const char *svalue) {
    return SUCCEEDED(codepage_set(options, svalue)) ? kOk : kInvalidValue;
}

static MmResult options_set_console(Options *options, const char *svalue) {
    for (const NameOrdinalPair *entry = options_console_map; entry->name; ++entry) {
        if (strcasecmp(svalue, entry->name) == 0) {
            options->console = entry->ordinal;
            return kOk;
        }
    }
    return kInvalidValue;
}

static MmResult options_set_default_type(Options *options, const char *svalue) {
    for (const NameOrdinalPair *entry = options_default_type_map; entry->name; ++entry) {
        if (strcasecmp(svalue, entry->name) == 0) {
            options->default_type = entry->ordinal;
            return kOk;
        }
    }
    return kInvalidValue;
}

static MmResult options_set_editor(Options *options, const char *svalue) {
    if (svalue[0] == '\0') return kInvalidValue;
    if (strlen(svalue) >= STRINGSIZE) return kStringTooLong;

    if (strcasecmp(svalue, "code") == 0) {
        strcpy(options->editor, "VSCode");
    } else if (strcasecmp(svalue, "default") == 0) {
        strcpy(options->editor, "Nano");
    } else {
        // Convert to standard capitalisation for standard editor values.
        options->editor[0] = '\0';
        for (const OptionsEditor *editor = options_editors; editor->id; ++editor) {
            if (strcasecmp(svalue, editor->id) == 0) {
                strcpy(options->editor, editor->value);
            }
        }
        if (options->editor[0] == '\0') strcpy(options->editor, svalue);
    }
    return kOk;
}

static MmResult options_set_explicit_type(Options *options, const char *svalue) {
    bool zvalue = false;
    MmResult result = options_parse_boolean(svalue, &zvalue);
    if (FAILED(result)) return result;
    options->explicit_type = zvalue;
    return kOk;
}

static MmResult options_set_fn_key(Options *options, OptionsId id, const char *svalue) {
    if (id < kOptionF1 || id > kOptionF12) return kInternalFault;
    if (strlen(svalue) >= STRINGSIZE) return kStringTooLong;
    strcpy(options->fn_keys[id - kOptionF1], svalue);
    return kOk;
}

static MmResult options_set_list_case(Options *options, const char *svalue) {
    for (const NameOrdinalPair *entry = options_list_case_map; entry->name; ++entry) {
        if (strcasecmp(svalue, entry->name) == 0) {
            options->list_case = entry->ordinal;
            return kOk;
        }
    }
    return kInvalidValue;
}

static MmResult options_set_resolution(Options *options, const char *svalue) {
    for (const NameOrdinalPair *entry = options_resolution_map; entry->name; ++entry) {
        if (strcasecmp(svalue, entry->name) == 0) {
            options->resolution = entry->ordinal;
            return kOk;
        }
    }
    return kInvalidValue;
}

static MmResult options_set_search_path(Options *options, const char *svalue) {
    if (svalue[0] == '\0') {
        strcpy(options->search_path, "");
        return kOk;
    }

    // TODO: generate kStringTooLong error when appropriate ?
    char canonical_path[STRINGSIZE];
    if (path_get_canonical(svalue, canonical_path, STRINGSIZE)
            && path_is_directory(canonical_path)) {
        strcpy(options->search_path, canonical_path);
        return kOk;
    } else {
        return kFileNotFound;
    }
}

static MmResult options_set_tab(Options *options, int ivalue) {
    if (ivalue == 2 || ivalue == 4 || ivalue == 8) {
        options->tab = (char) ivalue;
        return kOk;
    } else {
        return kInvalidValue;
    }
}

MmResult options_set_float_value(Options *options, OptionsId id, MMFLOAT fvalue) {
    switch (id) {

#if defined(OPTION_TESTS)
        case kOptionZFloat:
            options->zfloat = fvalue;
            return kOk;
#endif

        default: return kInternalFault;
    }
}

MmResult options_set_integer_value(Options *options, OptionsId id, MMINTEGER ivalue) {
    switch (id) {
        case kOptionBase:     return options_set_base(options, ivalue);
        case kOptionBreakKey: return options_set_break_key(options, ivalue);
        case kOptionTab:      return options_set_tab(options, ivalue);

#if defined(OPTION_TESTS)
        case kOptionZBoolean:
            if (ivalue == 0 || ivalue == 1) {
                options->zboolean = ivalue;
                return kOk;
            } else {
                return kInvalidValue;
            }
        case kOptionZInteger:
            options->zinteger = ivalue;
            return kOk;
#endif

        default: return kInternalFault;
    }
}

MmResult options_set_string_value(Options *options, OptionsId id, const char *svalue) {

    switch (options_definitions[id].type) {
        case kOptionTypeBoolean: {
            bool zvalue = false;
            MmResult result = options_parse_boolean(svalue, &zvalue);
            if (FAILED(result)) return result;
            return options_set_integer_value(options, id, (MMINTEGER) zvalue);
        }

        case kOptionTypeFloat: {
            MMFLOAT fvalue = 0.0;
            MmResult result = options_parse_float(svalue, &fvalue);
            if (FAILED(result)) return result;
            return options_set_float_value(options, id, fvalue);
        }

        case kOptionTypeInteger: {
            int ivalue = 0;
            MmResult result = options_parse_integer(svalue, &ivalue);
            if (FAILED(result)) return result;
            return options_set_integer_value(options, id, ivalue);
        }

        case kOptionTypeString:
            // Nothing to do, svalue is already initialised.
            break;

        default:
            return kInternalFault;
    }

    switch (id) {
        case kOptionCodePage:     return options_set_codepage(options, svalue);
        case kOptionConsole:      return options_set_console(options, svalue);
        case kOptionDefaultType:  return options_set_default_type(options, svalue);
        case kOptionEditor:       return options_set_editor(options, svalue);
        case kOptionExplicitType: return options_set_explicit_type(options, svalue);
        case kOptionF1:
        case kOptionF2:
        case kOptionF3:
        case kOptionF4:
        case kOptionF5:
        case kOptionF6:
        case kOptionF7:
        case kOptionF8:
        case kOptionF9:
        case kOptionF10:
        case kOptionF11:
        case kOptionF12:          return options_set_fn_key(options, id, svalue);
        case kOptionListCase:     return options_set_list_case(options, svalue);
        case kOptionResolution:   return options_set_resolution(options, svalue);
        case kOptionSearchPath:   return options_set_search_path(options, svalue);

#if defined(OPTION_TESTS)
        case kOptionZString:
            if (strlen(svalue) >= STRINGSIZE) return kStringTooLong;
            strcpy(options->zstring, svalue);
            return kOk;
#endif

        default: return kUnknownOption;
    }
}
