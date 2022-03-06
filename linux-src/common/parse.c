#include <ctype.h>
#include <stdio.h>

#include "mmb4l.h"
#include "console.h"
#include "cstring.h"
#include "error.h"

bool parse_is_end(char *p) {
    return *p == '\0' || *p == '\'';
}

char *parse_check_string(char *p, char *tkn) {
    skipspace(p);  // skip leading spaces
    while(*tkn && (toupper(*tkn) == toupper(*p))) { tkn++; p++; }  // compare the strings
    if(*tkn == 0 && (*p == ' ' || *p == ',' || *p == '\'' || *p == '(' || *p == 0)) {
        skipspace(p);
        return p;  // if successful return a pointer to the next non space character after the matched string
    }
    return NULL;  // or NULL if not
}

bool parse_bool(char *p) {
    if (parse_check_string(p, "ON") || parse_check_string(p, "TRUE")) {
        return true;
    } else if (parse_check_string(p, "OFF") || parse_check_string(p, "FALSE")) {
        return false;
    } else {
        return getint(p, 0, 1) == 1;
    }
}

int parse_colour(char *p, bool allow_bright) {
    char *p2;
    if ((p2 = parse_check_string(p, "BLACK"))) {
        return BLACK;
    } else if ((p2 = parse_check_string(p, "BLUE"))) {
        return BLUE;
    } else if ((p2 = parse_check_string(p, "BRIGHT"))) {
        if (allow_bright) {
            int colour = parse_colour(p2, 0);
            if (colour == -1) return -1;
            return colour + BRIGHT_BLACK;
        } else {
            ERROR_NOT_ALLOWED("BRIGHT");
        }
    } else if ((p2 = parse_check_string(p, "CYAN"))) {
        return CYAN;
    } else if ((p2 = parse_check_string(p, "GREEN"))) {
        return GREEN;
    } else if ((p2 = parse_check_string(p, "GRAY"))) {
        if (allow_bright) {
            return BRIGHT_BLACK;
        } else {
            ERROR_NOT_ALLOWED("GRAY");
        }
    } else if ((p2 = parse_check_string(p, "GREY"))) {
        if (allow_bright) {
            return BRIGHT_BLACK;
        } else {
            ERROR_NOT_ALLOWED("GREY");
        }
    } else if ((p2 = parse_check_string(p, "MAGENTA"))) {
        return MAGENTA;
    } else if ((p2 = parse_check_string(p, "PURPLE"))) {
        return MAGENTA;
    } else if ((p2 = parse_check_string(p, "RED"))) {
        return RED;
    } else if ((p2 = parse_check_string(p, "WHITE"))) {
        return WHITE;
    } else if ((p2 = parse_check_string(p, "YELLOW"))) {
        return YELLOW;
    }

    int colour = getint(p, BLACK, BRIGHT_WHITE);
    if (!allow_bright && colour > WHITE) colour = -1;
    return colour;
}

int parse_file_number(char *p, bool allow_zero) {
    skipspace(p); // Do we need this ?
    if (*p == '#') p++;
    int fnbr = getinteger(p);
    if (fnbr == 0 && !allow_zero) return -1;
    if (fnbr < 0 || fnbr > MAXOPENFILES) return -1;
    return fnbr;
}

/**
 * @brief Transforms input beginning with * into a corresponding RUN command.
 */
static MmResult parse_transform_star_command(char *input) {
    char *src = input;
    while (isspace(*src)) src++; // Skip whitespace.
    if (*src != '*') return kInternalFault;
    src++;

    // Trim any trailing whitespace from the input.
    char *end = input + strlen(input) - 1;
    while (isspace(*end)) *end-- = '\0';

    char tmp[STRINGSIZE + 32] = "RUN"; // Allocate extra space to avoid string overrun.
    char *dst = tmp + 3;

    if (*src == '"') {
        // Everything before the second quote is the name of the file to RUN.
        *dst++ = ' ';
        *dst++ = *src++; // Leading quote.
        while (*src && *src != '"') *dst++ = *src++;
        if (*src == '"') *dst++ = *src++; // Trailing quote.
    } else {
        // Everything before the first space is the name of the file to RUN.
        int count = 0;
        while (*src && !isspace(*src)) {
            if (++count == 1) {
                *dst++ = ' ';
                *dst++ = '\"';
            }
            *dst++ = *src++;
        }
        if (count) *dst++ = '\"';
    }

    while (isspace(*src)) src++; // Skip whitespace.

    // Everything after is arguments.
    if (*src) {
        *dst++ = ',';
        *dst++ = ' ';
        strcpy(dst, src);
    }

    if (dst >= tmp + STRINGSIZE) return kStringTooLong;

    // Copy transformed string back into the input buffer.
    strncpy(input, tmp, STRINGSIZE - 1);
    input[STRINGSIZE - 1] = '\0';

    return kOk;
}

static MmResult parse_transform_bang_cd_command(char *input, char *src) {
    // Allocate extra space to avoid string overrun.
    char tmp[STRINGSIZE + 32] = "CHDIR ";
    char *dst = tmp + 6;
    while (isspace(*src)) *src++;

    if (!*src) {
        // Special handling for 'cd' on its own.
        strcpy(input, "CHDIR \"~\"");
        return kOk;
    }

    // Start the command with a double quote unless the input started with one.
    if (*src != '\"') *dst++ = '\"';

    // Copy from src to dst.
    while (*src) {
        *dst++ = *src++;
        if (dst > tmp + STRINGSIZE - 1) return kStringTooLong;
    }

    // End the command with a double quote unless the input ended with one.
    if (*(src - 1) != '\"') *dst++ = '\"';

    // Copy transformed string back into the input buffer.
    strncpy(input, tmp, STRINGSIZE - 1);
    input[STRINGSIZE - 1] = '\0';

    *dst = '\0';

    return kOk;
}

/**
 * @brief Transforms input beginning with ! into a corresponding SYSTEM command.
 */
static MmResult parse_transform_bang_command(char *input) {
    char *src = input;
    while (isspace(*src)) src++; // Skip whitespace.
    if (*src != '!') return kInternalFault;
    src++;

    // Trim any whitespace after the bang.
    while (isspace(*src)) *src++;

    // Trim any trailing whitespace from the input.
    char *end = input + strlen(input) - 1;
    while (isspace(*end)) *end-- = '\0';

    if (!*src) {
        // Special case when it's just bang; this will be reported as a syntax error.
        strcpy(input, "SYSTEM");
        return kOk;
    }

    if (memcmp(src, "cd", 2) == 0
            && (*(src + 2) == '\0' || isblank(*(src + 2)))
            && !strstr(src, ";")
            && !strstr(src, "&&")
            && !strstr(src, "||")) {
        // Special case for the 'cd' command unless there are multiple commands.
        return parse_transform_bang_cd_command(input, src + 2);
    }

    // Allocate extra space to avoid string overrun.
    char tmp[STRINGSIZE + 32] = "SYSTEM ";
    char *dst = tmp + 7;

    // Start the command with a double quote unless the input started with one.
    if (*src != '\"') *dst++ = '\"';

    // Copy from src to dst replacing any quotes with Chr$(34).
    while (*src) {
        if (*src == '"') {
            if (dst > tmp + 7) {
                memcpy(dst, "\" + ", 4);
                dst += 4;
            }
            memcpy(dst, "Chr$(34)", 8);
            dst += 8;
            if (*(src + 1)) {
                memcpy(dst, " + \"", 4);
                dst += 4;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
        if (dst > tmp + STRINGSIZE - 2) return kStringTooLong;
    }

    // End the command with a double quote unless the input ended with one.
    if (memcmp(dst - 8, "Chr$(34)", 8) != 0) *dst++ = '\"';

    *dst = '\0';

    // Copy transformed string back into the input buffer.
    strncpy(input, tmp, STRINGSIZE - 1);
    input[STRINGSIZE - 1] = '\0';

    return kOk;
}

MmResult parse_transform_input_buffer(char *input) {
    char *p = input;
    while (isspace(*p)) p++; // Skip whitespace.
    switch (*p) {
        case '*':
            return parse_transform_star_command(input);
        case '!':
            return parse_transform_bang_command(input);
        default:
            return kOk;
    }
}
