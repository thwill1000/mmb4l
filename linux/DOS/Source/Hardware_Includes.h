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



#if !defined(HARDWARE_INCLUDES_H)
    #define HARDWARE_INCLUDES_H

    #include <errno.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include "Configuration.h"
    #include "DOS_Includes.h"
    #include "DOS_Misc.h"
    #include "File_IO.h"
    #include "../../MMBasic/VarTable.h"
    #include "Memory.h"
    #include "Editor.h"
    #include "option.h"

    // various debug macros
    #if defined(DEBUGMODE)
        void dump(char *p, int nbr);                                // defined in Main.c,  dump an area of memory in hex and ascii
        void DumpVarTbl(void);                                      // defined in MMBasic.c,  dump the variable table

        #define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); MMPrintString(s); MMPrintString("\r\n");}

        #define db(i) {IntToStr(inpbuf, i, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
        #define db2(i1, i2) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
        #define db3(i1, i2, i3) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i3, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}

        #define ds(s) {MMPrintString(s); MMPrintString("\r\n");}
        #define ds2(s1, s2) {MMPrintString(s1); MMPrintString(s2); MMPrintString("\r\n");}
        #define ds3(s1, s2, s3) {MMPrintString(s1); MMPrintString(s2); MMPrintString(s3); MMPrintString("\r\n");}

    #endif


#endif


