#if !defined(INTERRUPT_H)
#define INTERRUPT_H

/**
 * Checks if an interrupt has occurred and if so, set the next command to the interrupt routine.
 *
 * @return  'true' if interrupt detected, otherwise 'false'.
 */
int interrupt_check(void);

/** Handles return from an interrupt. */
void interrupt_return(void);

#endif
