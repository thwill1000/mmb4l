/***********************************************************************************************************************
Configuration.h

Include file that contains the configuration details for DOS running MMBasic.

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

#define MIPS16                                      // don't use mips16 attribute on functions

#define MMFLOAT double                              // precision of all floating point operations
#define STR_SIG_DIGITS 9                            // number of significant digits to use when converting MMFLOAT to a string
#define FLOAT_ROUNDING_LIMIT 0x7fffff               // used to limit rounding for large numbers in FloatToInt64()

// these 3 represent most of the RAM used
#define PROG_FLASH_SIZE   (512 * 1024)              // size of the program memory (in bytes)
#define HEAP_SIZE         (512 * 1024)              // size of the heap memory (in bytes)
#define MAXVARS             500                     // 8 + MAXVARLEN + MAXDIM * 2  (ie, 56 bytes) - these do not incl array members

// more static memory allocations (less important)
#define MAXFORLOOPS         50                      // each entry uses 17 bytes
#define MAXDOLOOPS          50                      // each entry uses 12 bytes
#define MAXGOSUB            1000                    // each entry uses 4 bytes
#define MAX_MULTILINE_IF    20                      // each entry uses 8 bytes
#define MAXTEMPSTRINGS      256                     // each entry takes up 4 bytes
#define MAXSUBFUN           512                     // each entry takes up 4 bytes

// operating characteristics
#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXSTRLEN           255                     // maximum length of a string
#define STRINGSIZE          256                     // must be 1 more than MAXSTRLEN.  2 of these buffers are staticaly created
#define MAXOPENFILES        10                      // maximum number of open files
#define MAXDIM              8                       // maximum nbr of dimensions to an array
#define TRACE_BUFF_SIZE     128                     // capacity of the trace buffer
#define MAXERRMSG           32                      // max error msg size (MM.ErrMsg$ is truncated to this)


// define the maximum number of arguments to PRINT, INPUT, WRITE, ON, DIM, ERASE, DATA and READ
// each entry uses zero bytes.  The number is limited by the length of a command line
#define MAX_ARG_COUNT       50

#define BREAK_KEY            3

