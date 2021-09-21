/***********************************************************************************************************************
MMBasic

Version.h

Copyright 2011 - 2021 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 ************************************************************************************************************************/



#if !defined(VERSION_INCLUDED)
    #define VERSION_INCLUDED

    // #define VERSION         "5.05.04"
    #define VERSION         "2021.0.0"
    #define YEAR            "2011-2021"
    #define COPYRIGHT       "Copyright " YEAR " Geoff Graham\r\n"


    // Uncomment the following line if you want the "Lite" version of the MX170 code
    // #define LITE


    #include <stdlib.h>
    #include <setjmp.h>
    #include <string.h>
    #include <ctype.h>
    #include <limits.h>
    #include <math.h>
    #include <float.h>

    // define the device/platform, include the appropriate hardware defines and set the startup message
    #if defined(__32MX170F256B__) || defined(__32MX270F256B__) || defined(__32MX170F256D__) || defined(__32MX270F256D__)
        #define MX170
        #define MICROMITE
        #if !defined(LITE)
            #define MES_SIGNON  "Micromite MKII MMBasic Ver " VERSION "\r\n"
        #else
            #define MES_SIGNON  "Micromite Lite MMBasic Ver " VERSION "\r\n"
        #endif
    #elif defined(__32MX470F512L__) || defined(__32MX470F512H__)
        #define MX470
        #define MICROMITE
        #define MES_SIGNON  "Micromite Plus MMBasic Ver " VERSION "\r\n"
    #elif defined(__32MX695F512L__) || defined(__32MX795F512L__) || defined(__32MX695F512H__) || defined(__32MX795F512H__)
        #define MAXIMITE
        #define COLOUR
        #undef VERSION
        #define VERSION         "5.05.00 Beta 2"                    // temporary while porting the Maximite code
        #define MES_SIGNON  "Maximite MMBasic Ver " VERSION "\r\n"
//    #elif defined(__386__)
    #elif defined(DOS)
        #undef DOS
        #define DOS
        #define __386__
        #define MES_SIGNON  "Windows MMBasic Ver " VERSION "\r\n"
    #elif defined(__linux__)
        #if defined(__ANDROID__)
            #define MM_DEVICE  "Android"
        #else
            #define MM_DEVICE  "Linux"
        #endif
        #if defined(__x86_64)
            #define MM_ARCH  "x86_64"
            #define ENV64BIT
        #elif defined(__aarch64__)
            #define MM_ARCH  "aarch64"
            #define ENV64BIT
        #elif defined(__arm__)
            #define MM_ARCH  "arm"
            #define ENV32BIT
        #else
            #error This architecture is not supported
        #endif
    #else
        #error This device is not supported
    #endif

    #if defined(__linux__) // Which includes __ANDROID__
        #define MES_SIGNON  MM_DEVICE " " MM_ARCH " MMBasic Ver " VERSION "-a1\r\n"
    #endif

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // debugging options
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // #define DEBUGMODE                     // enable debugging macros (with reduced program memory for the Micromite)

    // MX170 Backpack with ILI9341
    // #define TEST_CONFIG "OPTION LCDPANEL ILI9341, LANDSCAPE, 2, 23, 6 : OPTION TOUCH 7, 15 : GUI CALIBRATE 0, 252, 306, 932, 730 : OPTION SAVE : ? \"ILI9341 ON\""

    // Explore 100 with SSD1963_5
    // #define TEST_CONFIG "OPTION LCDPANEL SSD1963_5, LANDSCAPE, 48, 6 : OPTION TOUCH 1, 40, 39 : OPTION SDCARD 47 : GUI CALIBRATE 1, 108, 3849, 2050, -1342 : OPTION SAVE : ? \"SSD1963_5 ON\""

    // Explore 100 with SSD1963_5 and LCD CONSOLE/KEYBOARD
    // #define TEST_CONFIG "OPTION LCDPANEL SSD1963_5, LANDSCAPE, 48, 6 : OPTION TOUCH 1, 40, 39 : OPTION SDCARD 47 : GUI CALIBRATE 1, 108, 3849, 2050, -1342 : OPTION LCDPANEL CONSOLE : OPTION KEYBOARD US : OPTION SAVE : ? \"SSD1963_5 & kbd ON\""

    // Explore 100 with SSD1963_4
    // #define TEST_CONFIG "OPTION LCDPANEL SSD1963_4, LANDSCAPE, 48, 6 : OPTION TOUCH 1, 40, 39 : OPTION SDCARD 47 : GUI CALIBRATE 1, 3971, 3774, -1253, -772 : OPTION SAVE : ? \"SSD1963_4 ON\""

    // Peter's MM+ 64-pin board with ILI9341
    // #define TEST_CONFIG "OPTION LCDPANEL ILI9341, LANDSCAPE, 26, 28, 24 : OPTION TOUCH 15, 33 : OPTION SDCARD 52 :  : OPTION SAVE : ? \"ILI9341 ON\""

    // Peter's MM+ 64-pin board with SSD1963_4
    // #define TEST_CONFIG "OPTION LCDPANEL SSD1963_4, LANDSCAPE : OPTION TOUCH 51, 33, 50 : OPTION SDCARD 52 : GUI CALIBRATE 1, 3971, 3774, -1253, -772 : OPTION SAVE : ? \"SSD1963_4 ON\""

    // Rob's CGMICROBOARD2 (64-pin MM+ with 2.2" LCD)
    // #define TEST_CONFIG "OPTION LCDPANEL ILI9341, L, 21, 22, 23 : OPTION TOUCH 18, 14 :OPTION SDCARD 49, 30 : GUI CALIBRATE 0, 260, 446, 938, 733 : OPTION SAVE : ? \"ILI9341 ON\""


    #if defined(__DEBUG) && !defined(DEBUGMODE)
        #define DEBUGMODE
    #endif

    #if !defined(MX170) && defined(LITE)
        #error "Lite version not valid"
    #endif

    // define the device/platform, include the appropriate hardware defines
    #if defined(MICROMITE)
        #include "Micromite/Hardware_Includes.h"
    #elif defined(MAXIMITE)
        #include "Maximite/Hardware_Includes.h"
        #include "Maximite/Source/Flash.h"
    #elif defined(__linux__)
        #include "linux-src/Hardware_Includes.h"
    #elif defined(DOS)
        #include "DOS/Source/Hardware_Includes.h"
    #endif

    #include "MMBasic/MMBasic_Includes.h"

#endif

