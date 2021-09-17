#include <stdio.h>
#include <stdlib.h>

#include "common/console.h"
#include "common/global_aliases.h"

void console_key_to_string(int ch, char *buf);

volatile int MMAbort;
char g_break_key;
int g_key_complete = 0;
char *g_key_interrupt = NULL;
int g_key_select = 0;

void CheckAbort(void) { console_pump_input(); }

int main(int argc, char **argv) {
    printf("Press Keys\n");

    console_init();
    console_enable_raw_mode();
    atexit(console_disable_raw_mode);

    int ch = 0;
    char buf[10];
    while (ch != 3) {
        ch = console_getc();
        if (ch != -1) {
            console_key_to_string(ch, buf);
            printf("%s\n", buf);
        }
    }
}
