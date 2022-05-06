/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Version.h

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

#if !defined(VERSION_INCLUDED)
    #define VERSION_INCLUDED

    #define VERSION         "5.05.04"
    #define YEAR            "2011-2022"
    #define COPYRIGHT       "Copyright " YEAR " Geoff Graham\r\n"


    // Uncomment the following line if you want the "Lite" version of the MX170 code
    // #define LITE

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
        #define __mmb4l__
        #if defined(__ANDROID__)
            #define MM_ARCH  "Android aarch64"
            #define ENV64BIT
        #elif defined(__x86_64)
            #define MM_ARCH  "Linux x86_64"
            #define ENV64BIT
        #elif defined(__aarch64__)
            #define MM_ARCH  "Linux aarch64"
            #define ENV64BIT
        #elif defined(__arm__)
            #define MM_ARCH  "Linux armv6l"
            #define ENV32BIT
        #elif defined(__i686__)
            #define MM_ARCH  "Linux i686"
            #define ENV32BIT
        #else
            #error This architecture is not supported
        #endif
    #elif defined(__riscos__)
        #define __mmb4l__
        #define MM_ARCH "RISC OS"
        #define ENV32BIT
    #else
        #error This device is not supported
    #endif

    #if defined(__mmb4l__)
        #define MM_DEVICE  "MMB4L"
        #undef VERSION
        #define VERSION  "2022.01.00"
        #undef COPYRIGHT
        #define COPYRIGHT  "Copyright " YEAR " Geoff Graham\r\n" \
                           "Copyright 2016-2022 Peter Mather\r\n" \
                           "Copyright 2021-2022 Thomas Hugo Williams\r\n"
        #define MES_SIGNON  MM_ARCH " MMBasic Ver " VERSION "-a4\r\n"
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

#endif
