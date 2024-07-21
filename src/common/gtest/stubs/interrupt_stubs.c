/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../interrupt.h"

bool interrupt_check() { return true; }
void interrupt_clear() { }
void interrupt_disable(InterruptType type) {}
void interrupt_disable_serial_rx(int fnbr) {}
void interrupt_enable(InterruptType type, const char *fn) {}
void interrupt_enable_serial_rx(int fnbr, int64_t count, const char *interrupt_addr) {}
void interrupt_fire(InterruptType type) {}
void interrupt_fire_window_close(MmSurfaceId id) {}
