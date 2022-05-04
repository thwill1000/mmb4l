/*-*****************************************************************************

MMBasic for Linux (MMB4L)

memory.h

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

#if !defined(MMB4L_MEMORY_H)
#define MMB4L_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "../Configuration.h"
#include "../../MMBasic/VarTable.h"

extern char *StrTmp[];                                      // used to track temporary string space on the heap
extern int TempMemoryTop;                                   // this is the last index used for allocating temp memory
extern int TempMemoryIsChanged;                             // used to prevent unnecessary scanning of strtmp[]

typedef enum _M_Req {M_PROG, M_VAR} M_Req;

void m_alloc(int type, int size);
void *GetMemory(size_t msize);
void *GetTempMemory(int NbrBytes);
void *GetTempStrMemory(void);
void ClearTempMemory(void);
void ClearSpecificTempMemory(void *addr);
void FreeMemory(void *addr);
void InitHeap(void);
unsigned int UsedHeap(void);
int FreeSpaceOnHeap(void);
uintptr_t get_poke_addr(const char *p);
uintptr_t get_peek_addr(const char *p);

// RAM parameters
// ==============

// The virtual address that MMBasic can start using memory.
// #define RAMBase MMHeap

// The virtual address that marks the end of the RAM allocated to MMBasic.  This must be rounded up to PAGESIZE.
// This determines maximum amount of RAM that MMBasic can use for program, variables and the heap.
// MMBasic uses just over 5K of RAM for static variables and needs at least 4K for the stack (6K preferably).
// So, using a chip with 32KB, RAMALLOC could be set to RAMBASE + (22 * 1024).
// However, the PIC32 C compiler provides us with a convenient marker (see diagram above).
#define RAMEND   (MMHeap + HEAP_SIZE)

// The total amount of memory (in KB) that MMBasic might use.  This must be a constant (ie, not defined in terms of _splim)
// Used only to declare a static array to track memory use.  It does not consume much RAM so we set it to the largest possible size for the PIC32

// other (minor) memory management parameters
#define PAGESIZE        256                                         // the allocation granuality
#define PAGEBITS        2                                           // nbr of status bits per page of allocated memory, must be a power of 2

#define PUSED           1 //0b01                                    // flag that indicates that the page is in use
#define PLAST           2 //0b10                                    // flag to show that this is the last page in a single allocation

#define PAGESPERWORD    ((sizeof(uint32_t) * 8)/PAGEBITS)
// #define MRoundUp(a)     (((a) + (PAGESIZE - 1)) & (~(PAGESIZE - 1)))// round up to the nearest page size

extern struct s_vartbl *vartbl;
extern struct s_vartbl DOS_vartbl[MAXVARS];
extern char ProgMemory[];

#endif
