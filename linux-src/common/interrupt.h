#if !defined(MMB4L_INTERRUPT_H)
#define MMB4L_INTERRUPT_H

#include <stdbool.h>
#include <stdint.h>

#include "../Configuration.h"

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
void interrupt_enable_any_key(char *interrupt_addr);

/** Enables the 'ON KEY ASCIIcode' interrupt. */
void interrupt_enable_specific_key(int key, char *interrupt_addr);

/** Enables the specified 'SETTICK' interrupt. */
void interrupt_enable_tick(int irq, int64_t period_ns, char *interrupt_addr);

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
void interrupt_pause(char *return_stmt);

/**
 * Returns 'true' if a PAUSE statement was interrupted and should be resumed.
 * This checks AND clears the flag that records that a PAUSE was interrupted.
 */
bool interrupt_pause_needs_resuming(void);

#endif
