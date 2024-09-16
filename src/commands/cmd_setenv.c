/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_setenv.c

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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../common/error.h"
#include "../common/mmb4l.h"
#include "../core/tokentbl.h"

/** SETENV name$ = value$ */
void cmd_setenv(void) {
    char ss[3];
    ss[0] = tokenEQUAL;
    ss[1] =',';
    ss[2] = 0;
    getargs(&cmdline, 3, ss);
    if (argc != 3) ERROR_ON_FAILURE(kArgumentCount);
    const char *name = getCstring(argv[0]);
    const char *value = getCstring(argv[2]);

    // 'name' restricted to uppercase letters, digits and '_'.
    // It should not begin with a digit.
    if (strlen(name) == 0) ERROR_ON_FAILURE(kInvalidEnvironmentVariableName);
    bool first = true;
    for (const char *p = name; *p; ++p) {
        if (first) {
            if (!isupper(*p) && *p != '_') ERROR_ON_FAILURE(kInvalidEnvironmentVariableName);
            first = false;
        } else {
            if (!isupper(*p) && !isdigit(*p) && *p != '_') ERROR_ON_FAILURE(kInvalidEnvironmentVariableName);
        }
    }

    if (FAILED(setenv(name, value, 1))) ERROR_ON_FAILURE(errno);
}
