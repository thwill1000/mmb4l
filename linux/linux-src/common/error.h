#if !defined(MMB4L_ERROR_H)
#define MMB4L_ERROR_H

#include "../Configuration.h" // for STRINGSIZE

extern char error_file[STRINGSIZE];
extern int error_line;

void error(char *msg, ...);
int error_check();

#define ERROR_ARGUMENT_COUNT           error("Argument count")
#define ERROR_UNKNOWN_SUBCOMMAND       error("Unknown subcommand")
#define ERROR_ARGUMENT_NOT_INTEGER(s)  error("Argument " s " must be an integer")
#define ERROR_COULD_NOT(s)             error("Could not " s)
#define ERROR_INTERNAL_FAULT           error("Internal fault (sorry)")
#define ERROR_INVALID(s)               error("Invalid " s)
#define ERROR_INVALID_ARGUMENT         ERROR_INVALID("Argument")
#define ERROR_SYNTAX                   error("Syntax")
#define ERROR_UNIMPLEMENTED(s)         error("Unimplemented: " s)
#define ERROR_UNRECOGNISED_OPTION      error("Unrecognised option")

#endif
