/*
 * Copyright (c) 2021-2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/mmb4l.h"
#include "common/console.h"

void console_key_to_string(int ch, char *buf);
void error_throw(MmResult error) {}
void error_throw_ex(MmResult error, const char *msg, ...) {}

volatile int MMAbort;
Options mmb_options;

bool interrupt_check_key_press(char ch) { return false; }

void CheckAbort(void) { console_pump_input(); }

int main(int argc, char **argv) {
    printf("Press Keys\n");

    options_init(&mmb_options);
    mmb_options.break_key = 0; // So that it isn't caught.
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
