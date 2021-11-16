/***********************************************************************************************************************
Configuration.h

Include file that contains the configuration details for DOS running MMBasic.
  
Copyright 2011 - 2013 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following 
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/
#define MMFLOATPRECISION
// these 3 represent most of the RAM used
#define PROG_FLASH_SIZE		(1024 * 1024)				// size of the program memory (in bytes)
#define HEAP_SIZE		(4096 * 1024)				// size of the heap memory (in bytes)
#define EDIT_BUFFER_SIZE        (1024 * 1024)
#define MAXVARS             1000                     // 8 + MAXVARLEN + MAXDIM * 2  (ie, 56 bytes) - these do not incl array members
#define SOCKETBUFSIZE 16384
#define SOCKETBUFOUTSIZE 63335

// more static memory allocations (less important)
#define MAXFORLOOPS         10                      // each entry uses 17 bytes
#define MAXDOLOOPS          10                      // each entry uses 12 bytes
#define MAXGOSUB            50                     // each entry uses 4 bytes
#define MAX_MULTILINE_IF    10                      // each entry uses 8 bytes
#define MAXTEMPSTRINGS      1024                      // each entry takes up 4 bytes
#define MAXSUBFUN           200                      // each entry takes up 4 bytes
#define MAXMODULES          16                      // maximum nbr of modules that can be loaded simultaneously. each entry takes up 4 bytes

// operating characteristics
#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXSTRLEN           255                     // maximum length of a string
#define STRINGSIZE          256                     // must be 1 more than MAXSTRLEN.  2 of these buffers are staticaly created
#define MAXOPENFILES        10                      // maximum number of open files
#define MAXDIM              8                       // maximum nbr of dimensions to an array
#define NBRSETTICKS         4                       // the number of SETTICK interrupts available
#define MAXBLITBUF          51                       // the maximum number of BLIT buffers
#define MAXLAYER            10                      // maximum number of sprite layers

#define RoundUptoInt(a)     (((a) + (32 - 1)) & (~(32 - 1)))// round up to the nearest whole integer
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


// define the maximum number of arguments to PRINT, INPUT, WRITE, ON, DIM, ERASE, DATA and READ
// each entry uses zero bytes.  The number is limited by the length of a command line
#define MAX_ARG_COUNT       50

#define MAXPROMPTLEN        49                      // max length of a prompt incl the terminating null

// size of the console terminal emulator's screen
#define SCREENWIDTH     80
#define SCREENHEIGHT    24                          // this is the default and it can be changed using the OPTION command

#define BREAK_KEY           3                       // the default value (CTRL-C) for the break key.  Reset at the command prompt.
#define MMFLOAT  double
#define STR_SIG_DIGITS 9                            // number of significant digits to use when converting MMFLOAT to a string
#define MIPS16
#define ClearSavedVars()    {}
#define TestStackOverflow()  {}
#if MMFLOAT == double
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
#endif