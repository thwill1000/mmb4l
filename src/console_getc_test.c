/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/mmb4l.h"
#include "common/console.h"

void console_key_to_string(int ch, char *buf);
MmResult error_throw(MmResult result) { return result; }
MmResult error_throw_ex(MmResult result, const char *msg, ...) { return result; }
const char *audio_last_error() { return ""; }
const char *events_last_error() { return ""; }
const char *gamepad_last_error() { return ""; }
const char *graphics_last_error() { return ""; }

volatile int MMAbort;
Options mmb_options;

bool interrupt_check_key_press(char ch) { return false; }

void CheckAbort(void) { console_pump_input(); }

int main(int argc, char **argv) {
    printf("Press Keys\n");

    options_init(&mmb_options);
    mmb_options.break_key = 0; // So that it isn't caught.
    console_init(false);
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
