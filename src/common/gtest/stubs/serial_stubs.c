/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../serial.h"

MmResult serial_open(const char *comspec, int fnbr) { return kError; }
MmResult serial_close(int fnbr) { return kError; }
int serial_eof(int fnbr) { return -1; }
int serial_getc(int fnbr) { return -1; }
void serial_pump_input(int fnbr) { }
int serial_putc(int fnbr, int ch) { return -1; }
int serial_rx_queue_size(int fnbr) { return -1; }
int serial_write(int fnbr, const char *buf, size_t sz) { return -1; }
