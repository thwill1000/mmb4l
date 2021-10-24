#include <assert.h>

#include "console.h"
#include "interrupt.h"
#include "version.h"

#define skipelement(x)  while(*x) x++

/** Accessed from outside this compilation-unit. */
char *interrupt_return_stmt = NULL;

static int interrupt_count = 0;
static char *interrupt_any_key_addr = NULL;
static int interrupt_specific_key = 0;
static int interrupt_specific_key_pressed = false;
static char *interrupt_specific_key_addr = NULL;

void interrupt_init() {
    interrupt_clear();
}

void interrupt_clear() {
    interrupt_count = 0;
    interrupt_return_stmt = NULL;
    interrupt_any_key_addr = NULL;
    interrupt_specific_key = 0;
    interrupt_specific_key_pressed = false;
    interrupt_specific_key_addr = NULL;
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

int interrupt_check_key_press(char ch) {
    if (ch == interrupt_specific_key && interrupt_specific_key_addr) {
        interrupt_specific_key_pressed = true;
        return 1;
    } else {
        return 0;
    }
}
