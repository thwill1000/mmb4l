/*
 * Copyright (c) 2021-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/console.h"
#include "common/error.h"
#include "common/exit_codes.h"
#include "common/options.h"

// Defined in "main.c"
Options mmb_options;
volatile int MMAbort;
void CheckAbort(void) { console_pump_input(); }

// Defined in "common/console.c"
void console_key_to_string(int ch, char *buf);

// Defined in "common/fonttbl.c"
uint32_t font_height(uint32_t font) { return 12; }
uint32_t font_width(uint32_t font) { return 8; }

// Defined in "common/graphics.c"
uint32_t graphics_font;

// Defined in "common/interrupt.c"
bool interrupt_check_key_press(char ch) { return false; }

// Defined in "common/mmgetchar.c"
int MMgetchar(void) { return -1; }

int main(int argc, char **argv) {
    printf("Press Keys\n");

    options_init(&mmb_options);
    mmb_options.break_key = 0; // So that it isn't caught.
    ON_FAILURE_EXIT(console_init(false));
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
