#if !defined(MMB4L_PARSE_H)
#define MMB4L_PARSE_H

#include <stdbool.h>

#include "mmresult.h"

bool parse_is_end(char *p);
char *parse_check_string(char *p, char *tkn);
int parse_bool(char *p);
int parse_colour(char *p, bool allow_bright);
int parse_file_number(char *p, bool allow_zero);

/**
 * @brief  Does the string match the pattern for a LONGSTRING, i.e. name%() or name().
 */
bool parse_matches_longstring_pattern(const char *s);

/**
 * @brief Transforms star '*' and bang '!' commands in the input buffer
 *        to standard MMBasic commands.
 *
 * @param[in,out] The buffer to transform, usually called with the global 'inpbuf'.
 */
MmResult parse_transform_input_buffer(char *input);

#endif
