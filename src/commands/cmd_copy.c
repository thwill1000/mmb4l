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
#include "../core/tokentbl.h"

void cmd_copy(void) {  // thanks to Bryan Rentoul for the contribution
    char *oldf, *newf, ss[2];
    char c;
    int of, nf;

    ss[0] = tokenTO;  // this will be used to split up the argument line
    ss[1] = 0;
    {
        getargs(&cmdline, 3, ss);    // getargs macro must be the first executable stmt in a block
        if (argc != 3) ERROR_SYNTAX;
        oldf = getCstring(argv[0]);  // get the old file name and convert to a
                                     // standard C string
        newf = getCstring(argv[2]);  // get the new file name and convert to a
                                     // standard C string

        of = file_find_free();
        file_open(oldf, "r", of);

        nf = file_find_free();
        file_open(newf, "w", nf);  // We'll just overwrite any existing file
    }

    while (1) {
        if (file_eof(of)) break;
        c = file_getc(of);
        file_putc(nf, c);
    }

    file_close(of);
    file_close(nf);
}
