/***********************************************************************************************************************
MMBasic

Main.c

The startup code and essential functions for the DOS version of MMBasic.

Copyright 2011 - 2020 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific
individuals or organisations under the following provisions:

- This file, or any files that comprise the MMBasic source (modified or not),
may not be distributed or copied to any other person or organisation without
written permission.

- Object files (.o and .hex files) generated using this file (modified or not)
may not be distributed or copied to any other person or organisation without
written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.

************************************************************************************************************************/

#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "common/console.h"
#include "common/file.h"
#include "common/global_aliases.h"
#include "common/interrupt.h"
#include "common/utility.h"
#include "common/version.h"

extern int g_key_select;

extern char error_buffer[STRINGSIZE];
extern size_t error_buffer_pos;

// global variables used in MMBasic but must be maintained outside of the
// interpreter
int ListCnt;
int MMCharPos;
struct timespec g_timer;
int ExitMMBasicFlag = false;
volatile int MMAbort = false;
char *InterruptReturn = NULL;
struct option_s Option;
int WatchdogSet, IgnorePIN, InterruptUsed;
char *OnKeyGOSUB;
char *CFunctionFlash, *CFunctionLibrary, **FontTable;

int ErrorInPrompt = false;

char g_break_key = BREAK_KEY;

char *GetEnvPath(char *env);
void IntHandler(int signo);
int LoadFile(char *prog);
void dump_token_table(const struct s_tokentbl* tbl);

void InsertLastcmd(char *s);  // common/prompt.c
void prompt_get_input(void); // common/prompt.c

void init_options() {
    Option.ProgFlashSize = PROG_FLASH_SIZE;
    Option.Tab = 4;
    Option.console = SERIAL;
    Option.resolution = CHARACTER;
}

int main(int argc, char *argv[]) {
    static int PromptError = false;
    int RunCommandLineProgram = false;
    char *p;

    // get things setup to act like the Micromite version
    vartbl = DOS_vartbl;
    ProgMemory[0] = ProgMemory[1] = ProgMemory[2] = 0;
    init_options();
    Option.ProgFlashSize = PROG_FLASH_SIZE;
    Option.Tab = 4;
    // cmdCFUN = cmdCSUB = cmdIRET = 0xff;
    cmdCFUN = cmdCSUB = 0xff;

    InitHeap();  // init memory allocation

    console_enable_raw_mode();
    atexit(console_disable_raw_mode);
    console_set_title("MMBasic - Untitled");
    console_reset();
    console_clear();
    console_show_cursor(1);

    MMPrintString(MES_SIGNON);
    MMPrintString(COPYRIGHT);
    // MMPrintString("Copyright 2016-2021 Peter Mather\r\n");
    MMPrintString("Copyright 2021 Thomas Hugo Williams\r\n");
    MMPrintString("\r\n");

    OptionErrorSkip = 0;
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

    clock_gettime(CLOCK_REALTIME, &g_timer);
    srand(0);             // seed the random generator with zero

    // if there is something on the command line try to load it as a program, if
    // that fails try AUTORUN.BAS
    if (argc > 1) RunCommandLineProgram = LoadFile(argv[1]);
    if (!RunCommandLineProgram) RunCommandLineProgram = LoadFile("AUTORUN.BAS");
    if (!RunCommandLineProgram)
        RunCommandLineProgram = LoadFile("C:\\AUTORUN.BAS");

    if (getenv("MMDIR") != NULL) chdir(GetEnvPath("MMDIR"));

    if (setjmp(mark) != 0) {
        // we got here via a long jump which means an error or CTRL-C or the
        // program wants to exit to the command prompt

        console_show_cursor(1);
        console_reset();

        if (ExitMMBasicFlag) {
            return 0;  // program has executed an ExitMMBasic command
        }

        if (error_buffer_pos) {
            MMPrintString(error_buffer);
            error_buffer_pos = 0;
            memset(error_buffer, 0, STRINGSIZE);
        }
        MMPrintString("\r\n");

        ContinuePoint = nextstmt;       // in case the user wants to use the continue command
        *tknbuf = 0;                    // we do not want to run whatever is in the token buffer
        RunCommandLineProgram = false;  // nor the program on the command line
        memset(inpbuf, 0, STRINGSIZE);
    }

    if (RunCommandLineProgram) {
        RunCommandLineProgram = false;
        ClearRuntime();
        PrepareProgram(true);
        ExecuteProgram(ProgMemory);  // if AUTORUN.BAS or something is on the
                                     // command line, run it
    }

    while (1) {
        MMAbort = false;
        LocalIndex = 0;     // this should not be needed but it ensures that all
                            // space will be cleared
        ClearTempMemory();  // clear temp string space (might have been used by
                            // the prompt)
        CurrentLinePtr = NULL;  // do not use the line number in error reporting
        if (MMCharPos > 1) {
            MMPrintString("\r\n");  // prompt should be on a new line
        }
        PrepareProgram(false);
        if (!ErrorInPrompt && FindSubFun("MM.PROMPT", 0) >= 0) {
            ErrorInPrompt = true;
            ExecuteProgram("MM.PROMPT\0");
        } else {
            MMPrintString("> ");  // print the prompt
        }
        ErrorInPrompt = false;

        prompt_get_input();

        if (!*inpbuf) continue;  // ignore an empty line
        tokenise(true);          // turn into executable code
        if (*tknbuf == T_LINENBR)  // don't let someone use line numbers at the prompt
            tknbuf[0] = tknbuf[1] = tknbuf[2] = ' '; // convert the line number into spaces
        CurrentLinePtr = NULL;  // do not use the line number in error reporting
        //printf("tknbuf = %s\n", tknbuf);

        ExecuteProgram(tknbuf);  // execute the line straight away

        memset(inpbuf, 0, STRINGSIZE);
    }
}

void IntHandler(int signo) {
#if 0
    signal(SIGBREAK, IntHandler);
    signal(SIGINT, IntHandler);
#endif
    MMAbort = true;
}

int LoadFile(char *prog) {
#if 0
    FILE *f;
    char buf[STRINGSIZE];
    f = fopen(prog, "rb");
    if (f != NULL) {
        fclose(f);
        buf[0] = '"';
        strcpy(&buf[1], prog);
        strcat(buf, "\"");
        FileLoadProgram(buf);
        if (*ProgMemory == T_NEWLINE ||
            *ProgMemory == T_LINENBR) {  // is there a program to run?
            return true;
        }
    }
#endif
    return false;
}

char *GetEnvPath(char *env) {
    char *p;
    p = getenv(env);
    if (p == NULL) return NULL;
    if (*p == '\"') {
        p++;
        if (p[strlen(p) - 1] == '\"') p[strlen(p) - 1] = 0;
    }
    return p;
}

void FlashWriteInit(char *p, int nbr) {
    ProgMemory[0] = ProgMemory[1] = ProgMemory[2] = 0;
    console_set_title("MMBasic - Untitled");
    CurrentFile[0] = 0;
}

// get a character from the console
// returns -1 if nothing there
int MMInkey(void) {
    //CheckAbort();
    // return kbhit() ? DOSgetch() : -1;
    //return DOSgetch();

    // CheckAbort();
    //printf("Going in\n");
    int ch = console_getc();
    CheckAbort();
    // if (ch == 3) longjmp(mark, 1); // jump back to the input prompt if CTRL-C
    return ch;
}

void CheckAbort(void) {
    console_buffer_input();

    if (MMAbort) {
        g_key_select = 0;
        longjmp(mark, 1);  // jump back to the input prompt
    }
}

// get a keystroke.  Will wait forever for input
// if the char is a lf then replace it with a cr
// unless it was preceded by a cr and in that case throw away the char
// so console end of line is always cr
int MMgetchar(void) {
//     int c;
//     static char prevchar = 0;
//     DWORD mode;
//     GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);

// loopback:
//     SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
//                    mode & ~ENABLE_PROCESSED_INPUT);
//     c = DOSgetch();
//     SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
//     if (c == 3) longjmp(mark, 1);  // jump back to the input prompt if CTRL-C
//     if (c == '\n' && prevchar == '\r') {
//         prevchar = 0;
//         goto loopback;
//     }
//     prevchar = c;
//     if (c == '\n') c = '\r';
//     return c;

    static char prevchar = 0;
    int c;
    for (;;) {
        c = console_getc();
        //printf("\n0x%X\n", c);
        if (c == -1) {
            nanosleep(&ONE_MILLISECOND, NULL);
        } else if (c == 3) {
            longjmp(mark, 1); // jump back to the input prompt if CTRL-C
        } else if (c == '\n' && prevchar == '\r') {
            prevchar = 0;
        } else {
            break;
        }
    }
    prevchar = c;
    return c == '\n' ? '\r' : c;
}

// put a character out to the operating system
char MMputchar(char c) {
    // putch(c);
    putc(c, stdout);
    fflush(stdout);
    if (isprint(c)) MMCharPos++;
    if (c == '\r' || c == '\n') {
        MMCharPos = 1;
        ListCnt++;
    }
    return c;
}

// get a line from the keyboard or a file handle
void MMgetline(int filenbr, char *p) {
    int c, nbrchars = 0;
    char *tp;

    while (1) {
        CheckAbort();                // jump right out if CTRL-C
        if (MMfeof(filenbr)) break;  // end of file - stop collecting
        c = MMfgetc(filenbr);

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
                MMPrintString(tp);
                MMPrintString("\r\n");
                return;
            }
        }

        if (c == '\t') {  // expand tabs to spaces
            do {
                if (++nbrchars > MAXSTRLEN) error("Line is too long");
                *p++ = ' ';
                if (filenbr == 0) MMputchar(' ');
            } while (nbrchars % Option.Tab);
            continue;
        }

        if (c == '\b') {  // handle the backspace
            if (nbrchars) {
                if (filenbr == 0) MMPrintString("\b \b");
                nbrchars--;
                p--;
            }
            continue;
        }

        if (c == '\n') {  // what to do with a newline
            break;        // a newline terminates a line (for a file)
        }

        if (c == '\r') {
            if (filenbr == 0) {
                MMPrintString("\r\n");
                break;  // on the console this meand the end of the line - stop
                        // collecting
            } else
                continue;  // for files loop around looking for the following
                           // newline
        }

        if (isprint(c)) {
            if (filenbr == 0)
                MMputchar(c);  // The console requires that chars be echoed
        }

        if (++nbrchars > MAXSTRLEN)
            error("Line is too long");  // stop collecting if maximum length
        *p++ = c;                       // save our char
    }
    *p = 0;

    //printf("%s", p);
}

/** Checks if an interrupt has occurred. */
int check_interrupt(void) {
    return interrupt_check();
}

// dump a memory area to the console
// for debugging
void dump(char *p, int nbr) {
    char buf1[80], buf2[80], buf3[80], *b1, *b2, *pt;
    b1 = buf1;
    b2 = buf2;
    MMPrintString(
        "   addr    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    "
        "0123456789ABCDEF\r\n");
    b1 += sprintf(b1, "%8lx: ", (uintptr_t)p);
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
            MMPrintString(buf1);
            MMPrintString("   ");
            MMPrintString(buf2);
            b1 = buf1;
            b2 = buf2;
            b1 += sprintf(b1, "\r\n%8lx: ", (uintptr_t)p);
        }
    }
    if (b2 != buf2) {
        MMPrintString(buf1);
        MMPrintString("   ");
        for (pt = p; (uintptr_t)pt % 16 != 0; pt++) {
            MMPrintString("   ");
        }
        MMPrintString(buf2);
    }
    MMPrintString("\r\n");
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
char *GetIntAddress(char *p) {
    int32_t i;
    if (isnamestart(*p)) {     // if it starts with a valid name char
        i = FindSubFun(p, 0);  // try to find a matching subroutine
        if (i == -1)
            return findlabel(p);  // if a subroutine was NOT found it must be a label
        else
            return subfun[i];  // if a subroutine was found, return the address
                               // of the sub
    }

    return findline(getinteger(p), true);  // otherwise try for a line number
}
