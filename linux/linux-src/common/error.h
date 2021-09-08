#if !defined(ERROR_H)
#define ERROR_H

void error(char *msg, ...);

#define ERROR_ARGUMENT_COUNT           error("Argument count")
#define ERROR_UNKNOWN_SUBCOMMAND       error("Unknown subcommand")
#define ERROR_ARGUMENT_NOT_INTEGER(s)  error("Argument " s " must be an integer")
#define ERROR_COULD_NOT(s)             error("Could not " s)
#define ERROR_INVALID(s)               error("Invalid " s)
#define ERROR_SYNTAX                   error("Syntax")

#endif
