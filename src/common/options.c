/*-*****************************************************************************

MMBasic for Linux (MMB4L)

options.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "mmb4l.h"
#include "codepage.h"
#include "cstring.h"
#include "path.h"
#include "utility.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INVALID_VALUE  "???"

OptionsEditor options_editors[] = {
    { "Atom",    "atom ${file}:${line}",             false },
    { "Geany",   "geany --line=${line} ${file} &",   false },
    { "Gedit",   "gedit ${file} +${line} &",         false },
    { "Leafpad", "leafpad --jump=${line} ${file} &", false },
    { "Nano",    "nano +${line} ${file}",            true  },
    { "Sublime", "subl ${file}:${line}",             false },
    { "Vi",      "vi +${line} ${file}",              true  },
    { "Vim",     "vim +${line} ${file}",             true  },
    { "VSCode",  "code -g ${file}:${line}",          false },
    { "Xed",     "xed +${line} ${file} &",           false },
    { NULL, NULL, false }
};

static const NameOrdinalPair options_angle_map[] = {
    { "Radians", kRadians },
    { "Degrees", kDegrees },
    { NULL,      -1 }
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
    { NULL,        -1 }
};

static const NameOrdinalPair options_simulate_map[] = {
    { "MMB4L",             kSimulateMmb4l },
    { "MMBasic for Windows", kSimulateMmb4w },
    { "MMB4W",             kSimulateMmb4w },
    { "Colour Maximite 2", kSimulateCmm2 },
    { "CMM2",              kSimulateCmm2 },
    { "PicoMiteVGA",       kSimulatePicoMiteVga },
    { "Game*Mite",         kSimulateGameMite },
    { NULL,    -1 }
};

OptionsDefinition options_definitions[] = {
    { "Angle",       kOptionAngle,        kOptionTypeString,  false, "Radians",                 options_angle_map },
    { "Audio",       kOptionAudio,        kOptionTypeBoolean, true,  "On",                      NULL },
    { "AutoScale",   kOptionAutoScale,    kOptionTypeBoolean, true,  "On",                      NULL },
    { "Base",        kOptionBase,         kOptionTypeInteger, false, "0",                       NULL },
    { "Break",       kOptionBreakKey,     kOptionTypeInteger, false, "3" /* Ctrl-C */,          NULL },
    { "Case",        kOptionListCase,     kOptionTypeString,  true,  "Title",                   options_list_case_map },
    { "CodePage",    kOptionCodePage,     kOptionTypeString,  false, "None",                    codepage_name_to_ordinal_map },
    { "Console",     kOptionConsole,      kOptionTypeString,  false, "Serial",                  options_console_map },
    { "Default",     kOptionDefaultType,  kOptionTypeString,  false, "Float",                   options_default_type_map },
    { "Editor",      kOptionEditor,       kOptionTypeString,  true,  "Nano",                    options_editor_map },
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
    { "Simulate",    kOptionSimulate,     kOptionTypeString,  false, "MMB4L",                   options_simulate_map },
    { "Tab",         kOptionTab,          kOptionTypeInteger, true,  "4",                       NULL },
#if defined(OPTION_TESTS)
    { "ZBoolean",    kOptionZBoolean,     kOptionTypeBoolean, true,  "On",                      NULL },
    { "ZFloat",      kOptionZFloat,       kOptionTypeFloat,   true,  "2.71828",                 NULL },
    { "ZInteger",    kOptionZInteger,     kOptionTypeInteger, true,  "1945",                    NULL },
    { "ZString",     kOptionZString,      kOptionTypeString,  true,  "wombat",                  NULL },
#endif
    { NULL, -1, -1, false, "", NULL }
};

void options_init(Options *options) {
    memset(options, 0, sizeof(Options));

    // TODO: Do these even belong in options?
    options->autorun = 0;
    options->height = 0;
    options->width = 0;

    for (const OptionsDefinition *def = options_definitions; def->name; def++) {
        MmResult result = options_set_string_value(options, def->id, def->default_value);
        if (FAILED(result)) {
            fprintf(stderr, "%s\n", mmresult_to_string(result));
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Parses a single line expecting "name = value" format.
 *
 * If the line is empty, or whitespace only then returns kOk and an empty name.
 *
 * @param[in]  line   line to parse.
 * @param[out] name   buffer to output the name in.
 * @param[out] value  buffer to output the value in.
 */
static MmResult options_parse(const char *line, char *name, char *value) {
    // Clear name and value to avoid erroneous result being reported on error.
    name[0] = '\0';
    value[0] = '\0';

    // Check for empty or whitespace only line.
    const char *src = line;
    while (isspace(*src)) src++;
    if (!*src || *src == '#' || *src == ';') {
        *name = '\0';
        return 0;
    }

    const char *equals = strchr(line, '=');
    if (!equals) return kInvalidFormat;

    // Extract name.
    {
        char *dst = name;
        src = line;
        while (isspace(*src)) src++; // Ignore leading whitespace.
        for (; src < equals; ++src) {
            *dst++ = (*src == '-' ? ' ' : *src);
        }
        *dst = '\0';
        while (isspace(*--dst)) *dst = '\0'; // Trim trailing whitespace.
    }

    // Extract value.
    {
        char *dst = value;
        src = equals + 1;
        while (isspace(*src)) src++; // Ignore leading whitespace.
        while (*src && *src != '#' && *src != ';') *dst++ = *src++;
        *dst = '\0';
        while (isspace(*--dst)) *dst = '\0'; // Trim trailing whitespace.
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
    char buf[256] = { 0 };
    if (name[0] == '\0') {
        snprintf_nowarn(buf, 256, "line %d: %s for option.",
                line_num, mmresult_to_string(result));
    } else if (result == kUnknownOption) {
        snprintf_nowarn(buf, 256, "line %d: %s '%s'.",
                line_num, mmresult_to_string(result), name);
    } else {
        snprintf_nowarn(buf, 256, "line %d: %s for option '%s'.",
                line_num, mmresult_to_string(result), name);
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
    MmResult result = path_munge(filename, path, STRINGSIZE);
    if (FAILED(result)) return result;
    if (path_is_directory(path)) return kIsADirectory;

    errno = 0;
    FILE *f = fopen(path, "r");
    if (!f) return errno;

    char line[STRINGSIZE * 2];
    char name[STRINGSIZE];
    char value[STRINGSIZE];
    int line_num = 0;
    while (!feof(f) && fgets(line, STRINGSIZE * 2, f)) {
        line_num++;
        result = options_parse(line, name, value);
        if (SUCCEEDED(result)) {
            if (!*name) continue; // Skip empty lines.
            OptionsDefinition *def = NULL;
            result = options_get_definition(name, &def);
            if (SUCCEEDED(result)) result = options_set_string_value(options, def->id, value);
        }
        if (FAILED(result)) {
            if (!warning_cb) break;
            options_report_warning(line_num, name, result, warning_cb);
            result = kOk;
        }
    }

    fclose(f);

    return result;
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

bool options_has_default_value(const Options *options, OptionsId id) {
    char buf[STRINGSIZE];
    MmResult result = options_get_string_value(options, id, buf);
    if (FAILED(result)) return false;
    return strcmp(buf, options_definitions[id].default_value) == 0;
}

MmResult options_save(const Options *options, const char *filename) {
    char path[STRINGSIZE];
    MmResult result = path_munge(filename, path, STRINGSIZE);
    if (FAILED(result)) return result;

    errno = 0;
    FILE *f = fopen(path, "w");
    if (!f) return errno;

    char tmp[STRINGSIZE];
    for (OptionsDefinition *def = options_definitions; def->name; def++) {
        if (!def->saved) continue;
        if (options_has_default_value(options, def->id)) continue;
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
                case '\\':
                    *dst++ = '\\';
                    break;
                case '\"':
                    *dst++ = '\"';
                    break;
                case 'r':
                    *dst++ = '\r';
                    break;
                case 'n':
                    *dst++ = '\n';
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
                    src += 2;
                    break;
                }
                default:
                    return kInvalidString;
            }
            src += 2;
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
    char append[8];
    while (*src) {
        switch (*src) {
            case '\\':
                strcpy(append, "\\\\");
                break;
            case '"':
                strcpy(append, "\\\"");
                break;
            case '\r':
                strcpy(append, "\\r");
                break;
            case '\n':
                strcpy(append, "\\n");
                break;
            default:
                if (*src < 32 || *src > 126) {
                    sprintf(append, "\\%03o", *src);
                } else {
                    sprintf(append, "%c", *src);
                }
                break;
        }
        src++;

        // If the string will be too long then truncate it rather than reporting an error.
        if (dst - encoded + strlen(append) >= STRINGSIZE) return kStringTooLong;

        strcpy(dst, append);
        dst += strlen(append);
    }
    *dst = '\0';
    return kOk;
}

MmResult options_get_display_value(const Options *options, OptionsId id, char *svalue) {
    char tmp[STRINGSIZE];
    MmResult result = options_get_string_value(options, id, tmp);
    if (FAILED(result)) return result;

    // 'end' points at the last non-space character.
    const char *end = tmp + strlen(tmp) - 1;
    while (end > tmp && *end == ' ') end--;

    const char *src = tmp;
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
                if (*(src + 1) == '\n') {
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
        case kOptionAudio:
            *ivalue = options->audio;
            break;
        case kOptionAutoScale:
            *ivalue = options->auto_scale;
            break;
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

static MmResult options_get_codepage(const Options *options, char *page_name) {
    for (const NameOrdinalPair *entry = codepage_data_to_ordinal_map;
            entry->ordinal != -1;
            ++entry) {
        if (entry->name == options->codepage) {
            strcpy(page_name, codepage_name_to_ordinal_map[entry->ordinal].name);
            return kOk;
        }
    }

    return kInternalFault;
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
#if defined(ENV64BIT)
            if (SUCCEEDED(result)) sprintf(svalue, "%ld", ivalue);
#else
            if (SUCCEEDED(result)) sprintf(svalue, "%lld", ivalue);
#endif
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

        case kOptionAngle:
            assert(options->angle >= kRadians && options->angle <= kDegrees);
            options_ordinal_to_name(
                    options_definitions[kOptionAngle].enum_map,
                    options->angle,
                    svalue);
            break;

        case kOptionAudio:
        case kOptionAutoScale: {
            MMINTEGER ivalue;
            result = options_get_integer_value(options, id, &ivalue);
            if (SUCCEEDED(result)) sprintf(svalue, "%s", ivalue ? "On" : "Off");
            return result;
        }

        case kOptionCodePage:
            if (FAILED(options_get_codepage(options, svalue))) {
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

        case kOptionSimulate:
            assert(options->simulate >= kSimulateMmb4l && options->simulate <= kSimulateGameMite);
            options_ordinal_to_name(
                    options_definitions[kOptionSimulate].enum_map,
                    options->simulate,
                    svalue);
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

static MmResult options_set_angle(Options *options, const char *svalue) {
    for (const NameOrdinalPair *entry = options_angle_map; entry->name; ++entry) {
        if (strcasecmp(svalue, entry->name) == 0) {
            options->angle = entry->ordinal;
            return kOk;
        }
    }
    return kInvalidValue;
}

static MmResult options_set_audio(Options *options, int ivalue) {
    if (ivalue == 0 || ivalue == 1) {
        options->audio = ivalue;
        return kOk;
    } else {
        return kInvalidValue;
    }
}

static MmResult options_set_auto_scale(Options *options, int ivalue) {
    if (ivalue == 0 || ivalue == 1) {
        options->auto_scale = ivalue;
        return kOk;
    } else {
        return kInvalidValue;
    }
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

static MmResult options_set_codepage(Options *options, const char *page_name) {
    for (const NameOrdinalPair *entry = codepage_name_to_ordinal_map; entry->name; ++entry) {
        if (strcasecmp(page_name, entry->name) == 0) {
            options->codepage = codepage_data_to_ordinal_map[entry->ordinal].name;
            return kOk;
        }
    }
    return kInvalidValue;
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
        // Convert to standard capitalisation for standard editor names.
        options->editor[0] = '\0';
        for (const OptionsEditor *editor = options_editors; editor->name; ++editor) {
            if (strcasecmp(svalue, editor->name) == 0) {
                strcpy(options->editor, editor->name);
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

    char canonical_path[STRINGSIZE];
    MmResult result = path_get_canonical(svalue, canonical_path, STRINGSIZE);
    if (FAILED(result)) return result;
    if (!path_exists(canonical_path)) return kFileNotFound;
    if (!path_is_directory(canonical_path)) return kNotADirectory;

    strcpy(options->search_path, canonical_path);
    return kOk;
}

static MmResult options_set_simulate(Options *options, const char *svalue) {
    for (const NameOrdinalPair *entry = options_simulate_map; entry->name; ++entry) {
        if (strcasecmp(svalue, entry->name) == 0) {
            options->simulate = entry->ordinal;
            return kOk;
        }
    }
    return kInvalidValue;
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
        case kOptionAudio:     return options_set_audio(options, ivalue);
        case kOptionAutoScale: return options_set_auto_scale(options, ivalue);
        case kOptionBase:      return options_set_base(options, ivalue);
        case kOptionBreakKey:  return options_set_break_key(options, ivalue);
        case kOptionTab:       return options_set_tab(options, ivalue);

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
        case kOptionAngle:        return options_set_angle(options, svalue);
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
        case kOptionSimulate:     return options_set_simulate(options, svalue);

#if defined(OPTION_TESTS)
        case kOptionZString:
            if (strlen(svalue) >= STRINGSIZE) return kStringTooLong;
            strcpy(options->zstring, svalue);
            return kOk;
#endif

        default: return kUnknownOption;
    }
}
