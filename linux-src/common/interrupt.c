#include <assert.h>

#include "console.h"
#include "interrupt.h"
#include "mmtime.h"
#include "serial.h"
#include "version.h"

#define skipelement(x)  while(*x) x++

typedef struct {
    int64_t due_ns;
    char *interrupt_addr;
    int64_t period_ns;
} TickStruct;

typedef struct {
    int64_t count;
    char *interrupt_addr;
} SerialRxStruct;

static int interrupt_count = 0;
static char *interrupt_any_key_addr = NULL;
static bool interrupt_pause_flag = false;
static char *interrupt_return_stmt = NULL;
static int interrupt_specific_key = 0;
static int interrupt_specific_key_pressed = false;
static char *interrupt_specific_key_addr = NULL;
static TickStruct interrupt_ticks[NBRSETTICKS + 1];
static SerialRxStruct interrupt_serial_rx[MAXOPENFILES + 1];

void interrupt_init() {
    interrupt_clear();
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
}

bool interrupt_running() {
    return interrupt_return_stmt != NULL;
}

static int handle_interrupt(char *interrupt_address) {
    static char rti[2]; // TODO: does this really need to be static ?

    LocalIndex++;                                                   // IRETURN will decrement this
    interrupt_return_stmt = nextstmt;                               // for when IRETURN is executed
    // if the interrupt is pointing to a SUB token we need to call a subroutine
    if  (*interrupt_address == cmdSUB) {
        rti[0] = cmdIRET;                                           // setup a dummy IRETURN command
        rti[1] = 0;
        if (gosubindex >= MAXGOSUB) error("Too many SUBs for interrupt");
        errorstack[gosubindex] = CurrentLinePtr;
        gosubstack[gosubindex++] = rti;                             // return from the subroutine to the dummy IRETURN command
        LocalIndex++;                                               // return from the subroutine will decrement LocalIndex
        skipelement(interrupt_address);                             // point to the body of the subroutine
    }

    nextstmt = interrupt_address;                                   // the next command will be in the interrupt routine
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

    return false;
}

void interrupt_return(void) {
    if (interrupt_return_stmt == NULL) error("Not in interrupt");
    checkend(cmdline);
    nextstmt = interrupt_return_stmt;
    if (LocalIndex) ClearVars(LocalIndex--);  // delete any local variables
    TempMemoryIsChanged = true;  // signal that temporary memory should be checked
    *CurrentInterruptName = 0;
    interrupt_return_stmt = NULL;
}

void interrupt_disable_any_key() {
    if (interrupt_any_key_addr) {
        interrupt_any_key_addr = NULL;
        interrupt_count--;
    }
}

void interrupt_enable_any_key(char *interrupt_addr) {
    if (!interrupt_any_key_addr) interrupt_count++;
    interrupt_any_key_addr = interrupt_addr;
}

void interrupt_disable_specific_key() {
    if (interrupt_specific_key_addr) {
        interrupt_specific_key_addr = NULL;
        interrupt_count--;
    }
}

void interrupt_enable_specific_key(int key, char *interrupt_addr) {
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

void interrupt_enable_tick(int irq, int64_t period_ns, char *interrupt_addr) {
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

void interrupt_pause(char *return_stmt) {
    assert(!interrupt_pause_flag);
    interrupt_return_stmt = return_stmt;
    interrupt_pause_flag = true;
}

bool interrupt_pause_needs_resuming(void) {
    bool result = interrupt_pause_flag;
    interrupt_pause_flag = false;
    return result;
}

void interrupt_enable_serial_rx(int fnbr, int64_t count, char *interrupt_addr) {
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
