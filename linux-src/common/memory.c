/*-*****************************************************************************

MMBasic for Linux (MMB4L)

memory.c

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

// This module manages all memory allocation for MMBasic.

#include <stdio.h>

#include "mmb4l.h"
#include "error.h"

// allocate static memory for programs, variables and the heap
// this is simple memory management because DOS has plenty of memory

#define MMAP_SIZE  ((HEAP_SIZE / PAGESIZE) / PAGESPERWORD) + 1

// memory for the program
char ProgMemory[PROG_FLASH_SIZE];

// memory for the memory map used in heap management
uint32_t mmap[MMAP_SIZE];

// memory for the actual heap
char MMHeap[HEAP_SIZE];

// memory for the variable table
struct s_vartbl DOS_vartbl[MAXVARS];

// arrays used to track temporary strings
char *StrTmp[MAXTEMPSTRINGS];           // used to track temporary string space on the heap
char StrTmpLocalIndex[MAXTEMPSTRINGS];  // used to track the LocalIndex for each temporary string space on the heap
int TempMemoryIsChanged = false;        // used to prevent unnecessary scanning of strtmp[]

// global functions
unsigned int MBitsGet(void *addr);
void MBitsSet(void *addr, int bits);
void *getheap(int size);

/***********************************************************************************************************************
 Public memory management functions
************************************************************************************************************************/

// every time a variable is added this must be called to verify that enough memory is free
void m_alloc(int type, int size) {
    if(type == M_VAR) {
        vartbl = (struct s_vartbl *)DOS_vartbl;
        if(size >= MAXVARS * sizeof(struct s_vartbl)) ERROR_OUT_OF_MEMORY;
    }
}

// get some memory from the heap
void *GetMemory(size_t msize) {
    TestStackOverflow();                                            // throw an error if we have overflowed the PIC32's stack
    return getheap(msize);                                          // allocate space
}

// Get a temporary buffer of any size, returns a pointer to the buffer
// The space only lasts for the length of the command or in the case of a sub/fun until it has exited.
// A pointer to the space is also saved in strtmp[] so that the memory can be automatically freed at the end of the command
// StrTmpLocalIndex[] is used to track the sub/fun nesting level at which it was created
void *GetTempMemory(int NbrBytes) {
    int i;
    for(i = 0; i < MAXTEMPSTRINGS; i++)
        if(StrTmp[i] == NULL) {
            StrTmpLocalIndex[i] = LocalIndex;
            StrTmp[i] = GetMemory(NbrBytes);
            TempMemoryIsChanged = true;
            return StrTmp[i];
        }
    ERROR_OUT_OF_MEMORY;
    return NULL;
}

// get a temporary string buffer
// this is used by many BASIC string functions.  The space only lasts for the length of the command.
void *GetTempStrMemory(void) {
    return GetTempMemory(STRINGSIZE);
}

// clear any temporary string spaces (these last for just the life of a command) and return the memory to the heap
// this will not clear memory allocated with a local index less than LocalIndex, sub/funs will increment LocalIndex
// and this prevents the automatic use of ClearTempMemory from clearing memory allocated before calling the sub/fun
void ClearTempMemory(void) {
    int i;
//dp("ClearTempMemory");
    for(i = 0; i < MAXTEMPSTRINGS; i++) {
        if(StrTmpLocalIndex[i] >= LocalIndex && StrTmp[i] != NULL) {
            FreeMemory(StrTmp[i]);
            StrTmp[i] = NULL;
        }
    }
    TempMemoryIsChanged = false;
}

void ClearSpecificTempMemory(void *addr) {
    int i;
//dpIGClearSpecificTempMemory(%p)", addr);
    for(i = 0; i < MAXTEMPSTRINGS; i++) {
        if(StrTmp[i] == addr) {
            FreeMemory(addr);
            StrTmp[i] = NULL;
            return;
        }
    }
}

void FreeMemory(void *addr) {
    int bits;
    // dp("FreeMemory(%p)", addr);
    do {
        if (addr < (void *) MMHeap || addr >= (void *) RAMEND) return;
        bits = MBitsGet(addr);
        MBitsSet(addr, 0);
        addr += PAGESIZE;
    } while (bits != (PUSED | PLAST));
}

void InitHeap(void) {
#if 0
    printf("MMHeap = %lX\n", MMHeap);
    printf("ProgMemory = %lX\n", ProgMemory);
    printf("HEAP_SIZE = %ld\n", HEAP_SIZE);
    printf("PAGESIZE = %ld\n", PAGESIZE);
    printf("PAGESPERWORD = %ld\n", PAGESPERWORD);
    printf("RAMEND = %lX\n", RAMEND);
    printf("MMAP SIZE = %d\n", MMAP_SIZE);
#endif
    for (int i = 0; i < MMAP_SIZE; i++) mmap[i] = 0;
    for (int i = 0; i < MAXTEMPSTRINGS; i++) StrTmp[i] = NULL;
    MBitsSet((char *) RAMEND, PUSED | PLAST);
}

/***********************************************************************************************************************
 Private memory management functions
************************************************************************************************************************/

unsigned int MBitsGet(void *addr) {
    unsigned int i, *p;
    // addr -= (int)MMHeap;
    uintptr_t addrx = (uintptr_t)addr - (uintptr_t)MMHeap;
    p = &mmap[(addrx / PAGESIZE) / PAGESPERWORD];  // point to the word in the memory map
    i = (((addrx / PAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS;  // get the position of the bits in the word
    return (*p >> i) & ((1 << PAGEBITS) - 1);
}

void MBitsSet(void *addr, int bits) {
    unsigned int i, *p;
    // addr -= (int)MMHeap;
    uintptr_t addrx = (uintptr_t)addr - (uintptr_t)MMHeap;
    p = &mmap[(addrx / PAGESIZE) / PAGESPERWORD];  // point to the word in the memory map
    i = (((addrx / PAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS;  // get the position of the bits in the word
    *p = (bits << i) | (*p & (~(((1 << PAGEBITS) - 1) << i)));
}

void *getheap(int size) {
    unsigned int j, n;
    char *addr;
    j = n = (size + PAGESIZE - 1)/PAGESIZE;                         // nbr of pages rounded up
    for(addr = (char *) RAMEND -  PAGESIZE; addr > MMHeap; addr -= PAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            if(--n == 0) {                                          // found a free slot
                j--;
                MBitsSet(addr + (j * PAGESIZE), PUSED | PLAST);     // show that this is used and the last in the chain of pages
                while(j--) MBitsSet(addr + (j * PAGESIZE), PUSED);  // set the other pages to show that they are used
                memset(addr, 0, size);                              // zero the memory
//dp("alloc = %p", addr);
                return (void *)addr;
            }
        } else
            n = j;                                                  // not enough space here so reset our count
    }
    // out of memory
    LocalIndex = 0;
    ClearTempMemory();                                              // hopefully this will give us enough to print the prompt
    ERROR_OUT_OF_MEMORY;
    return NULL;                                                    // keep the compiler happy
}

/** Counts the unused pages of heap and returns the result multiplied by PAGESIZE. */
int FreeSpaceOnHeap(void) {
    unsigned int nbr;
    char *addr;
    nbr = 0;
    for(addr = (char *) RAMEND -  PAGESIZE; addr > MMHeap; addr -= PAGESIZE)
        if(!(MBitsGet(addr) & PUSED)) nbr++;
    return nbr * PAGESIZE;
}

/** Counts the used pages of heap and returns the result multiplied by PAGESIZE. */
unsigned int UsedHeap(void) {
    unsigned int nbr;
    char *addr;
    nbr = 0;
    for(addr = (char *) RAMEND -  PAGESIZE; addr > MMHeap; addr -= PAGESIZE)
        if(MBitsGet(addr) & PUSED) nbr++;
    return nbr * PAGESIZE;
}

#ifdef __xDEBUG
void heapstats(char *m1) {
    int blk, siz, fre, hol;
    char *addr;
    blk = siz = fre = hol = 0;
    for(addr = (char *) RAMEND - PAGESIZE; addr >= MMHeap; addr -= PAGESIZE) {
        if(MBitsGet((void *)addr) & PUSED)
            {siz++; hol = fre;}
        else
            fre++;
        if(MBitsGet((void *)addr) & PLAST)
            blk++;
    }
    dp("%s allocations = %d (using %dKB)  holes = %dKB   free = %dKB", m1, blk, siz/4, hol/4, (fre - hol)/4);
}
#endif

// TODO: should validate the returned address is POKEable.
uintptr_t get_poke_addr(char *p) {
    return (uintptr_t) getinteger(p);

    // TODO
    // if ((i > (unsigned int)DOS_vartbl &&
    //      i < (unsigned int)DOS_vartbl + sizeof(struct s_vartbl) * MAXVARS) ||
    //     (i > (unsigned int)MMHeap && i < (unsigned int)MMHeap + HEAP_SIZE))
    //     return i;
    // else
    //     ERROR_INVALID_ADDRESS;
    // return 0;
}

uintptr_t get_peek_addr(char *p) {
    return (uintptr_t) getinteger(p);
}
