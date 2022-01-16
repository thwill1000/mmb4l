/***********************************************************************************************************************
MMBasic

DOS_Includes.h

General definitions for DOS MMBasic.

Copyright 2011 - 2020 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/

#if !defined(MMB4L_DOS_INCLUDES_H)
#define MMB4L_DOS_INCLUDES_H

#include <stdint.h>

// redefine the standard float routines used in MMBasic to their double versions
//#if MMFLOAT == double
    #define powf pow
    #define log10f log10
    #define floorf floor
    #define fabsf fabs
    #define atanf atan
    #define cosf cos
    #define expf exp
    #define logf log
    #define sinf sin
    #define sqrtf sqrt
    #define tanf tan
//#endif

// global variables used in MMBasic but must be maintained outside of the interpreter
extern char *StartEditPoint;
extern int StartEditChar;
extern int WatchdogSet, IgnorePIN;
extern int MMCharPos;
extern int error_line;
extern char error_file[STRINGSIZE];

#define FONT_BUILTIN_NBR 0
#define FONT_TABLE_SIZE 0

// Aliases for functions using "legacy" names in MMBasic core:
#define check_interrupt  interrupt_check
#define CloseAllFiles    file_close_all
#define getConsole       console_getc
#define MMfeof           file_eof
#define MMfgetc          file_getc
#define MMfputc          file_putc
#define MMputchar        console_putc

// functions not used in the DOS version... so make them go away
#define SaveOptions()    {}
#define ClearSavedVars()    {}
#define uSec(a)  {}
#define TestStackOverflow()  {}
#define LoadOptions()   {}

#define ClearExternalIO()         // same

// console related I/O
int MMgetchar(void);
char MMputchar(char c);

void CheckAbort(void);
extern char *CFunctionFlash, *CFunctionLibrary, **FontTable;

// misc
char *GetIntAddress(char *p);
void FlashWriteInit(char *p, int nbr);
int codepage_set(const char *page_name);
void dump(char *p, int nbr);
void interrupt_clear(void);

#define JMP_BREAK  1
#define JMP_END    2
#define JMP_ERROR  3
#define JMP_NEW    4
#define JMP_QUIT   5
#define JMP_UNEXPECTED  999

// Global variables specific to MMB4L
extern uint8_t mmb_exit_code;

#endif // #if !defined(MMB4L_DOS_INCLUDES_H)
