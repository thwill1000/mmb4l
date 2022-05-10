/*-*****************************************************************************

MMBasic for Linux (MMB4L)

error.c

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

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/exit_codes.h"
#include "../common/options.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"

extern jmp_buf ErrNext;

static char error_buffer[STRINGSIZE] = { 0 };
static size_t error_buffer_pos = 0;

void error_init(ErrorState *error_state) {
    error_state->code = 0;
    *error_state->file = '\0';
    error_state->line = -1;
    *error_state->message = '\0';
    error_state->skip = 0;
}

static void MMErrorString(const char *msg) {
    char *src = (char *) msg;
    char *dst = error_buffer + error_buffer_pos;
    while (*src) *dst++ = *src++;
    error_buffer_pos += strlen(msg);
    error_buffer[error_buffer_pos] = '\0';
}

void MMErrorChar(char c) {
    error_buffer[error_buffer_pos++] = c;
}

static void get_line_and_file(int *line, char *file_path) {

    *line = -1;
    memset(file_path, 0, STRINGSIZE);

    if (!CurrentLinePtr) return;

    if (CurrentLinePtr >= (char *) ProgMemory + PROG_FLASH_SIZE) return;

    if (*CurrentLinePtr != T_NEWLINE) {
        // normally CurrentLinePtr points to a T_NEWLINE token but in this
        // case it does not so we have to search for the start of the line
        // and set CurrentLinePtr to that
        char *p = (char *) ProgMemory;
        char *tp = p;
        while (*p != 0xff) {
            while (*p)
                p++;  // look for the zero marking the start of an element
            if (p >= CurrentLinePtr ||
                p[1] == 0) {  // the previous line was the one that we wanted
                CurrentLinePtr = tp;
                break;
            }
            if (p[1] == T_NEWLINE) {
                tp = ++p;  // save because it might be the line we want
            }
            p++;  // step over the zero marking the start of the element
            skipspace(p);
            if (p[0] == T_LABEL) p += p[1] + 2;  // skip over the label
        }
    }

    if (CurrentLinePtr >= (char *) ProgMemory + PROG_FLASH_SIZE) return;

    // We now have CurrentLinePtr pointing to the start of the line.
    llist(tknbuf, CurrentLinePtr);
    // p = tknbuf;
    // skipspace(p);

    char *pipe_pos = strchr(tknbuf, '|');
    if (!pipe_pos) return;

    char *comma_pos = strchr(pipe_pos, ',');
    if (comma_pos) {
        // Line is from a #included file.
        pipe_pos++;
        comma_pos++;
        *line = atoi(comma_pos);

        if (!path_get_parent(CurrentFile, file_path, STRINGSIZE)) return;
        // TODO: prevent buffer overflow.
        int len = strlen(file_path);
        file_path[len++] = '/';
        memcpy(file_path + len, pipe_pos, comma_pos - pipe_pos - 1);
        file_path[len + comma_pos - pipe_pos] = '\0';
    } else {
        // Line is from the main file.
        pipe_pos++;
        *line = atoi(pipe_pos);
        strcpy(file_path, CurrentFile);
    }
}

void error_buffer_clear(void) {
    error_buffer_pos = 0;
    memset(error_buffer, 0, STRINGSIZE);
}

// throw an error
// displays the error message and aborts the program
// the message can contain variable text which is indicated by a special character in the message string
//  $ = insert a string at this place
//  @ = insert a character
//  % = insert a number
// the optional data to be inserted is the second argument to this function
// this uses longjump to skip back to the command input and cleanup the stack
static void verror(MmResult error, const char *msg, va_list argp) {
    // ScrewUpTimer=0;
    mmb_error_state.code = error;
    error_buffer_clear();
    options_load(&mmb_options, OPTIONS_FILE_NAME, NULL);  // make sure that the option struct is in a clean state

    // if((OptionConsole & 2) && !mmb_error_state.skip) {
    //     SetFont(PromptFont);
    //     gui_fcolour = PromptFC;
    //     gui_bcolour = PromptBC;
    //     if(CurrentX != 0) MMErrorString("\r\n");                   // error
    //     message should be on a new line
    // }

    if (MMCharPos > 1 && !mmb_error_state.skip) MMErrorString("\r\n");

    get_line_and_file(&mmb_error_state.line, mmb_error_state.file);

    if (mmb_error_state.line > 0) {
        char buf[STRINGSIZE * 2];
        if (strcmp(mmb_error_state.file, CurrentFile) == 0) {
            sprintf(buf, "Error in line %d: ", mmb_error_state.line);
        } else {
            sprintf(buf, "Error in %s line %d: ", mmb_error_state.file, mmb_error_state.line);
        }
        MMErrorString(buf);
    } else {
        MMErrorString("Error: ");
    }

    if (mmb_error_state.skip) {
        *mmb_error_state.file = '\0';
        mmb_error_state.line = -1;
    }

    if (*msg) {
        while (*msg) {
            if (*msg == '$')
                MMErrorString(va_arg(argp, char *));
            else if (*msg == '@')
                MMErrorChar(va_arg(argp, int));
            else if (*msg == '%' || *msg == '|') {
                char buf[20];
                IntToStr(buf, va_arg(argp, int), 10);
                MMErrorString(buf);
            } else if (*msg == '&') {
                char buf[20];
                IntToStr(buf, va_arg(argp, int), 16);
                MMErrorString(buf);
            } else {
                MMErrorChar(*msg);
            }
            msg++;
        }
        if (!mmb_error_state.skip) MMErrorString("\r\n");
    }

    // Don't overflow mmb_error_state.message.
    strncpy(mmb_error_state.message, error_buffer, MAXERRMSG - 1);
    mmb_error_state.message[MAXERRMSG - 1] = '\0';

    if (mmb_error_state.skip) {
        longjmp(ErrNext, 1);
    } else {
        longjmp(mark, JMP_ERROR);
    }
}

void error_throw_legacy(const char *msg, ...) {
    va_list argp;
    va_start(argp, msg);
    verror(kError, msg, argp);
    assert(0); // Don't expect to get here because of long_jmp().
    va_end(argp);
}

void error_throw_ex(MmResult error, const char *msg, ...) {
    va_list argp;
    va_start(argp, msg);
    verror(error, msg, argp);
    assert(0); // Don't expect to get here because of long_jmp().
    va_end(argp);
}

void error_throw(MmResult error) {
    error_throw_ex(error, mmresult_to_string(error));
}

uint8_t error_to_exit_code(MmResult error) {
    switch (error) {
        default:
            return EX_FAIL;
    }
}
