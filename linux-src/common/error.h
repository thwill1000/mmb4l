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

#define ERROR_ALREADY_OPEN                error_throw_ex(kError, "File or device already open")
#define ERROR_ARGUMENT_COUNT              error_throw_ex(kSyntax, "Argument count")
#define ERROR_ARRAY_NOT_SQUARE            error_throw_ex(kError, "Array must be square")
#define ERROR_ARRAY_SIZE_MISMATCH         error_throw_ex(kError, "Array size mismatch")
#define ERROR_ARG_NOT_ARRAY(i)            error_throw_ex(kError, "Argument % must be an array")
#define ERROR_ARG_NOT_FLOAT_ARRAY(i)      error_throw_ex(kError, "Argument % must be a floating point array", i)
#define ERROR_ARG_NOT_INTEGER(i)          error_throw_ex(kError, "Argument % must be an integer", i)
#define ERROR_ARG_NOT_INTEGER_ARRAY(i)    error_throw_ex(kError, "Argument % must be an integer array", i)
#define ERROR_ARG_NOT_NUMBER(i)           error_throw_ex(kError, "Argument % must be a number", i)
#define ERROR_ARG_NOT_NUMBER_ARRAY(i)     error_throw_ex(kError, "Argument % must be a number array", i)
#define ERROR_ARG_NOT_2D_FLOAT_ARRAY(i)   error_throw_ex(kError, "Argument % must be a 2D floating point array", i)
#define ERROR_ARG_NOT_2D_NUMBER_ARRAY(i)  error_throw_ex(kError, "Argument % must be a 2D number array", i)
#define ERROR_CANNOT_CHANGE_A_CONSTANT    error_throw_ex(kError, "Cannot change a constant")
#define ERROR_COULD_NOT(s)                error_throw_ex(kError, "Could not $", s)
#define ERROR_DST_ARRAY_TOO_SMALL         error_throw_ex(kError, "Destination array too small")
#define ERROR_ENV_VAR_TOO_LONG            error_throw_ex(kStringTooLong, "Environment variable value too long")
#define ERROR_INTEGER_ARRAY_TOO_SMALL     error_throw_ex(kError, "Integer array too small")
#define ERROR_INTERNAL_FAULT              error_throw(kInternalFault)
#define ERROR_INVALID(s)                  error_throw_ex(kError, "Invalid $", s)
#define ERROR_INVALID_ADDRESS             ERROR_INVALID("address")
#define ERROR_INVALID_ARGUMENT            ERROR_INVALID("argument")
#define ERROR_INVALID_CHARACTER           ERROR_INVALID("character")
#define ERROR_INVALID_FILE_NUMBER         ERROR_INVALID("file number")
#define ERROR_INVALID_IN_PROGRAM          ERROR_INVALID("in a program")
#define ERROR_INVALID_OPTION_VALUE        ERROR_INVALID("value for option")
#define ERROR_INVALID_VARIABLE            ERROR_INVALID("variable")
#define ERROR_LINE_LENGTH                 error_throw_ex(kStringTooLong, "Line length")
#define ERROR_LINE_TOO_LONG               error_throw_ex(kStringTooLong, "Line too long")
#define ERROR_NOT_ALLOWED(s)              error_throw_ex(kError, "$ not allowed", s)
#define ERROR_NOT_OPEN                    error_throw_ex(kError, "File or device not open")
#define ERROR_NOT_SERIAL_PORT             error_throw_ex(kError, "Not a serial port")
#define ERROR_NUMBER_OUT_OF_BOUNDS        error_throw_ex(kError, "Number out of bounds")
#define ERROR_OUT_OF_MEMORY               error_throw_ex(kError, "Not enough memory")
#define ERROR_PATH_TOO_LONG               error_throw_ex(kFilenameTooLong, "Path too long")
#define ERROR_SIZE_MISMATCH               error_throw_ex(kError, "Size mismatch")
#define ERROR_STRING_TOO_LONG             error_throw(kStringTooLong)
#define ERROR_SYNTAX                      error_throw(kSyntax)
#define ERROR_SYSTEM_COMMAND_FAILED       error_throw_ex(kError, "System command failed")
#define ERROR_TOO_MANY_OPEN_FILES         error_throw_ex(kError, "Too many open files")
#define ERROR_UNIMPLEMENTED(s)            error_throw_ex(kUnimplemented, "Unimplemented: $", s)
#define ERROR_UNKNOWN_ARGUMENT            error_throw_ex(kError, "Unknown argument")
#define ERROR_UNKNOWN_COMMAND             error_throw_ex(kSyntax, "Unknown command")
#define ERROR_UNKNOWN_OPTION              error_throw(kUnknownOption)
#define ERROR_UNKNOWN_SUBCOMMAND          error_throw_ex(kSyntax, "Unknown subcommand")
#define ERROR_UNKNOWN_USER_ERROR          error_throw_ex(kError, "Unspecified error")
#define ERROR_UNSUPPORTED_FLAG(s)         error_throw_ex(kError, "Unsupported flag: $", s)

#endif
