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

/**
 * @brief Transforms input beginning with ! into a corresponding SYSTEM command.
 */
static MmResult parse_transform_bang_command(char *input) {
    return kError;
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
