/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmresult.c

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

#include "cstring.h"
#include "mmresult.h"
#include "../Configuration.h"

#include <string.h>

const char *audio_last_error();
const char *events_last_error();
const char *gamepad_last_error();
const char *graphics_last_error();

MmResult mmresult_last_code = kOk;
char mmresult_last_msg[STRINGSIZE] = { 0 };

static void formatAudioApiError() {
    snprintf(mmresult_last_msg, STRINGSIZE, "Audio error: %s", audio_last_error());
}

static void formatEventsApiError() {
    snprintf(mmresult_last_msg, STRINGSIZE, "Events error: %s", events_last_error());
}

static void formatGamepadApiError() {
    snprintf(mmresult_last_msg, STRINGSIZE, "Gamepad error: %s", gamepad_last_error());
}

static void formatGraphicsApiError() {
    snprintf(mmresult_last_msg, STRINGSIZE, "Graphics error: %s", graphics_last_error());
}

const char *mmresult_to_string(MmResult result) {
    if (result && result == mmresult_last_code) {
        // Use cached message.
        return mmresult_last_msg;
    }

    if (result > kOk && result < kError) {
        if (result == kFilenameTooLong) return "Pathname too long";
        return strerror(result);
    }

    switch (result) {
        case kOk:            return "No error";
        case kError:         return "MMBasic error";
        case kInternalFault: return "Internal fault (sorry)";
        case kSyntax:        return "Syntax";
        case kArgumentCount: return "Argument count";
        case kStringTooLong: return "String too long";
        case kInvalidFormat: return "Invalid format";
        case kUnknownOption: return "Unknown option";
        case kInvalidArray:  return "Invalid array value";
        case kInvalidBool:   return "Invalid boolean value";
        case kInvalidFloat:  return "Invalid float value";
        case kInvalidInt:    return "Invalid integer value";
        case kInvalidString: return "Invalid string value";
        case kInvalidValue:  return "Invalid value";
        case kUnknownSystemCommand: return "Unknown system command";
        case kOverflow:             return "Overflow";
        case kNotPersistent:        return "Invalid for non-persistent option";
        case kNameTooLong:          return "Name too long";
        case kUnimplemented:        return "Unimplemented function";
        case kFunctionNotFound:     return "Function not found";
        case kVariableNotFound:     return "Variable not found";
        case kTooManyFunctions:     return "Too many functions/labels/subroutines";
        case kTooManyVariables:     return "Too many variables";
        case kDuplicateFunction:    return "Function/subroutine already declared";
        case kHashmapFull:          return "Hashmap full";
        case kInvalidName:          return "Invalid name";
        case kInvalidArrayDimensions: return "Dimensions";
        case kFunctionTypeMismatch: return "Not a function";
        case kInvalidCommandLine: return "Invalid command line arguments";
        case kStringLength:         return "String length";
        case kTooManyDefines:             return "Too many #DEFINE directives";
        case kOutOfMemory:                return "Not enough memory";
        case kLineTooLong:                return "Line too long";
        case kProgramTooLong:             return "Program too long";
        case kUnterminatedComment:        return "Unterminated multiline comment";
        case kNoCommentToTerminate:       return "No comment to terminate";
        case kAudioApiError:
            formatAudioApiError();
            return mmresult_last_msg;
        case kEventsApiError:
            formatEventsApiError();
            return mmresult_last_msg;
        case kGamepadApiError:
            formatGamepadApiError();
            return mmresult_last_msg;
        case kGraphicsApiError:
            formatGraphicsApiError();
            return mmresult_last_msg;
        case kGraphicsInvalidId:          return "Invalid graphics surface ID";
        case kGraphicsSurfaceNotCreated:  return "Graphics surface could not be created";
        case kGraphicsSurfaceNotFound:    return "Graphics surface does not exist";
        case kGraphicsSurfaceExists:      return "Graphics surface already exists";
        case kGraphicsSurfaceTooLarge:    return "Graphics surface too large";
        default:                          return "Unknown result code";
    }
}
