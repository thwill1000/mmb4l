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

#include <assert.h>
#include <string.h>

#include <SDL.h>

#include "console.h"
#include "error.h"
#include "exit_codes.h"
#include "interrupt.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "queue.h"
#include "serial.h"
#include "utility.h"
#include "../core/commandtbl.h"

#define ERROR_NOT_AN_INTERRUPT  error_throw_ex(kError, "Not in interrupt")
#define ERROR_TOO_MANY_SUBS     error_throw_ex(kError, "Too many SUBs for interrupt")
#define WINDOW_EVENT_QUEUE_CAPACITY  10

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
static Queue interrupt_window_event_queue;
static Interrupt interrupt_list[kInterruptLast];

void interrupt_init() {
    // Only expected to be called once on application startup.
    static bool called = false;
    if (called) ON_FAILURE_ERROR(kInternalFault);
    called = true;

    interrupt_clear();

    ON_FAILURE_ERROR(queue_init(&interrupt_window_event_queue, SDL_WindowEvent,
                                  WINDOW_EVENT_QUEUE_CAPACITY));

    char *p = DUMMY_IRETURN;
    commandtbl_encode(&p, cmdIRET);
    *p = '\0';
}

void interrupt_clear(void) {
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
    queue_clear(&interrupt_window_event_queue);
    for (size_t i = 0; i < kInterruptLast; ++i) memset(interrupt_list + i, 0, sizeof(Interrupt));
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

static inline void interrupt_add_local_integer_const(const char *name, MMINTEGER value) {
    int var_idx;
    ON_FAILURE_ERROR(vartbl_add(name, T_INT | T_CONST | T_IMPLIED, LocalIndex, NULL, 0,
                                  &var_idx));
    vartbl[var_idx].val.i = value;
}

static int handle_window_interrupt() {
    // Get the oldest window event.
    SDL_WindowEvent event;
    MmResult result = queue_dequeue(&interrupt_window_event_queue, &event);
    ON_FAILURE_ERROR_EX(result, false);

    // If this was the last event then reduce the number of interrupts.
    if (queue_is_empty(&interrupt_window_event_queue)) interrupt_count--;

    // Get the MMB4L window.
    MmSurfaceId window_id = graphics_find_window(event.windowID);
    if (!graphics_surface_exists(window_id)) {
        if (event.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            // We can receive this event after the window has been destroyed.
            return true;
        }
        ON_FAILURE_ERROR_EX(kInternalFault, false);
    }
    MmSurface *window = &graphics_surfaces[window_id];

    // If there is no interrupt routine registered then we ignore the event unless is is a
    // CLOSE event in which case we destroy the window and END the program.
    if (!window->interrupt_addr) {
        if (event.event == SDL_WINDOWEVENT_CLOSE) {
            (void) graphics_surface_destroy(window);
            mmb_exit_code = EX_OK;
            longjmp(mark, JMP_END);
        } else {
            return true;
        }
    }

    // Get interrupt SUB signature.
    // It should already have been validated when the window was created.
    FunctionSignature *fn = (FunctionSignature *) GetTempMemory(sizeof(FunctionSignature));
    const char *p2 = window->interrupt_addr;
    const char *cached_line_ptr = CurrentLinePtr;
    CurrentLinePtr = window->interrupt_addr; // So any error is reported on the correct line.
    ON_FAILURE_ERROR_EX(parse_fn_sig(&p2, fn), false);
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
        ON_FAILURE_ERROR_EX(parse_name(&p, name), false);
        ON_FAILURE_ERROR_EX(vartbl_add(name, fn->params[i].type, LocalIndex, NULL, 0,
                                         &var_idx[i]), false);
    }
    vartbl[var_idx[0]].val.i = window_id;
    vartbl[var_idx[1]].val.i = event.event; // event_id

    // Setup local constants corresponding to event constants.
    interrupt_add_local_integer_const("WINDOW_EVENT_CLOSE", SDL_WINDOWEVENT_CLOSE);
    interrupt_add_local_integer_const("WINDOW_EVENT_FOCUS_GAINED", SDL_WINDOWEVENT_FOCUS_GAINED);
    interrupt_add_local_integer_const("WINDOW_EVENT_FOCUS_LOST", SDL_WINDOWEVENT_FOCUS_LOST);
    interrupt_add_local_integer_const("WINDOW_EVENT_MINIMISED", SDL_WINDOWEVENT_MINIMIZED);
    interrupt_add_local_integer_const("WINDOW_EVENT_MAXIMISED", SDL_WINDOWEVENT_MAXIMIZED);
    interrupt_add_local_integer_const("WINDOW_EVENT_RESTORED", SDL_WINDOWEVENT_RESTORED);

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
    if (!queue_is_empty(&interrupt_window_event_queue)) handle_window_interrupt();

    // All other interrupts.
    for (size_t i = 0; i < kInterruptLast; ++i) {
        Interrupt *interrupt = interrupt_list + i;
        if (interrupt->fn && interrupt->fired) {
            interrupt->fired = false;
            return handle_interrupt(interrupt->fn);
        }
    }

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

void interrupt_fire_window_event(SDL_WindowEvent *event) {
    MmResult result = queue_enqueue(&interrupt_window_event_queue, *event);
    if (result == kContainerFull) {
        // Discard the oldest event.
        SDL_WindowEvent tmp;
        ON_FAILURE_ERROR(queue_dequeue(&interrupt_window_event_queue, &tmp));
        result = queue_enqueue(&interrupt_window_event_queue, *event);
    }
    ON_FAILURE_ERROR(result);
    if (queue_size(&interrupt_window_event_queue) == 1) interrupt_count++;
}

void interrupt_enable(InterruptType type, const char *fn) {
    if (interrupt_list[type].fn) interrupt_count--;
    interrupt_list[type].fn = fn;
    interrupt_list[type].fired = false;
    if (fn) interrupt_count++;
}

void interrupt_disable(InterruptType type) {
    interrupt_enable(type, NULL);
}

void interrupt_fire(InterruptType type) {
    interrupt_list[type].fired = true;
}
