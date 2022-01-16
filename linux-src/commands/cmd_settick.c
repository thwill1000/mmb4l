#include <limits.h>

#include "../common/error.h"
#include "../common/interrupt.h"
#include "../common/mmtime.h"
#include "../common/version.h"

void cmd_settick(void) {
    char *p = checkstring(cmdline, "FAST");
    if (p) {
        ERROR_UNIMPLEMENTED("SETTICK FAST");
        // cmd_fasttick(p);
        // return;
    }

    getargs(&cmdline, 5, ",");
    if (argc != 3 && argc != 5) ERROR_ARGUMENT_COUNT;

    int64_t period_ns = MILLISECONDS_TO_NANOSECONDS(getint(argv[0], 0, INT_MAX));
    int irq = 0;
    if (argc == 5) irq = getint(argv[4], 1, NBRSETTICKS) - 1;
    if (period_ns == 0) {
        interrupt_disable_tick(irq);
    } else {
        interrupt_enable_tick(irq, period_ns, GetIntAddress(argv[2]));
    }
}
