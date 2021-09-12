#if !defined(MMB4L_FILE)
#define MMB4L_FILE

#include <stdint.h>

#include "version.h"

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
char *MMgetcwd(void);
void MMgetline(int filenbr, char *p); // main.c
void MMfopen(char *fname, char *mode, int file_num);
char MMfputc(char c, int file_num);

#endif
