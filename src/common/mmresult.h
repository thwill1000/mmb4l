/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmresult.h

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#if !defined(MMRESULT_H)
#define MMRESULT_H

#include <errno.h>
#include <stdint.h>

// MmResult encompasses both the standard C errno in range 1 .. 255
// plus MMBasic specific error codes.

typedef int32_t MmResult;

typedef enum {
    kOk                   = 0,
    kFileNotFound         = ENOENT,
    kPermissionDenied     = EACCES,
    kFileExists           = EEXIST,
    kNotADirectory        = ENOTDIR,
    kIsADirectory         = EISDIR,
    kFilenameTooLong      = ENAMETOOLONG,
    kTooManySymbolicLinks = ELOOP,
    kError                = 256,
    kInternalFault,
    kSyntax,
    kArgumentCount,
    kStringLength,
    kStringTooLong,
    kInvalidFormat,
    kUnknownOption,
    kInvalidBool,
    kInvalidFloat,
    kInvalidInt,
    kInvalidString,
    kInvalidValue,
    kUnknownSystemCommand,
    kNotPersistent,
    kNameTooLong,
    kOverflow,
    kUnimplemented,
    kFunctionNotFound,
    kVariableNotFound,
    kTooManyFunctions,
    kTooManyVariables,
    kDuplicateFunction,
    kHashmapFull,
    kInvalidName,
    kInvalidArrayDimensions,
    kFunctionTypeMismatch,
    kInvalidCommandLine,
    kTooManyDefines,
    kOutOfMemory,
    kLineTooLong,
    kProgramTooLong,
    kUnterminatedComment,
    kNoCommentToTerminate,
    kAudioApiError,
    kEventsApiError,
    kGamepadApiError,
    kGraphicsApiError,
    kGraphicsInvalidId,
    kGraphicsInvalidWriteSurface,
    kGraphicsSurfaceNotCreated,
    kGraphicsSurfaceNotFound,
    kGraphicsSurfaceExists,
    kGraphicsSurfaceTooLarge,
    kImageTooLarge,
    kImageInvalidFormat,
    kInvalidFont,
    kInvalidFontScaling,
    kUnknownDevice,
    kUnsupportedOnCurrentDevice,
    kInvalidMode,
    kInvalidFlag,
    kUnexpectedText,
    kCannotBlitCloseWindow,
    kMissingType,
    kTypeSpecifiedTwice,
    kInvalidFunctionDefinition,
    kInvalidSubDefinition,
    kMissingCloseBracket,
    kMissingOpenBracket,
    kUnexpectedCloseBracket,
    kInvalidArrayParameter,
    kTooManyParameters,
    kInvalidInterruptSignature,
    kGamepadNotFound,
    kGamepadInvalidId,
    kGamepadNotOpen
} MmResultCode;

/**
 * @brief Gets the string corresponding to a given result code.
 */
const char *mmresult_to_string(MmResult result);

#endif
