/*-*****************************************************************************

MMBasic for Linux (MMB4L)

main.c

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

#include "common/mmb4l.h"
#include "common/cmdline.h"
#include "common/console.h"
#include "common/cstring.h"
#include "common/exit_codes.h"
#include "common/file.h"
#include "common/interrupt.h"
#include "common/mmtime.h"
#include "common/parse.h"
#include "common/path.h"
#include "common/program.h"
#include "common/prompt.h"
#include "common/serial.h"
#include "common/utility.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// global variables used in MMBasic but must be maintained outside of the
// interpreter
volatile int MMAbort = false;
ErrorState mmb_normal_error_state;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
int WatchdogSet, IgnorePIN;
char *OnKeyGOSUB;
char *CFunctionFlash, *CFunctionLibrary, **FontTable;

CmdLineArgs mmb_args = { 0 };
uint8_t mmb_exit_code = EX_OK;

static const char mmbasic_dir[] = "~/.mmbasic";

void IntHandler(int signo);
void dump_token_table(const struct s_tokentbl* tbl);

/**
 * If 'true' then RUN program specified by 'mmb_args.run_cmd'.
 * Only used within main() but cannot be a local variable (on the stack)
 * because after a longjmp() it would be restored to its value when
 * setjmp() was called - this is undesireable.
 */
static bool run_flag;

void print_banner() {
    console_puts(MES_SIGNON);
    console_puts(COPYRIGHT);
}

static void init_mmbasic_config_dir() {
    MmResult result = path_mkdir(mmbasic_dir);
    if (FAILED(result)) {
        fprintf(
            stderr,
            "\nFailed to create directory '%s': %s\n",
            mmbasic_dir,
            mmresult_to_string(result));
        exit(EX_FAIL);
    }
}

static void init_options_cb(const char *msg) {
    static int count = 0;

    if (strcmp(msg, "END") == 0) {
        if (count > 0) console_puts("\r\n");
        return;
    }

    if (count == 0) {
        console_puts("Warnings in '");
        console_puts(OPTIONS_FILE_NAME);
        console_puts("':\r\n");
    }

    console_puts(msg);
    console_puts("\r\n");

    count++;
}

static void init_options() {
    options_init(&mmb_options);

    MmResult result = options_load(&mmb_options, OPTIONS_FILE_NAME, init_options_cb);
    switch (result) {
        case kOk:
            // Options loaded, but may still have output warnings.
            break;
        case kFileNotFound:
            // Ignore and use default options.
            break;
        default:
            fprintf(stderr, "\nFailed to load options: %s\n", mmresult_to_string(result));
            exit(EX_FAIL);
            break;
    }
    init_options_cb("END");
}

void set_start_directory() {
    if (mmb_args.directory[0] == '\0') {
        char *MMDIR = getenv("MMDIR");
        if (MMDIR) {
            snprintf(mmb_args.directory, 256, "%s", MMDIR);
            mmb_args.directory[255] = '\0';
        }
    }
    char *p = mmb_args.directory;
    cstring_unquote(p);
    if (p[0] == '\0') return;

    errno = 0;
    if (chdir(p) != 0) {
        console_puts("Error: could not set starting directory '");
        console_puts(p);
        console_puts("'.\r\n");
        console_puts(strerror(errno));
        console_puts(".\r\n");
        console_puts("\r\n");
    }
}

static void reset_console_title() {
    char title[STRINGSIZE + 10];
    sprintf(title, "MMBasic - %s", CurrentFile[0] == '\0' ? "Untitled" : CurrentFile);
    console_set_title(title);
}

/** Handle return via longjmp(). */
void longjmp_handler(int jmp_state) {

    console_show_cursor(true);
    console_reset();
    if (MMCharPos > 1) console_puts("\r\n");

    int do_exit = false;
    switch (jmp_state) {
        case JMP_BREAK:
            mmb_exit_code = EX_BREAK;
            do_exit = !mmb_args.interactive;
            break;

        case JMP_END:
            do_exit = !mmb_args.interactive;
            break;

        case JMP_ERROR:
            console_puts(mmb_error_state_ptr->message);
            console_puts("\r\n");
            mmb_exit_code = error_to_exit_code(mmb_error_state_ptr->code);
            do_exit = !mmb_args.interactive;
            break;

        case JMP_NEW:
            mmb_exit_code = EX_OK; // Probably not necessary.
            do_exit = false;
            break;

        case JMP_QUIT:
            do_exit = true;
            break;

        default:
            fprintf(stderr, "Unexpected return value from setjmp()");
            exit(EX_FAIL);
            break;
    }

    if (do_exit) {
        exit(mmb_exit_code);
    }

    ContinuePoint = nextstmt;  // In case the user wants to use the continue command
    *tknbuf = 0;               // we do not want to run whatever is in the token buffer
    memset(inpbuf, 0, STRINGSIZE);

    reset_console_title();
}

int main(int argc, char *argv[]) {
    if (FAILED(cmdline_parse(argc, (const char **) argv, &mmb_args))) {
        fprintf(stderr, "Invalid command line arguments\n");
        cmdline_print_usage();
        exit(EX_FAIL);
    }

    if (mmb_args.help) {
        cmdline_print_usage();
        exit(EX_OK);
    }

    if (mmb_args.version) {
        print_banner();
        exit(EX_OK);
    }

    ProgMemory[0] = ProgMemory[1] = ProgMemory[2] = 0;

    InitHeap();  // init memory allocation

    console_init();
    console_enable_raw_mode();
    atexit(console_disable_raw_mode);

    if (mmb_args.interactive) {
        reset_console_title();
        console_reset();
        console_clear();
        console_show_cursor(true);

        print_banner();
        console_puts("\r\n");
    }

    init_mmbasic_config_dir();
    init_options();
    error_init(mmb_error_state_ptr);

    InitBasic();

    //printf("Commands\n--------\n");
    //dump_token_table(commandtbl);
    //printf("\n");
    //printf("Tokens\n--------\n");
    //dump_token_table(tokentbl);

# if 0
    signal(SIGBREAK, IntHandler);  // CTRL-C handler
    signal(SIGINT, IntHandler);
#endif

    interrupt_init();
    mmtime_init();
    srand(0);  // seed the random generator with zero
    set_start_directory();

    run_flag = mmb_args.run_cmd[0] != '\0';

    // Note that weird restrictions on what you can do with the return value
    // from setjmp() mean we cannot simply write longjmp_handler(setjmp(mark));
    switch (setjmp(mark)) {
        case 0: break;
        case JMP_BREAK: longjmp_handler(JMP_BREAK); break;
        case JMP_END:   longjmp_handler(JMP_END); break;
        case JMP_ERROR: longjmp_handler(JMP_ERROR); break;
        case JMP_NEW:   longjmp_handler(JMP_NEW); break;
        case JMP_QUIT:  longjmp_handler(JMP_QUIT); break;
        default:        longjmp_handler(JMP_UNEXPECTED); break;
    }

    while (1) {
        MMAbort = false;
        LocalIndex = 0;     // this should not be needed but it ensures that all
                            // space will be cleared
        ClearTempMemory();  // clear temp string space (might have been used by
                            // the prompt)
        CurrentLinePtr = NULL;  // do not use the line number in error reporting
        if (MMCharPos > 1) {
            console_puts("\r\n");  // prompt should be on a new line
        }
        //PrepareProgram(false); // This seems superflous so comment it out and see what breaks!
        // if (!ErrorInPrompt && FindSubFun("MM.PROMPT", 0) >= 0) {
        //     ErrorInPrompt = true;
        //     ExecuteProgram("MM.PROMPT\0");
        // } else {
        if (mmb_args.interactive) {
            console_puts("> ");  // print the prompt
        }
        // }
        // ErrorInPrompt = false;

        // This will clear all the interrupts including ON KEY
        // TODO: is this too drastic ? it means all interrupts will have been
        //       lost if the user tries to CONTINUE a program that has halted
        //       from CTRL-C or ERROR.
        interrupt_clear();

        memset(inpbuf, 0, STRINGSIZE);
        if (run_flag) {
            if (mmb_args.interactive) {
                console_puts(mmb_args.run_cmd);
                console_puts("\r\n");
            }
            strcpy(inpbuf, mmb_args.run_cmd);
            run_flag = false;
        } else {
            prompt_get_input();
            MmResult result = parse_transform_input_buffer(inpbuf);
            if (FAILED(result)) error_throw(result);
        }

        if (!*inpbuf) continue;  // ignore an empty line
        tokenise(true);          // turn into executable code
        if (*tknbuf == T_LINENBR)  // don't let someone use line numbers at the prompt
            tknbuf[0] = tknbuf[1] = tknbuf[2] = ' '; // convert the line number into spaces
        CurrentLinePtr = NULL;  // do not use the line number in error reporting

        ExecuteProgram(tknbuf);  // execute the line straight away
    }
}

void IntHandler(int signo) {
#if 0
    signal(SIGBREAK, IntHandler);
    signal(SIGINT, IntHandler);
#endif
    MMAbort = true;
}

void FlashWriteInit() {
    ProgMemory[0] = ProgMemory[1] = ProgMemory[2] = 0;
    console_set_title("MMBasic - Untitled");
    CurrentFile[0] = 0;
}

/**
 * Peforms "background" tasks:
 *  - pump for console input
 *  - pump for serial port input
 */
static void perform_background_tasks() {
    // TODO: consolidate with pumping the serial port connections ?
    console_pump_input();

    // Pump all the serial port connections for input.
    for (int i = 1; i <= MAXOPENFILES; ++i) {
        if (file_table[i].type == fet_serial) {
            serial_pump_input(i);
        }
    }
}

void CheckAbort(void) {
    if (!MMAbort) perform_background_tasks();

    if (MMAbort) {
        // g_key_select = 0;
        longjmp(mark, JMP_BREAK);  // jump back to the input prompt
    }
}

// get a keystroke.  Will wait forever for input
// if the char is a lf then replace it with a cr
// unless it was preceded by a cr and in that case throw away the char
// so console end of line is always cr
int MMgetchar(void) {
    static char prevchar = 0;
    int c;
    for (;;) {
        c = console_getc();
        if (c == -1) {
            if (!isatty(STDIN_FILENO)) {
                // In this case there will never be anything to read.
                if (MMCharPos > 1) console_puts("\r\n");
                console_puts("Error: STDIN exhausted\r\n");
                mmb_exit_code = 1;
                longjmp(mark, JMP_QUIT);
            }
            nanosleep(&ONE_MILLISECOND, NULL);
        // } else if (c == 3) {
        //     longjmp(mark, JMP_BREAK); // jump back to the input prompt if CTRL-C
        } else if (c == '\n' && prevchar == '\r') {
            prevchar = 0;
        } else {
            break;
        }
    }
    prevchar = c;
    return c == '\n' ? '\r' : c;
}

// get a line from the keyboard or a file handle
void MMgetline(int filenbr, char *p) {
    int c, nbrchars = 0;
    const char *tp;

    while (1) {
        CheckAbort();  // jump right out if CTRL-C

        if ((file_table[filenbr].type == fet_file) && file_eof(filenbr)) break; // End of file.
        c = file_getc(filenbr);

        // -1 - no character.
        //  0 - the null character which we ignore.
        if (c <= 0) continue;

        // if this is the console, check for a programmed function key and
        // insert the text
        if (filenbr == 0) {
            tp = NULL;
            if (c == F2) tp = "RUN";
            if (c == F3) tp = "LIST";
            if (c == F4) tp = "EDIT";
            if (c == F5) tp = "WEDIT";
            if (tp) {
                strcpy(p, tp);
                console_puts(tp);
                console_puts("\r\n");
                return;
            }
        }

        if (c == '\t') {  // expand tabs to spaces
            do {
                if (++nbrchars > MAXSTRLEN) ERROR_LINE_TOO_LONG;
                *p++ = ' ';
                if (filenbr == 0) console_putc(' ');
            } while (nbrchars % mmb_options.tab);
            continue;
        }

        if (c == '\b') {  // handle the backspace
            if (nbrchars) {
                if (filenbr == 0) console_puts("\b \b");
                nbrchars--;
                p--;
            }
            continue;
        }

        if (c == '\n') {  // what to do with a newline
            break;        // a newline terminates a line (for a file or serial)
        }

        if (c == '\r') {
            if (filenbr == 0) {
                console_puts("\r\n");
                break;  // on the console this means the end of the line
                        // - stop collecting
            } else {
                continue;  // for files and serial loop around looking for the
                           // following newline
            }
        }

        if (isprint(c) && (filenbr == 0)) {
            console_putc(c);  // The console requires that chars be echoed
        }

        if (++nbrchars > MAXSTRLEN) ERROR_LINE_TOO_LONG;  // stop collecting if maximum length

        // TODO: currently this function can return strings containing control
        //       characters, i.e. c < 32.
        //       Perhaps we should replace these with another character such as
        //       '?' or with a hex code <02> or <0x02>.
        //       The same might apply to c = 0 which we currently ignore.
        //       Possibly the behaviour could be controlled by an OPTION.

        *p++ = c;  // save our char
    }
    *p = 0;
}

// dump a memory area to the console
// for debugging
void dump(char *p, int nbr) {
    char buf1[80], buf2[80], *b1, *b2, *pt;
    b1 = buf1;
    b2 = buf2;
    console_puts(
        "   addr    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    "
        "0123456789ABCDEF\r\n");
#if defined(ENV64BIT)
    b1 += sprintf(b1, "%8lx: ", (uintptr_t) p);
#else
    b1 += sprintf(b1, "%8ix: ", (uintptr_t) p);
#endif
    for (pt = p; (uintptr_t)pt % 16 != 0; pt--) {
        b1 += sprintf(b1, "   ");
        b2 += sprintf(b2, " ");
    }
    while (nbr > 0) {
        b1 += sprintf(b1, "%02x ", *p);
        b2 += sprintf(b2, "%c", (*p >= ' ' && *p < 0x7f) ? *p : '.');
        p++;
        nbr--;
        if ((uintptr_t)p % 16 == 0) {
            console_puts(buf1);
            console_puts("   ");
            console_puts(buf2);
            b1 = buf1;
            b2 = buf2;
#if defined(ENV64BIT)
            b1 += sprintf(b1, "\r\n%8lx: ", (uintptr_t) p);
#else
            b1 += sprintf(b1, "\r\n%8ix: ", (uintptr_t) p);
#endif
        }
    }
    if (b2 != buf2) {
        console_puts(buf1);
        console_puts("   ");
        for (pt = p; (uintptr_t)pt % 16 != 0; pt++) {
            console_puts("   ");
        }
        console_puts(buf2);
    }
    console_puts("\r\n");
}

void dump_token_table(const struct s_tokentbl* tbl) {
    int i = 0;
    for (;;) {
        printf("%3d:  %-15s, %5d, %5d, 0x%8lX\n", i, tbl[i].name, tbl[i].type, tbl[i].precedence, (unsigned long int) tbl[i].fptr);
        if (*(tbl[i].name) == 0) break;
        i++;
    }
}

/**
 * Gets the address for a MMBasic interrupt.
 *
 * This will handle a line number, a label or a subroutine,
 * all areas of MMBasic that can generate an interrupt use this function.
 */
const char *GetIntAddress(const char *p) {
    int32_t i;
    if (isnamestart(*p)) {     // if it starts with a valid name char
        i = FindSubFun(p, 0);  // try to find a matching subroutine
        if (i == -1)
            return findlabel(p);  // if a subroutine was NOT found it must be a label
        else
            return funtbl[i].addr;  // if a subroutine was found, return the address
                                    // of the sub
    }

    return findline(getinteger(p), true);  // otherwise try for a line number
}
