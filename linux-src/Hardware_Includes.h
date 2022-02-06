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

    #include "Configuration.h"
    #include "DOS_Includes.h"
    #include "../MMBasic/VarTable.h"
    #include "common/console.h"
    #include "common/draw.h"
    #include "common/file.h"
    #include "common/interrupt.h"
    #include "common/memory.h"
    #include "common/option.h"
    #include "commands/mmb4l_commands.h"
    #include "functions/mmb4l_functions.h"
    #include "operators/mmb4l_operators.h"

    #define HDMI 0
    #define SSD1963_4       1
    #define SSD1963_5       2
    #define SSD1963_5A      3
    #define SSD1963_7       4
    #define SSD1963_7A      5
    #define SSD1963_8       6
    #define SSD1963_4P      7
    #define SSD_PANEL       7    // anything less than or equal to SSD_PANEL is handled by the SSD driver, anything more by the SPI driver

    #define ILI9341         8
    #define ILI9163         9
    #define ST7735          10
    #define ST7789          11
    #define ILI9481         12
    #define ILI9486         13
    #define SPI_PANEL       13    // anything less than or equal to SSD_PANEL is handled by the SSD driver, anything more by the SPI driver
    #define USER            14
    #define VGA             15
    #define LANDSCAPE       1
    #define PORTRAIT        2
    #define RLANDSCAPE      3
    #define RPORTRAIT       4
    #define DISPLAY_LANDSCAPE   (Option.DISPLAY_ORIENTATION & 1)

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


