/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_copy.c

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

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utility.h"
#include "../core/tokentbl.h"

void cmd_copy(void) {
    char ss[2] = { tokenTO, '\0' };
    getargs(&cmdline, 3, ss);
    if (argc != 3) ON_FAILURE_ERROR(kArgumentCount);

    char *src_filename = GetTempStrMemory();
    ON_FAILURE_ERROR(parse_filename(argv[0], src_filename, STRINGSIZE));

    char *dst_filename = GetTempStrMemory();
    ON_FAILURE_ERROR(parse_filename(argv[2], dst_filename, STRINGSIZE));

    const int src_fnbr = file_find_free();
    ON_FAILURE_ERROR(file_open(src_filename, "r", src_fnbr));

    const int dst_fnbr = file_find_free();
    MmResult result = file_open(dst_filename, "w", dst_fnbr);  // We'll just overwrite any existing file
    if (FAILED(result)) {
        (void) file_close(src_fnbr);
        ON_FAILURE_ERROR(result);
    }

    char c;
    while (1) {
        if (file_eof(src_fnbr)) break;
        c = file_getc(src_fnbr);
        file_putc(dst_fnbr, c);
    }

    result = file_close(src_fnbr);
    if (FAILED(result)) {
        (void) file_close(dst_fnbr);
        ON_FAILURE_ERROR(result);
    }
    ON_FAILURE_ERROR(file_close(dst_fnbr));
}
