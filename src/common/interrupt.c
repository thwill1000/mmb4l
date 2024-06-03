/*-*****************************************************************************

MMBasic for Linux (MMB4L)

interrupt.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "mmb4l.h"
#include "console.h"
#include "error.h"
#include "exit_codes.h"
#include "interrupt.h"
#include "mmtime.h"
#include "serial.h"
#include "../core/commandtbl.h"

#include <assert.h>

#define ERROR_NOT_AN_INTERRUPT  error_throw_ex(kError, "Not in interrupt")
#define ERROR_TOO_MANY_SUBS     error_throw_ex(kError, "Too many SUBs for interrupt")

#define skipelement(x)  while(*x) x++

typedef struct {
    int64_t due_ns;
    const char *interrupt_addr;
    int64_t period_ns;
} TickStruct;

typedef struct {
    int64_t count;
    const char *interrupt_addr;
} SerialRxStruct;

static char DUMMY_IRETURN[3]; // Dummy IRETURN call.
static int interrupt_count = 0;
static bool interrupt_legacy = false; // Is the current interrupt using a label/line number ?
static const char *interrupt_any_key_addr = NULL;
static bool interrupt_pause_flag = false;
static const char *interrupt_return_stmt = NULL;
static int interrupt_specific_key = 0;
static int interrupt_specific_key_pressed = false;
static const char *interrupt_specific_key_addr = NULL;
static TickStruct interrupt_ticks[NBRSETTICKS + 1];
static SerialRxStruct interrupt_serial_rx[MAXOPENFILES + 1];
static ErrorState interrupt_error_state;
static MmSurfaceId interrupt_window_close_id;

void interrupt_init() {
    interrupt_clear();
    char *p = DUMMY_IRETURN;
    commandtbl_encode(&p, cmdIRET);
    *p = '\0';
}

void interrupt_clear() {
    interrupt_count = 0;
    interrupt_return_stmt = NULL;
    interrupt_any_key_addr = NULL;
    interrupt_pause_flag = false;
    interrupt_specific_key = 0;
    interrupt_specific_key_pressed = false;
    interrupt_specific_key_addr = NULL;
    for (int i = 0; i <= NBRSETTICKS; ++i) {
        interrupt_ticks[i].due_ns = 0;
        interrupt_ticks[i].interrupt_addr = NULL;
        interrupt_ticks[i].period_ns = 0;
    }
    for (int i = 0; i <= MAXOPENFILES; ++i) {
        interrupt_serial_rx[i].count = 0;
        interrupt_serial_rx[i].interrupt_addr = NULL;
    }
    interrupt_window_close_id = -1;
}

bool interrupt_running() {
    return interrupt_return_stmt != NULL;
}

static int handle_interrupt(const char *interrupt_address) {
    LocalIndex++;  // IRETURN will decrement this unless the interrupt routine is a SUB in which
                   // case exiting the SUB decrements it.
    mmb_error_state_ptr = &interrupt_error_state; // Swap to the interrupt error state
    error_init(mmb_error_state_ptr);              //   and clear it
    interrupt_return_stmt = nextstmt;             //   for when IRETURN is executed.

    const CommandToken token = commandtbl_decode(interrupt_address);
    if (token == cmdSUB) {
        if (gosubindex >= MAXGOSUB) ERROR_TOO_MANY_SUBS;
        errorstack[gosubindex] = CurrentLinePtr;
        gosubstack[gosubindex++] = DUMMY_IRETURN;  // Return from the subroutine to the dummy IRETURN command.
        skipelement(interrupt_address);            // Point to the body of the SUB.
        interrupt_legacy = false;
    } else if (token == cmdFUN) {
        return kInternalFault;
    } else {
        // Label or line number.
        interrupt_legacy = true;
    }

    // Set the next command to be the body of the interrupt routine.
    nextstmt = interrupt_address;
    return true;
}

static int handle_window_interrupt() {
    MmSurfaceId window_id = interrupt_window_close_id;
    interrupt_window_close_id = -1; // Clear the interrupt.
    interrupt_count--;

    if (!graphics_surface_exists(window_id)) error_throw(kInternalFault);
    MmSurface *window = &graphics_surfaces[window_id];

    if (!window->interrupt_addr) {
        graphics_surface_destroy(window);
        mmb_exit_code = EX_OK;
        longjmp(mark, JMP_END);
    }

    // Get interrupt SUB signature.
    // It should already have been validated when the window was created.
    FunctionSignature *fn = (FunctionSignature *) GetTempMemory(sizeof(FunctionSignature));
    const char *p2 = window->interrupt_addr;
    const char *cached_line_ptr = CurrentLinePtr;
    CurrentLinePtr = window->interrupt_addr; // So any error is reported on the correct line.
    MmResult result = parse_fn_sig(&p2, fn);
    if (FAILED(result)) error_throw(result);
    CurrentLinePtr = cached_line_ptr;

    // Setup stack and return state.
    LocalIndex++;  // IRETURN will decrement this unless the interrupt routine is a SUB in which
                   // case exiting the SUB decrements it.
    mmb_error_state_ptr = &interrupt_error_state;  // Swap to the interrupt error state
    error_init(mmb_error_state_ptr);               //   and clear it
    interrupt_return_stmt = nextstmt;              //   for when IRETURN is executed
    if (gosubindex >= MAXGOSUB) ERROR_TOO_MANY_SUBS;
    errorstack[gosubindex] = CurrentLinePtr;
    gosubstack[gosubindex++] = DUMMY_IRETURN;  // Return from the subroutine to the dummy IRETURN command.
    interrupt_legacy = false;

    // Setup local variables corresponding to parameters.
    int var_idx[2];
    char name[MAXVARLEN + 1];
    for (uint8_t i = 0; i < 2; ++i) {
        const char *p = fn->addr + fn->params[i].name_offset;
        result = parse_name(&p, name);
        if (FAILED(result)) error_throw(result);
        result = vartbl_add(name, fn->params[i].type, LocalIndex, NULL, 0, &var_idx[i]);
        if (FAILED(result)) error_throw(result);
    }
    vartbl[var_idx[0]].val.i = window_id;
    vartbl[var_idx[1]].val.i = 1; // event_id

    ClearSpecificTempMemory(fn);

    // Set the next command to be the body of the interrupt SUB.
    const char *interrupt_addr = window->interrupt_addr;
    skipelement(interrupt_addr);
    nextstmt = interrupt_addr;

    return true;
}

bool interrupt_check(void) {

    // Quick exit if no interrupts have been set.
    if (interrupt_count < 1) return false;

    // Skip interrupt processing if we are already processing an interrupt or are in immediate mode.
    if (interrupt_return_stmt != NULL || CurrentLinePtr == NULL) return false;

    // Check for an ON KEY loc interrupt.
    if (interrupt_any_key_addr && console_kbhit()) {
        return handle_interrupt(interrupt_any_key_addr);
    }

    // Check for an ON KEY ascii_code%, handler_sub() interrupt.
    if (interrupt_specific_key_addr && interrupt_specific_key_pressed) {
        interrupt_specific_key_pressed = false;
        return handle_interrupt(interrupt_specific_key_addr);
    }

    // Check for SETTICK interrupts.
    int64_t now_ns = mmtime_now_ns();
    // printf("%ld\n", now_ns);
    for (int i = 0; i < NBRSETTICKS; ++i) {
        // printf("Interrupt %d, period = %ld, due = %ld, fn = %ld\n",
        //         i,
        //         interrupt_ticks[i].period_ns,
        //         interrupt_ticks[i].due_ns,
        //         interrupt_ticks[i].interrupt_addr);
        if (interrupt_ticks[i].interrupt_addr) {
            if (now_ns >= interrupt_ticks[i].due_ns) {
                interrupt_ticks[i].due_ns += interrupt_ticks[i].period_ns;
                return handle_interrupt(interrupt_ticks[i].interrupt_addr);
            }
        }
    }

    // Check for serial port interrupts.
    for (int i = 1; i <= MAXOPENFILES; ++i) {
        SerialRxStruct *entry = &(interrupt_serial_rx[i]);
        if (entry->interrupt_addr
                && serial_rx_queue_size(i) >= entry->count) {
            return handle_interrupt(entry->interrupt_addr);
        }
    }

    // Check for window interrupts.
    if (interrupt_window_close_id != -1) handle_window_interrupt();

    return false;
}

void interrupt_return(void) {
    if (interrupt_return_stmt == NULL) ERROR_NOT_AN_INTERRUPT;
    checkend(cmdline);
    nextstmt = interrupt_return_stmt;
    if (interrupt_legacy && LocalIndex > 0) {
        // If the interrupt routine was not a SUB then clear local variables and decrement
        // LocalIndex. If it was a SUB then leaving the SUB will have handled this.
        ClearVars(LocalIndex--);
    }
    TempMemoryIsChanged = true;  // signal that temporary memory should be checked
    *CurrentInterruptName = 0;
    interrupt_return_stmt = NULL;
    mmb_error_state_ptr = &mmb_normal_error_state; // swap back to the normal error state
    if (mmb_error_state_ptr->skip > 0) mmb_error_state_ptr->skip++;
}

void interrupt_disable_any_key() {
    if (interrupt_any_key_addr) {
        interrupt_any_key_addr = NULL;
        interrupt_count--;
    }
}

void interrupt_enable_any_key(const char *interrupt_addr) {
    if (!interrupt_any_key_addr) interrupt_count++;
    interrupt_any_key_addr = interrupt_addr;
}

void interrupt_disable_specific_key() {
    if (interrupt_specific_key_addr) {
        interrupt_specific_key_addr = NULL;
        interrupt_count--;
    }
}

void interrupt_enable_specific_key(int key, const char *interrupt_addr) {
    if (!interrupt_specific_key_addr) interrupt_count++;
    interrupt_specific_key = key;
    interrupt_specific_key_addr = interrupt_addr;
}

void interrupt_disable_tick(int irq) {
    assert(irq >= 0 && irq < NBRSETTICKS);
    if (interrupt_ticks[irq].interrupt_addr) {
        interrupt_ticks[irq].due_ns       = 0;
        interrupt_ticks[irq].interrupt_addr = NULL;
        interrupt_ticks[irq].period_ns    = 0;
        interrupt_count--;
    }
}

void interrupt_enable_tick(int irq, int64_t period_ns, const char *interrupt_addr) {
    assert(irq >= 0 && irq < NBRSETTICKS);
    assert(period_ns > 0);
    assert(interrupt_addr);
    if (!interrupt_ticks[irq].interrupt_addr) interrupt_count++;
    interrupt_ticks[irq].due_ns       = mmtime_now_ns() + period_ns;
    interrupt_ticks[irq].interrupt_addr = interrupt_addr;
    interrupt_ticks[irq].period_ns    = period_ns;
    // printf("Interrupt %d, period = %ld, due = %ld, fn = %ld\n",
    //         irq,
    //         interrupt_ticks[irq].period_ns,
    //         interrupt_ticks[irq].due_ns,
    //         interrupt_ticks[irq].interrupt_addr);
}

bool interrupt_check_key_press(char ch) {
    if (ch == interrupt_specific_key && interrupt_specific_key_addr) {
        interrupt_specific_key_pressed = true;
        return true;
    } else {
        return false;
    }
}

void interrupt_pause(const char *return_stmt) {
    assert(!interrupt_pause_flag);
    interrupt_return_stmt = return_stmt;
    interrupt_pause_flag = true;
}

bool interrupt_pause_needs_resuming(void) {
    bool result = interrupt_pause_flag;
    interrupt_pause_flag = false;
    return result;
}

void interrupt_enable_serial_rx(int fnbr, int64_t count, const char *interrupt_addr) {
    assert(fnbr > 0 && fnbr <= MAXOPENFILES);
    assert(count > 0);
    assert(interrupt_addr);
    assert(!interrupt_serial_rx[fnbr].interrupt_addr);
    interrupt_serial_rx[fnbr].count = count;
    interrupt_serial_rx[fnbr].interrupt_addr = interrupt_addr;
    interrupt_count++;
}

void interrupt_disable_serial_rx(int fnbr) {
    assert(fnbr > 0 && fnbr <= MAXOPENFILES);
    if (interrupt_serial_rx[fnbr].interrupt_addr) {
        interrupt_serial_rx[fnbr].count = 0;
        interrupt_serial_rx[fnbr].interrupt_addr = NULL;
        interrupt_count--;
    }
}

void interrupt_fire_window_close(MmSurfaceId id) {
    if (interrupt_window_close_id == -1) {
        interrupt_count++;
        interrupt_window_close_id = id;
    }
}
