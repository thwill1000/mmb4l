/*-*****************************************************************************

MMBasic for Linux (MMB4L)

MMBasic.c

Copyright 2011-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(COMMANDTBL_H)
#define COMMANDTBL_H

#include <stdint.h>

typedef uint16_t CommandToken;

struct s_tokentbl {      // structure of the command and token tables.
    const char *name;    // the string (eg, PRINT, FOR, ASC(, etc)
    char type;           // the type returned (T_NBR, T_STR, T_INT)
    char precedence;     // precedence used by operators only.  operators with equal precedence are processed left to right.
    void (*fptr)(void);  // pointer to the function that will interpret that token
};

#define C_BASETOKEN            0x80  // the base of the token numbers
#define INVALID_COMMAND_TOKEN  0xFFFF   

#define GetCommandValue(s)  commandtbl_get(s)
#define CommandTableSize    commandtbl_size

/** Gets the type of a command. */
#define commandtype(i)  ((i < CommandTableSize - 1) ? commandtbl[i].type : 0)

/** Gets the function pointer of a command. */
#define commandfunction(i)  ((i < CommandTableSize - 1) ? commandtbl[i].fptr : commandtbl[0].fptr)

/** Gets the name of a command. */
#define commandname(i)  ((i < CommandTableSize - 1) ? commandtbl[i].name : "")

void cmd_autosave(void);
void cmd_blit();
void cmd_box(void);
void cmd_call(void);
void cmd_case(void);
void cmd_cfunction(void);
void cmd_chdir(void);
void cmd_circle(void);
void cmd_clear(void);
void cmd_close(void);
void cmd_cls(void);
void cmd_colour(void);
void cmd_console(void);
void cmd_const(void);
void cmd_continue(void);
void cmd_copy(void);
void cmd_cursor(void);
void cmd_dim(void);
void cmd_do(void);
void cmd_device(void);
void cmd_dummy(void);
void cmd_edit(void);
void cmd_else(void);
void cmd_endfun(void);
void cmd_endsub(void);
void cmd_end(void);
void cmd_erase(void);
void cmd_error(void);
void cmd_execute(void);
void cmd_exitfor(void);
void cmd_exit(void);
void cmd_files(void);
void cmd_font(void);
void cmd_for(void);
void cmd_gosub(void);
void cmd_goto(void);
void cmd_graphics(void);
void cmd_if(void);
void cmd_image(void);
void cmd_inc(void);
void cmd_input(void);
void cmd_ireturn(void);
void cmd_kill(void);
void cmd_let(void);
void cmd_line(void);
void cmd_lineinput(void);
void cmd_list(void);
void cmd_load(void);
void cmd_longstring(void);
void cmd_loop(void);
void cmd_math(void);
void cmd_memory(void);
void cmd_mid(void);
void cmd_mkdir(void);
void cmd_mmdebug(void);
void cmd_mode(void);
void cmd_new(void);
void cmd_next(void);
void cmd_null(void);
void cmd_null(void);
void cmd_on(void);
void cmd_open(void);
void cmd_option(void);
void cmd_pause(void);
void cmd_pixel(void);
void cmd_play(void);
void cmd_poke(void);
void cmd_print(void);
void cmd_quit(void);
void cmd_randomize(void);
void cmd_rbox(void);
void cmd_read(void);
void cmd_rename(void);
void cmd_restore(void);
void cmd_return(void);
void cmd_rmdir(void);
void cmd_run(void);
void cmd_seek(void);
void cmd_select(void);
void cmd_settick(void);
void cmd_settitle(void);
void cmd_sort(void);
void cmd_subfun(void);
void cmd_system(void);
void cmd_text(void);
void cmd_timer(void);
void cmd_trace(void);
void cmd_triangle(void);
void cmd_troff(void);
void cmd_tron(void);
void cmd_xmodem(void);

void commandtbl_init();
CommandToken commandtbl_get(const char *s);

static inline CommandToken commandtbl_decode(const char *p) {
   return ((CommandToken)(p[0] & 0x7F)) | ((CommandToken)(p[1] & 0x7F) << 7);
}

static inline void commandtbl_encode(char **p, CommandToken cmd) {
  *((*p)++) = (char) (cmd & 0x7F) + C_BASETOKEN;
  *((*p)++) = (char) (cmd >> 7) + C_BASETOKEN;
}

extern const struct s_tokentbl commandtbl[];
extern int commandtbl_size;

// Store commonly used commands for faster token checking.
extern CommandToken cmdCASE, cmdCASE_ELSE, cmdCFUN, cmdCSUB, cmdDATA, cmdDEFINEFONT, cmdDO;
extern CommandToken cmdELSE, cmdELSEIF, cmdELSE_IF, cmdENDIF, cmdEND_CSUB, cmdEND_DEFINEFONT;
extern CommandToken cmdEND_FUNCTION;
extern CommandToken cmdENDIF, cmdEND_IF, cmdEND_SELECT, cmdEND_SUB, cmdFOR, cmdFUN;
extern CommandToken cmdIF, cmdIRET, cmdLET, cmdLOOP, cmdNEXT, cmdPRINT;
extern CommandToken cmdREM, cmdSELECT_CASE, cmdSUB, cmdWEND, cmdWHILE;

#endif
