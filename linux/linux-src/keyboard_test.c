#include <stdio.h>
#include <stdlib.h>

#include "common/console.h"
#include "common/global_aliases.h"

volatile int MMAbort;
char g_break_key;

void CheckAbort(void) {}

int main(int argc, char **argv) {
    printf("Press Keys\n");

    console_enable_raw_mode();
    atexit(console_disable_raw_mode);

    for (;;) {
        int ch = console_getc();
    }
}