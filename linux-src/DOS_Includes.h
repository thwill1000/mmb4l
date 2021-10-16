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

#define forever 1
#define true  1
#define false   0

// global variables used in MMBasic but must be maintained outside of the interpreter
extern char *StartEditPoint;
extern int StartEditChar;
extern char *InterruptReturn;
extern int WatchdogSet, IgnorePIN, InterruptUsed;
extern int MMCharPos;
extern int error_line;
extern char error_file[STRINGSIZE];

#define FONT_BUILTIN_NBR 0
#define FONT_TABLE_SIZE 0

#define getConsole  MMInkey

// functions not used in the DOS version... so make them go away
#define SaveOptions()    {}
#define ClearSavedVars()    {}
#define uSec(a)  {}
#define TestStackOverflow()  {}
#define LoadOptions()   {}

#define VCHARS  25                // nbr of lines in the DOS box (used in LIST)
#define ClearExternalIO()         // same

#define FILENAME_LENGTH 12
#define NBRERRMSG 17              // number of file error messages

// console related I/O
int MMInkey(void);
int MMgetchar(void);
char MMputchar(char c);

void CheckAbort(void);
extern char *CFunctionFlash, *CFunctionLibrary, **FontTable;

// misc
char *GetIntAddress(char *p);
int check_interrupt(void);
void FlashWriteInit(char *p, int nbr);
void dump(char *p, int nbr);

#define JMP_BREAK  1
#define JMP_END    2
#define JMP_ERROR  3
#define JMP_NEW    4
#define JMP_QUIT   5

#endif // #if !defined(MMB4L_DOS_INCLUDES_H)
