#if !defined(MMB4L_FILE)
#define MMB4L_FILE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../Configuration.h"

#define HANDLE uint64_t

extern FILE *MMFilePtr[MAXOPENFILES];
extern HANDLE *MMComPtr[MAXOPENFILES];
extern int OptionFileErrorAbort;
extern char CurrentFile[STRINGSIZE];

void CloseAllFiles(void);
int FindFreeFileNbr(void);
void MMfclose(int file_num);
int MMfeof(int filenbr);
int MMfgetc(int file_num);
void MMgetline(int filenbr, char *p); // main.c
void MMfopen(char *fname, char *mode, int file_num);
char MMfputc(char c, int file_num);

/** Does the file exist? */
bool file_exists(const char *path);

/** Is the file empty? */
bool file_is_empty(const char *path);

/** Is the file a regular file, or a symbolic link to a regular file? */
bool file_is_regular(const char *path);

/** Does the filename have a given extension/suffix? */
bool file_has_extension(
        const char *path, const char *extension, bool case_insensitive);

#endif
