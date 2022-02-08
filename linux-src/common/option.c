#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "option.h"
#include "utility.h"
#include "version.h"

#define INVALID_VALUE  "???"

void (*options_load_error_callback)(const char *) = NULL;

/**
 * Parses a single line expecting "name = value" format.
 *
 * If the line is empty, or whitespace only then returns success
 * and an empty name.
 *
 * @param[in]  line   line to parse.
 * @param[out] name   buffer to output the name in.
 * @param[out] value  buffer to output the value in.
 * @return 0 on success, -1 on error.
 */
static OptionsResult options_parse(const char *line, char *name, char *value) {
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
}

static OptionsResult options_parse_bool(const char *value, bool *out) {
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

static OptionsResult options_parse_int(const char *value, int *out) {
    char *endptr;
    int i = strtol(value, &endptr, 10);
    if (*endptr) {
        return kInvalidInt;
    } else {
        *out = i;
        return kOk;
    }
}

static OptionsResult options_parse_float(const char *value, MMFLOAT *out) {
    char *endptr;
    MMFLOAT f = strtod(value, &endptr);
    if (*endptr) {
        return kInvalidFloat;
    } else {
        *out = f;
        return kOk;
    }
}

static OptionsResult options_parse_string(const char *value, char *out) {
    if (*value != '"' || *(value + strlen(value) - 1) != '"') return kInvalidString;
    strncpy(out, value + 1, strlen(value) - 2);
    *(out + strlen(value) - 2) = '\0';
    return kOk;
}

static OptionsResult options_parse_list_case(const char *value, char *list_case) {
    if (strcasecmp(value, "title") == 0) {
        *list_case = CONFIG_TITLE;
    } else if (strcasecmp(value, "lower") == 0) {
        *list_case = CONFIG_LOWER;
    } else if (strcasecmp(value, "upper") == 0) {
        *list_case = CONFIG_UPPER;
    } else {
        return kInvalidEnum;
    }
    return kOk;
}

static OptionsResult options_parse_tab(const char *value, char *tab) {
    int tmp = 0;
    OptionsResult result = options_parse_int(value, &tmp);
    if (FAILED(result)) return result;
    if (tmp == 2 || tmp == 4 || tmp == 8) {
        *tab = (char) tmp;
    } else {
        return kInvalidInt;
    }
    return kOk;
}

static OptionsResult options_set(struct option_s *options, const char *name, const char *value) {
    int result = kUnknownOption;

    if (strcasecmp(name, "listcase") == 0) {
        result = options_parse_list_case(value, &(options->Listcase));
    } else if (strcasecmp(name, "tab") == 0) {
        result = options_parse_tab(value, &(options->Tab));
#if defined OPTION_TESTS
    } else if (strcasecmp(name, "persistent-bool") == 0) {
        result = options_parse_bool(value, &(options->persistent_bool));
    } else if (strcasecmp(name, "persistent-float") == 0) {
        result = options_parse_float(value, &(options->persistent_float));
    } else if (strcasecmp(name, "persistent-string") == 0) {
        result = options_parse_string(value, options->persistent_string);
#endif
    }

    return result;
}

static void options_report_error(int line_num, char *name, OptionsResult result) {
    char buf[256];
    switch (result) {
        case kUnknownOption:
            sprintf(buf, "line %d: unknown option '%s'.", line_num, name);
            break;
        case kInvalidBool:
            sprintf(buf, "line %d: invalid boolean value for option '%s'.", line_num, name);
            break;
        case kInvalidEnum:
            sprintf(buf, "line %d: invalid enum value for option '%s'.", line_num, name);
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
    }
    options_load_error_callback(buf);
}

OptionsResult options_load(struct option_s *options, const char *filename) {
    char path[STRINGSIZE];
    if (!munge_path(filename, path, STRINGSIZE)) {
        return kOtherIoError;
    }

    errno = 0;
    FILE *f = fopen(path, "r");
    if (!f) {
        switch (errno) {
            case ENOENT: return kFileNotFound;
            default:     return kOtherIoError;
        }
    }

    char line[256];
    char name[128];
    char value[128];
    int result = 0;
    int line_num = 0;
    while (!feof(f) && fgets(line, 256, f)) {
        line_num++;
        result = options_parse(line, name, value);
        if (!SUCCEEDED(result)) return result;
        if (!*name) continue; // Skip empty lines.
        result = options_set(options, name, value);
        if (!SUCCEEDED(result)) {
            if (options_load_error_callback) {
                options_report_error(line_num, name, result);
            } else {
                fclose(f);
                return result;
            }
        }
    }
    fclose(f);
    return errno ? kOtherIoError : kOk;
}

static int options_save_bool(FILE *f, const char *name, bool value) {
    return fprintf(f, "%s = %s\n", name, value ? "true" : "false") > 0 ? 0 : -1;
}

static int options_save_int(FILE *f, const char *name, int value) {
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
static OptionsResult options_create_parent_directory(const char *path) {
    char dir[STRINGSIZE];
    if (!get_parent_path(path, dir, STRINGSIZE)) {
        return kOtherIoError;
    }

    struct stat st = {0};
    if (FAILED(stat(dir, &st))) {
        if (FAILED(mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
            return kOtherIoError;
        }
    }

    return kOk;
}

OptionsResult options_save(const struct option_s *options, const char *filename) {
    char path[STRINGSIZE];
    if (!munge_path(filename, path, STRINGSIZE)) {
        return kOtherIoError;
    }

    OptionsResult result = options_create_parent_directory(path);
    if (FAILED(result)) return result;

    errno = 0;
    FILE *f = fopen(path, "w");
    if (!f) return kOtherIoError;
    char buf[STRINGSIZE];
    option_list_case_to_string(options->Listcase, buf);
    options_save_enum(f, "listcase", buf);
    options_save_int(f, "tab", options->Tab);
#if defined OPTION_TESTS
    options_save_bool(f, "persistent-bool", options->persistent_bool);
    options_save_float(f, "persistent-float", options->persistent_float);
    options_save_string(f, "persistent-string", options->persistent_string);
#endif
    fclose(f);
    return kOk;
}

void option_console_to_string(enum option_console console, char *buf) {
    switch (console) {
        case BOTH:   strcpy(buf, "Both"); break;
        case SCREEN: strcpy(buf, "Screen"); break;
        case SERIAL: strcpy(buf, "Serial"); break;
        default:     strcpy(buf, INVALID_VALUE); break;
    }
}

void option_explicit_to_string(char explicit, char *buf) {
    strcpy(buf, explicit ? "On" : "Off");
}

void option_list_case_to_string(char list_case, char *buf) {
    switch (list_case) {
        case CONFIG_LOWER: strcpy(buf, "Lower"); break;
        case CONFIG_UPPER: strcpy(buf, "Upper"); break;
        case CONFIG_TITLE: strcpy(buf, "Title"); break;
        default:           strcpy(buf, INVALID_VALUE); break;
    }
}

void option_resolution_to_string(enum option_resolution resolution, char *buf) {
    switch (resolution) {
        case CHARACTER: strcpy(buf, "Character"); break;
        case PIXEL:     strcpy(buf, "Pixel"); break;
        default:        strcpy(buf, INVALID_VALUE); break;
    }
}

void option_type_to_string(char type, char *buf) {
    switch (type) {
        case T_INT:    strcpy(buf, "Integer"); break;
        case T_NBR:    strcpy(buf, "Float"); break;
        case T_STR:    strcpy(buf, "String"); break;
        case T_NOTYPE: strcpy(buf, "None"); break;
        default:       strcpy(buf, INVALID_VALUE); break;
    }
}
