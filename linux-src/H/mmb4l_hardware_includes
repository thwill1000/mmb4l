/***********************************************************************************************************************
MMBasic

Hardware_Includes.h

Defines the hardware aspects for the Windows/DOS version of MMBasic.

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

#if !defined(MMB4L_HARDWARE_INCLUDES_H)
#define MMB4L_HARDWARE_INCLUDES_H

#include <errno.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Configuration.h"
#include "common/console.h"
#include "common/file.h"
#include "common/interrupt.h"
#include "common/memory.h"
#include "common/options.h"
#include "commands/mmb4l_commands.h"
#include "functions/mmb4l_functions.h"
#include "operators/mmb4l_operators.h"

// Redefine the standard float routines used in MMBasic to their double versions
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

// Global variables used in MMBasic but must be maintained outside of the interpreter
extern int IgnorePIN;
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

// Aliases for identifiers using "legacy" names in MMBasic core:
#define Autorun          autorun
#define CONFIG_LOWER     kLower
#define CONFIG_UPPER     kUpper
#define DefaultType      mmb_options.default_type
#define Height           height
#define Listcase         list_case
#define Option           mmb_options
#define OptionBase       mmb_options.base
#define OptionExplicit   mmb_options.explicit_type
#define ProgFlashSize    prog_flash_size
#define Width            width

// Functions not used in MMB4L ... so make them go away
#define uSec(a)  {}
#define TestStackOverflow()  {}

#define ClearExternalIO()         // same

void CheckAbort(void);
int MMgetchar(void);

extern char *CFunctionFlash;
extern char *CFunctionLibrary;
extern char **FontTable;

// various debug macros
#if defined(DEBUGMODE)
    void dump(char *p, int nbr);   // defined in Main.c,  dump an area of memory in hex and ascii
    void DumpVarTbl(void);         // defined in MMBasic.c,  dump the variable table

    #define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); MMPrintString(s); MMPrintString("\r\n");}

    #define db(i) {IntToStr(inpbuf, i, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define db2(i1, i2) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define db3(i1, i2, i3) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i3, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}

    #define ds(s) {MMPrintString(s); MMPrintString("\r\n");}
    #define ds2(s1, s2) {MMPrintString(s1); MMPrintString(s2); MMPrintString("\r\n");}
    #define ds3(s1, s2, s3) {MMPrintString(s1); MMPrintString(s2); MMPrintString(s3); MMPrintString("\r\n");}

#endif

#endif
