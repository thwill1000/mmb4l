#if !defined(MMB4L_ERROR_H)
#define MMB4L_ERROR_H

#include <stdint.h>

#include "../Configuration.h" // for STRINGSIZE
#include "mmresult.h"

extern char error_file[STRINGSIZE];
extern int error_line;

void error(char *msg, ...);
void error_throw(MmResult error);
void error_throw_ex(MmResult error, const char *msg, ...);
uint8_t error_to_exit_code(MmResult error);

#define ERRNO_DEFAULT  1

#define ERROR_ALREADY_OPEN             error("File or device already open")
#define ERROR_ARGUMENT_COUNT           error("Argument count")
#define ERROR_UNKNOWN_COMMAND          error("Unknown command")
#define ERROR_UNKNOWN_SUBCOMMAND       error("Unknown subcommand")
#define ERROR_ARGUMENT_NOT_INTEGER(s)  error("Argument " s " must be an integer")
#define ERROR_ARGUMENT_NOT_INTEGER_ARRAY(s)  error("Argument " s " must be an integer array")
#define ERROR_COULD_NOT(s)             error("Could not " s)
#define ERROR_INTERNAL_FAULT           error("Internal fault (sorry)")
#define ERROR_INVALID(s)               error("Invalid " s)
#define ERROR_INVALID_ARGUMENT         ERROR_INVALID("argument")
#define ERROR_INVALID_FILE_NUMBER      ERROR_INVALID("file number")
#define ERROR_INVALID_IN_PROGRAM       ERROR_INVALID("in a program")
#define ERROR_INVALID_OPTION_VALUE     ERROR_INVALID("value for option")
#define ERROR_LINE_LENGTH              error("Line length")
#define ERROR_NOT_ALLOWED(s)           error(s " not allowed")
#define ERROR_NOT_OPEN                 error("File or device not open")
#define ERROR_NOT_SERIAL_PORT          error("Not a serial port")
#define ERROR_OUT_OF_MEMORY            error("Not enough memory")
#define ERROR_PATH_TOO_LONG            error_throw_ex(ENAMETOOLONG, "Path too long")
#define ERROR_STRING_TOO_LONG          error("String too long")
#define ERROR_SYNTAX                   error("Syntax")
#define ERROR_UNIMPLEMENTED(s)         error("Unimplemented: " s)
#define ERROR_UNKNOWN_OPTION           error("Unknown option")
#define ERROR_UNSUPPORTED_FLAG(s)      error("Unsupported flag: " s)

#endif
