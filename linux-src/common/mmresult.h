#if !defined(MMRESULT_H)
#define MMRESULT_H

#include <errno.h>
#include <stdint.h>

// MmResult encompasses both the standard C errno in range 1 .. 255
// plus MMBasic specific error codes.

typedef int32_t MmResult;

typedef enum {
    kOk              = 0,
    kFileNotFound    = ENOENT,
    kNotADirectory   = ENOTDIR,
    kFilenameTooLong = ENAMETOOLONG,
    kError           = 256,
    kInternalFault,
    kSyntax,
    kStringTooLong,
    kInvalidFormat,
    kUnknownOption,
    kInvalidBool,
    kInvalidFloat,
    kInvalidInt,
    kInvalidString,
    kInvalidValue,
    kUnknownSystemCommand,
    kUnimplemented
} MmResultCode;

/**
 * @brief Gets the string corresponding to a given result code.
 */
const char *mmresult_to_string(MmResult result);

#endif
