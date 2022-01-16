#if !defined(MMB4L_PARSE_H)
#define MMB4L_PARSE_H

#include <stdbool.h>

bool parse_is_end(char *p);
char *parse_check_string(char *p, char *tkn);
int parse_bool(char *p);
int parse_colour(char *p, bool allow_bright);
int parse_file_number(char *p, bool allow_zero);

#endif
