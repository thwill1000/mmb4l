/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Configuration.h

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

#include <stdint.h>

#define MIPS16                                      // don't use mips16 attribute on functions

#define MMFLOAT double                              // precision of all floating point operations
#define MMINTEGER int64_t
#define UNSIGNED_MMINTEGER uint64_t
#define STR_SIG_DIGITS 9                            // number of significant digits to use when converting MMFLOAT to a string
#define FLOAT_ROUNDING_LIMIT 0x7fffff               // used to limit rounding for large numbers in FloatToInt64()

// these 3 represent most of the RAM used
#define PROG_FLASH_SIZE     (512 * 1024)            // size of the program memory (in bytes)
// #define HEAP_SIZE        (512 * 1024)            // size of the heap memory (in bytes)
#define HEAP_SIZE           (512 * 1024 * 2)        // size of the heap memory (in bytes)
#define MAXVARS             1024                    // 8 + MAXVARLEN + MAXDIM * 2  (ie, 56 bytes) - these do not incl array members
#define VARS_HASHMAP_SIZE   1371                    // Size of the variables hash table
                                                    //  - first prime number at least 1/3 greater than MAXVARS.

// more static memory allocations (less important)
#define MAXFORLOOPS         50                      // each entry uses 17 bytes
#define MAXDOLOOPS          50                      // each entry uses 12 bytes
#define MAXGOSUB            1000                    // each entry uses 4 bytes
#define MAX_MULTILINE_IF    20                      // each entry uses 8 bytes
#define MAXTEMPSTRINGS      256                     // each entry takes up 4 bytes
#define NBRSETTICKS         4                       // the number of SETTICK interrupts available
#define MAXSUBFUN           512                     // each entry takes up 4 bytes
#define FUN_HASHMAP_SIZE    683                     // Size of the functions hash table
                                                    //  - first prime number at least 1/3 greater than MAXSUBFUN.

// operating characteristics
#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXSTRLEN           255                     // maximum length of a string
#define STRINGSIZE          256                     // must be 1 more than MAXSTRLEN.  2 of these buffers are staticaly created
#define MAXOPENFILES        10                      // maximum number of open files
#define MAXDIM              8                       // maximum nbr of dimensions to an array
#define TRACE_BUFF_SIZE     128                     // capacity of the trace buffer
//#define MAXERRMSG           32                      // max error msg size (MM.ErrMsg$ is truncated to this)
#define MAXERRMSG           256                     // max error msg size (MM.ErrMsg$ is truncated to this)


// define the maximum number of arguments to PRINT, INPUT, WRITE, ON, DIM, ERASE, DATA and READ
// each entry uses zero bytes.  The number is limited by the length of a command line
#define MAX_ARG_COUNT       50

#define BREAK_KEY            3

