/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_xmodem.c

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

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/xmodem.h"

// TODO: Disable and restore break key ?

void cmd_xmodem(void) {
    char *p;
    bool receive;
    if ((p = checkstring(cmdline, "RECEIVE"))) {
        receive = true;
    } else if ((p = checkstring(cmdline, "SEND"))) {
        receive = false;
    } else {
        ERROR_SYNTAX;
    }

    char *filename;
    int serial_fnbr;

    {
        getargs(&p, 3, ",");  // Macro needs to start a new block.
        if (argc != 3) ERROR_ARGUMENT_COUNT;

        filename = getCstring(argv[0]);
        serial_fnbr = parse_file_number(argv[2], false);
        if (serial_fnbr == -1) ERROR_INVALID_FILE_NUMBER;
    }

    int file_fnbr = file_find_free();
    file_open(filename, receive ? "wb" : "rb", file_fnbr);

    if (receive) {
        xmodem_receive(file_fnbr, serial_fnbr);
    } else {
        xmodem_send(file_fnbr, serial_fnbr);
    }

    file_close(file_fnbr);
}
