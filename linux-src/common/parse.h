#if !defined(PARSE_H)
#define PARSE_H

int parse_is_end(char *p);
char *parse_check_string(char *p, char *tkn);
int parse_bool(char *p);
int parse_colour(char *p, int allow_bright);

#endif