#include "console.h"
#include "interrupt.h"
#include "version.h"

#define skipelement(x)  while(*x) x++

extern int g_key_complete;
extern char *g_key_interrupt;

static int handle_interrupt(char *interrupt_address) {
    static char rti[2]; // TODO: does this really need to be static ?

    LocalIndex++;                                                   // IRETURN will decrement this
    InterruptReturn = nextstmt;                                     // for when IRETURN is executed
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

int interrupt_check(void) {

    // Quick exit if no interrupts have been set.
    if (!InterruptUsed) return false;

    // Skip interrupt processing if we are already processing an interrupt or are in immediate mode.
    if (InterruptReturn != NULL || CurrentLinePtr == NULL) return false;

    // Check for an ON KEY loc interrupt.
    if (OnKeyGOSUB && console_kbhit()) {
        return handle_interrupt(OnKeyGOSUB);
    }

    // Check for an ON KEY ascii_code%, handler_sub() interrupt.
    if (g_key_interrupt != NULL && g_key_complete) {
        g_key_complete = false;
        return handle_interrupt(g_key_interrupt);
    }

    return false;
}

void interrupt_return(void) {
    if (InterruptReturn == NULL) error("Not in interrupt");
    checkend(cmdline);
    nextstmt = InterruptReturn;
    if (LocalIndex) ClearVars(LocalIndex--);  // delete any local variables
    TempMemoryIsChanged = true;  // signal that temporary memory should be checked
    *CurrentInterruptName = 0;
    InterruptReturn = NULL;
}
