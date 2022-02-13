#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/mmb4l.h"
#include "common/console.h"

void console_key_to_string(int ch, char *buf);

volatile int MMAbort;
char g_break_key;

bool interrupt_check_key_press(char ch) { return false; }

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
