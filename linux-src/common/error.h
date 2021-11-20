#if !defined(MMB4L_ERROR_H)
#define MMB4L_ERROR_H

#include <stdint.h>

#include "../Configuration.h" // for STRINGSIZE

extern char error_file[STRINGSIZE];
extern int error_line;

void error(char *msg, ...);
void error_code(int32_t error, char *msg, ...);
void error_system(int32_t error);
int error_check();
uint8_t error_to_exit_code(int32_t error_code);

#define ERRNO_DEFAULT  1

#define ERROR_ARGUMENT_COUNT           error("Argument count")
#define ERROR_UNKNOWN_COMMAND          error("Unknown command")
#define ERROR_UNKNOWN_SUBCOMMAND       error("Unknown subcommand")
#define ERROR_ARGUMENT_NOT_INTEGER(s)  error("Argument " s " must be an integer")
#define ERROR_ARGUMENT_NOT_INTEGER_ARRAY(s)  error("Argument " s " must be an integer array")
#define ERROR_COULD_NOT(s)             error("Could not " s)
#define ERROR_INTERNAL_FAULT           error("Internal fault (sorry)")
#define ERROR_INVALID(s)               error("Invalid " s)
#define ERROR_INVALID_ARGUMENT         ERROR_INVALID("argument")
#define ERROR_INVALID_IN_PROGRAM       ERROR_INVALID("in a program")
#define ERROR_LINE_LENGTH              error("Line length")
#define ERROR_NOT_ALLOWED(s)           error(s " not allowed")
#define ERROR_OUT_OF_MEMORY            error("Not enough memory")
#define ERROR_SYNTAX                   error("Syntax")
#define ERROR_UNIMPLEMENTED(s)         error("Unimplemented: " s)
#define ERROR_UNRECOGNISED_OPTION      error("Unrecognised option")

#endif
