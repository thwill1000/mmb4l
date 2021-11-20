#if !defined(MMB4L_FILE)
#define MMB4L_FILE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../Configuration.h"

enum FileEntryType { fet_closed, fet_file, fet_serial };

typedef struct {
    enum FileEntryType type;
    union {
        FILE *file_ptr;
        int serial_fd;
    };
} FileEntry;

extern FileEntry file_table[MAXOPENFILES];

void MMgetline(int fnbr, char *p); // main.c

/** Finds the first available free file number. */
int file_find_free(void);

void file_open(char *fname, char *mode, int fnbr);
void file_close(int fnbr);
void file_close_all(void);
int file_eof(int fnbr);
int file_getc(int fnbr);
char file_putc(char ch, int fnbr);

/** Does the file exist? */
bool file_exists(const char *path);

/** Is the file empty? */
bool file_is_empty(const char *path);

/** Is the file a regular file, or a symbolic link to a regular file? */
bool file_is_regular(const char *path);

/**
 * Gets the file-extension, if any from a path.
 *
 * @param   path  the path.
 * @return  pointer to the start of the file-extension within 'path', or
 *          pointer to '\0' at the end of 'path' if it has not file-extension.
 */
const char *file_get_extension(const char *path);

/** Does the filename have a given suffix? */
bool file_has_suffix(
        const char *path, const char *suffix, bool case_insensitive);

#endif
