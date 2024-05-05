/*-*****************************************************************************

MMBasic for Linux (MMB4L)

interrupt.h

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

#if !defined(MMB4L_INTERRUPT_H)
#define MMB4L_INTERRUPT_H

#include "graphics.h"
#include "../Configuration.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  kInterruptGamepad1,
  kInterruptGamepad2,
  kInterruptGamepad3,
  kInterruptGamepad4,
  kInterruptLast
} InterruptType;

typedef struct {
  const char *fn;
  bool fired;
} Interrupt;

/** Initialises interrupts. */
void interrupt_init(void);

/** Clears all interrupts. */
void interrupt_clear(void);

/**
 * Checks if an interrupt has occurred and if so,
 * sets the next command to the interrupt routine.
 *
 * @return  'true' if interrupt detected, otherwise 'false'.
 */
bool interrupt_check(void);

/** Are we currently executing an interrupt handler ? */
bool interrupt_running(void);

/** Handles return from an interrupt. */
void interrupt_return(void);

/** Disables the 'ON KEY' interrupt. */
void interrupt_disable_any_key(void);

/** Disables the 'ON KEY ASCIIcode' interrupt. */
void interrupt_disable_specific_key(void);

/** Disables the specified 'SETTICK' interrupt. */
void interrupt_disable_tick(int irq);

/** Enables the 'ON KEY' interrupt. */
void interrupt_enable_any_key(const char *interrupt_addr);

/** Enables the 'ON KEY ASCIIcode' interrupt. */
void interrupt_enable_specific_key(int key, const char *interrupt_addr);

/** Enables the specified 'SETTICK' interrupt. */
void interrupt_enable_tick(int irq, int64_t period_ns, const char *interrupt_addr);

/**
 * Checks if the specified character matches that set for the 'ON KEY ASCIIcode'
 * interrupt.
 */
bool interrupt_check_key_press(char ch);

/**
 * Flags that a PAUSE has been interrupted.
 *
 * @param return_stmt  the PAUSE statement that execution should be resumed at.
 */
void interrupt_pause(const char *return_stmt);

/**
 * Returns 'true' if a PAUSE statement was interrupted and should be resumed.
 * This checks AND clears the flag that records that a PAUSE was interrupted.
 */
bool interrupt_pause_needs_resuming(void);

void interrupt_enable_serial_rx(int fnbr, int64_t count, const char *interrupt_addr);

void interrupt_disable_serial_rx(int fnbr);

/** Fires a window closed interrupt. */
void interrupt_fire_window_close(MmSurfaceId id);

/** Enables an interrupt. */
void interrupt_enable(InterruptType type, const char *fn);

/** Disables an interrupt. */
void interrupt_disable(InterruptType type);

/** Fires an interrupt. */
void interrupt_fire(InterruptType type);

#endif
