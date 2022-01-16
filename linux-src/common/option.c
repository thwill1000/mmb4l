#include <string.h>

#include "option.h"
#include "version.h"

#define INVALID_VALUE  "???"

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
