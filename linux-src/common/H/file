#if !defined(MMB4L_FILE)
#define MMB4L_FILE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../Configuration.h"
#include "rx_buf.h"

enum FileEntryType { fet_closed, fet_file, fet_serial };

typedef struct {
    enum FileEntryType type;
    union {
        FILE *file_ptr;
        int serial_fd;
    };
    RxBuf rx_buf;
} FileEntry;

extern FileEntry file_table[MAXOPENFILES + 1];

void MMgetline(int fnbr, char *p); // main.c

/** Finds the first available free file number. */
int file_find_free(void);

void file_open(char *fname, char *mode, int fnbr);
void file_close(int fnbr);
void file_close_all(void);
int file_eof(int fnbr);
int file_getc(int fnbr);
int file_loc(int fnbr);
int file_lof(int fnbr);
int file_putc(int ch, int fnbr);
void file_seek(int fnbr, int idx);

#endif
