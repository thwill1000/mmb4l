/***********************************************************************************************************************
MMBasic

memory.h

Include file that contains the globals and defines for memory allocation for MMBasic running in the DOS environmrny.

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
uintptr_t get_poke_addr(char *p);
uintptr_t get_peek_addr(char *p);

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
