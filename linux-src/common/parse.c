#include "console.h"
#include "error.h"
#include "version.h"

int parse_is_end(char *p) {
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

int parse_bool(char *p) {
    if (parse_check_string(p, "ON") || parse_check_string(p, "TRUE")) {
        return 1;
    } else if (parse_check_string(p, "OFF") || parse_check_string(p, "FALSE")) {
        return 0;
    } else {
        return getint(p, 0, 1);
    }
}

int parse_colour(char *p, int allow_bright) {
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
