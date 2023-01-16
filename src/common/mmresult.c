/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmresult.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <string.h>

#include "mmresult.h"

const char *mmresult_to_string(MmResult result) {
    if (result > kOk && result < kError) {
        return strerror(result);
    }

    switch (result) {
        case kOk:            return "No error";
        case kError:         return "MMBasic error";
        case kInternalFault: return "Internal fault (sorry)";
        case kSyntax:        return "Syntax";
        case kStringTooLong: return "String too long";
        case kInvalidFormat: return "Invalid format";
        case kUnknownOption: return "Unknown option";
        case kInvalidString: return "Invalid string value";
        case kInvalidValue:  return "Invalid value";
        case kUnknownSystemCommand: return "Unknown system command";
        case kNotAFunction:         return "Not a function";
        case kNotASubroutine:       return "Not a subroutine";
        case kNotPersistent:        return "Invalid for non-persistent option";
        case kNameTooLong:          return "Name too long";
        case kUnimplemented:        return "Unimplemented function";
        case kFunctionNotFound:     return "Function not found";
        case kVariableNotFound:     return "Variable not found";
        case kTooManyFunctions:     return "Too many functions/subroutines";
        case kTooManyVariables:     return "Too many variables";
        case kDuplicateFunction:    return "Duplicate function/subroutine declaration";
        case kHashmapFull:          return "Hashmap full";
        case kInvalidName:          return "Invalid name";
        case kInvalidArrayDimensions: return "Dimensions";
        default:                    return "Unknown result code";
    }
}
