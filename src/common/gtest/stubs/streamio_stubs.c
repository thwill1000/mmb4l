/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../streamio.h"

MmResult streamio_init(MmResult (*flush_fn)(),
                       MmResult (*putc_fn)(char),
                       MmResult (*write_fn)(const char *, size_t *)) {
    return kOk;
}

MmResult streamio_close(int fnbr) { return kOk; }
MmResult streamio_close_all(void) { return kOk; }
int streamio_eof(int fnbr) { return 0; }
int streamio_find_free(void) { return 0; }
MmResult streamio_flush(int fnbr) { return kOk; }
int streamio_getc(int fnbr) { return 0; }
bool streamio_is_file(int fnbr) { return false; }
bool streamio_is_serial(int fnbr) { return false; }
int streamio_loc(int fnbr) { return 0; }
int streamio_lof(int fnbr) { return 0; }
MmResult streamio_open(const char *path, const char *mode, int fnbr) { return kOk; }
int streamio_putc(int fnbr, int ch) { return 0; }
size_t streamio_read(int fnbr, char *buf, size_t sz) { return 0; }
void streamio_seek(int fnbr, int idx) {}
size_t streamio_write(int fnbr, const char *buf, size_t sz) { return 0; }
