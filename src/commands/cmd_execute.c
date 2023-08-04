/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_execute.c

Copyright 2021-2023 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <string.h>

#define ERROR_MULTIPLE_STATEMENTS  error_throw_ex(kError, "Only single statements allowed")

// char execute_buffer[STRINGSIZE] = { 0 };

// void execute_run() {
//     char *p = inpbuf;
//     char *q, *s;
//     char fn[FF_MAX_LFN];
//     FRESULT fr;
//     FILINFO fno;
//     p[0] = GetCommandValue("RUN");
//     memmove(&p[1], &p[4], strlen(p) - 4);
//     if ((q = strchr(p, ':'))) {
//         q--;
//         *q = '0';
//     }
//     p[strlen(p) - 3] = 0;
//     if ((q = strchr(p, '\"')) != 0) {
//         if ((s = strchr(&q[1], '\"')) != 0) *s = 0;
//         strcpy(fn, &q[1]);
//         strcpy(fn, q);
//         if (strchr(fn, '.') == NULL) {
//             cstring_cat(fn, ".BAS", STRINGSIZE);
//         }
//         if (!InitSDCard()) {
//         };  // make sure the SDcard is there
//         fr = f_stat(&fn[1],
//                     &fno);  // check for the file in the current directory
//         *s = '\"';
//         if (fr != FR_OK ||
//             (fno.fattrib & AM_DIR)) {  // file was not in the current
//                                        // directory so insert the path
//             int slen = strlen(p) - (uint32_t)p + (uint32_t)&q[1];
//             memmove(&q[1 + strlen((char *)Option.path)], &q[1], slen + 1);
//             memcpy(&q[1], (char *)Option.path, strlen((char *)Option.path));
//         }
//     }
//     CloseAudio(1);
//     strcpy(tknbuf, inpbuf);
//     longjmp(run, 1);
// }

void execute_other() {
    int i = 0;
    int toggle = 0;

    while (inpbuf[i]) {
        if (inpbuf[i] == 34) {
            if (toggle == 0)
                toggle = 1;
            else
                toggle = 0;
        }
        if (!toggle) {
            if (inpbuf[i] == ':') ERROR_MULTIPLE_STATEMENTS;
            // inpbuf[i] = toupper(inpbuf[i]);
        }
        i++;
    }
    tokenise(true);  // and tokenise it (the result is in tknbuf)
    memset(inpbuf, 0, INPBUF_SIZE);
    tknbuf[strlen(tknbuf)] = 0;
    tknbuf[strlen(tknbuf) + 1] = 0;

    const char *nextstmt_cache = nextstmt;  // save the globals used by commands
    // ScrewUpTimer = 1000;
    ExecuteProgram(tknbuf);  // execute the function's code
    // ScrewUpTimer = 0;
    // TempMemoryIsChanged = true;                                     //
    // signal that temporary memory should be checked
    nextstmt = nextstmt_cache;
}

void execute(const char *mycmd) {
    skipspace(mycmd);
    strcpy(inpbuf, getCstring(mycmd));
    execute_other();
    // if (toupper(inpbuf[0]) == 'R'
    //         && toupper(inpbuf[1]) == 'U'
    //         && toupper(inpbuf[2]) == 'N') {
    //     execute_run();
    // } else {
    //     execute_other();
    // }
}

void cmd_execute(void) {
    execute(cmdline);
}
