#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "mmb4l.h"
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

void options_init(Options *options) {
    memset(options, 0, sizeof(Options));

    options->autorun = 0;
    options->base = 0;
    options->break_key = 3; // Ctrl-C
    options->codepage = NULL;
    options->console = kSerial;
    options->default_type = 0x1; // T_NBR
    options_set(options, "editor", "default");
    options->explicit_type = false;

    // Note that character 0202 (octal) is used as the mapping for the left cursor key
    // (see 'console.h') so it moves the insertion point to between the quotes.
    strcpy(options->fn_keys[0],  "FILES\r\n");
    strcpy(options->fn_keys[1],  "RUN\r\n");
    strcpy(options->fn_keys[2],  "LIST\r\n");
    strcpy(options->fn_keys[3],  "EDIT\r\n");
    strcpy(options->fn_keys[4],  "AUTOSAVE \"\"\202");
    strcpy(options->fn_keys[5],  "XMODEM RECEIVE \"\"\202");
    strcpy(options->fn_keys[6],  "XMODEM SEND \"\"\202");
    strcpy(options->fn_keys[7],  "EDIT \"\"\202");
    strcpy(options->fn_keys[8],  "LIST \"\"\202");
    strcpy(options->fn_keys[9],  "RUN \"\"\202");
    strcpy(options->fn_keys[10], "");
    strcpy(options->fn_keys[11], "");

    options->height = 0;
    options->list_case = kTitle;
    options->prog_flash_size = PROG_FLASH_SIZE;
    options->resolution = kCharacter;
    options->tab = 4;
    options->width = 0;

#if defined OPTION_TESTS
    options->zboolean = true;
    options->zfloat = 2.71828;
    options->zinteger = 1945;
    strcpy(options->zstring, "wombat");
#endif
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
    p = (char *) line;
    while (isspace(*p)) p++; // Trim leading whitespace.
    while (p < pos) *name++ = *p++;
    *name = '\0';
    while (isspace(*--name)) *name = '\0'; // Trim trailing whitespace.

    // Extract value.
    p = pos + 1;
    while (isspace(*p)) p++; // Trim leading whitespace.
    while (*p && *p != '#' && *p != ';') *value++ = *p++;
    *value = '\0';
    while(isspace(*--value)) *value = '\0'; // Trim trailing whitespace.

    return kOk;
}

static MmResult options_parse_boolean(const char *value, bool *out) {
    if (strcasecmp(value, "0") == 0 || strcasecmp(value, "false") == 0) {
        *out = false;
        return kOk;
    } else if (strcasecmp(value, "1") == 0 || strcasecmp(value, "true") == 0) {
        *out = true;
        return kOk;
    } else {
        return kInvalidBool;
    }
}

static MmResult options_parse_integer(const char *value, int *out) {
    char *endptr;
    int i = strtol(value, &endptr, 10);
    if (*endptr) {
        return kInvalidInt;
    } else {
        *out = i;
        return kOk;
    }
}

static MmResult options_parse_float(const char *value, MMFLOAT *out) {
    char *endptr;
    MMFLOAT f = strtod(value, &endptr);
    if (*endptr) {
        return kInvalidFloat;
    } else {
        *out = f;
        return kOk;
    }
}

static MmResult options_parse_string(const char *value, char *out) {
    if (*value != '"' || *(value + strlen(value) - 1) != '"') return kInvalidString;
    char tmp[STRINGSIZE];
    strncpy(tmp, value + 1, strlen(value) - 2);
    *(tmp + strlen(value) - 2) = '\0';
    return options_decode_string(tmp, out);
}

static MmResult options_parse_editor(const char *value, char *editor) {
    for (int i = 0; options_editors[i].id; ++i) {
        if (strcasecmp(value, options_editors[i].id) == 0) {
            // Standard editor.
            strcpy(editor, options_editors[i].value);
            return kOk;
        }
    }

    if (value[0] == '\"' && value[strlen(value) - 1] == '\"') {
        // Manually specified editor.
        strcpy(editor, value);
        return kOk;
    }

    return kInvalidValue;
}

static MmResult options_parse_list_case(const char *value, OptionsListCase *list_case) {
    if (strcasecmp(value, "title") == 0) {
        *list_case = kTitle;
    } else if (strcasecmp(value, "lower") == 0) {
        *list_case = kLower;
    } else if (strcasecmp(value, "upper") == 0) {
        *list_case = kUpper;
    } else {
        return kInvalidValue;
    }
    return kOk;
}

static MmResult options_parse_search_path(const char *value, char *search_path) {
    char tmp[STRINGSIZE];
    MmResult result = options_parse_string(value, tmp);
    if (SUCCEEDED(result)) {
        if (*tmp) {
            char canonical_path[STRINGSIZE];
            if (path_get_canonical(tmp, canonical_path, STRINGSIZE)
                    && path_is_directory(canonical_path)) {
                strcpy(search_path, canonical_path);
            } else {
                result = kFileNotFound;
            }
        } else {
            strcpy(search_path, "");
        }
    }
    return result;
}

static MmResult options_parse_tab(const char *value, char *tab) {
    int tmp = 0;
    MmResult result = options_parse_integer(value, &tmp);
    if (FAILED(result)) return result;
    if (tmp == 2 || tmp == 4 || tmp == 8) {
        *tab = (char) tmp;
    } else {
        return kInvalidInt;
    }
    return kOk;
}

static MmResult options_parse_fn_key(Options *options, const char *name, const char *value) {
    char buf[STRINGSIZE];
    int f = 1;
    while (f < 13) {
        sprintf(buf, "f%d", f);
        if (strcasecmp(name, buf) == 0) break;
        f++;
    }
    if (f == 13) return kUnknownOption;
    return options_parse_string(value, options->fn_keys[f - 1]);
}

MmResult options_set(Options *options, const char *name, const char *value) {
    if (strcasecmp(name, "editor") == 0) {
        return options_parse_editor(value, options->editor);
    } else if (strcasecmp(name, "listcase") == 0) {
        return options_parse_list_case(value, &(options->list_case));
    } else if (strcasecmp(name, "search-path") == 0) {
        return options_parse_search_path(value, options->search_path);
    } else if (strcasecmp(name, "tab") == 0) {
        return options_parse_tab(value, &(options->tab));
#if defined OPTION_TESTS
    } else if (strcasecmp(name, "zboolean") == 0) {
        return options_parse_boolean(value, &(options->zboolean));
    } else if (strcasecmp(name, "zfloat") == 0) {
        return options_parse_float(value, &(options->zfloat));
    } else if (strcasecmp(name, "zinteger") == 0) {
        return options_parse_integer(value, &(options->zinteger));
    } else if (strcasecmp(name, "zstring") == 0) {
        return options_parse_string(value, options->zstring);
#endif
    } else if (toupper(name[0]) == 'F' && isdigit(name[1])) { // f1-12
        return options_parse_fn_key(options, name, value);
    } else {
        return kUnknownOption;
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
        case kInvalidBool:
            sprintf(buf, "line %d: invalid boolean value for option '%s'.", line_num, name);
            break;
        case kInvalidValue:
            sprintf(buf, "line %d: invalid value for option '%s'.", line_num, name);
            break;
        case kInvalidFloat:
            sprintf(buf, "line %d: invalid float value for option '%s'.", line_num, name);
            break;
        case kInvalidInt:
            sprintf(buf, "line %d: invalid integer value for option '%s'.", line_num, name);
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

MmResult options_load(Options *options, const char *filename, OPTIONS_WARNING_CB warning_cb) {
    char path[STRINGSIZE];
    if (!path_munge(filename, path, STRINGSIZE)) return errno;

    errno = 0;
    FILE *f = fopen(path, "r");
    if (!f) return errno;

    char line[256];
    char name[128];
    char value[128];
    int result = 0;
    int line_num = 0;
    while (!feof(f) && fgets(line, 256, f)) {
        line_num++;
        result = options_parse(line, name, value);
        if (SUCCEEDED(result)) {
            if (!*name) continue; // Skip empty lines.
            result = options_set(options, name, value);
        }
        if (!SUCCEEDED(result)) {
            if (warning_cb) options_report_warning(line_num, name, result, warning_cb);
        }
    }
    fclose(f);
    return kOk;
}

static int options_save_boolean(FILE *f, const char *name, bool value) {
    return fprintf(f, "%s = %s\n", name, value ? "true" : "false") > 0 ? 0 : -1;
}

static int options_save_integer(FILE *f, const char *name, int value) {
    return fprintf(f, "%s = %d\n", name, value) > 0 ? 0 : -1;
}

static int options_save_float(FILE *f, const char *name, MMFLOAT value) {
    return fprintf(f, "%s = %g\n", name, value) > 0 ? 0 : -1;
}

static int options_save_string(FILE *f, const char *name, const char *value) {
    return fprintf(f, "%s = \"%s\"\n", name, value);
}

static int options_save_enum(FILE *f, const char *name, const char *value) {
    return fprintf(f, "%s = %s\n", name, value);
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

MmResult options_save(const Options *options, const char *filename) {
    char path[STRINGSIZE];
    if (!path_munge(filename, path, STRINGSIZE)) return errno;

    MmResult result = options_create_parent_directory(path);
    if (FAILED(result)) return result;

    errno = 0;
    FILE *f = fopen(path, "w");
    if (!f) return errno;

    char name[32];
    char value[STRINGSIZE];
    options_editor_to_string(options->editor, value);
    options_save_enum(f, "editor", value);
    for (int i = 0; i < 12; ++i) {
        sprintf(name, "f%d", i + 1);
        options_fn_key_to_string(options->fn_keys[i], value);
        options_save_string(f, name, value);
    }
    options_list_case_to_string(options->list_case, value);
    options_save_enum(f, "listcase", value);
    options_save_string(f, "search-path", options->search_path);
    options_save_integer(f, "tab", options->tab);
#if defined OPTION_TESTS
    options_save_boolean(f, "zboolean", options->zboolean);
    options_save_float(f, "zfloat", options->zfloat);
    options_save_integer(f, "zinteger", options->zinteger);
    options_save_string(f, "zstring", options->zstring);
#endif
    fclose(f);
    return kOk;
}

void options_console_to_string(OptionsConsole console, char *buf) {
    switch (console) {
        case kBoth:   strcpy(buf, "Both"); break;
        case kScreen: strcpy(buf, "Screen"); break;
        case kSerial: strcpy(buf, "Serial"); break;
        default:      strcpy(buf, INVALID_VALUE); break;
    }
}

void options_editor_to_string(const char *editor, char *buf) {
    // Return 'editor' value verbatim, any invalid values should have been
    // caught before this point.
    strcpy(buf, editor);
}

void options_explicit_to_string(char explicit, char *buf) {
    strcpy(buf, explicit ? "On" : "Off");
}

void options_fn_key_to_string(const char *fn_key, char *buf) {
    options_encode_string(fn_key, buf);
}

void options_list_case_to_string(OptionsListCase list_case, char *buf) {
    switch (list_case) {
        case kTitle: strcpy(buf, "Title"); break;
        case kLower: strcpy(buf, "Lower"); break;
        case kUpper: strcpy(buf, "Upper"); break;
        default:     strcpy(buf, INVALID_VALUE); break;
    }
}

void options_resolution_to_string(OptionsResolution resolution, char *buf) {
    switch (resolution) {
        case kCharacter: strcpy(buf, "Character"); break;
        case kPixel:     strcpy(buf, "Pixel"); break;
        default:         strcpy(buf, INVALID_VALUE); break;
    }
}

void options_type_to_string(char type, char *buf) {
    switch (type) {
        case T_INT:    strcpy(buf, "Integer"); break;
        case T_NBR:    strcpy(buf, "Float"); break;
        case T_STR:    strcpy(buf, "String"); break;
        case T_NOTYPE: strcpy(buf, "None"); break;
        default:       strcpy(buf, INVALID_VALUE); break;
    }
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
