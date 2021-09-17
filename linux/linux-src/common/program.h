#if !defined(MMB4L_PROGRAM_H)
#define MMB4L_PROGRAM_H

extern char CurrentFile[STRINGSIZE];

void ListNewLine(int *ListCnt, int all); // MMBasic/Commands.c

int program_load_file(char *filename);
void program_list_csubs(int all);

#endif
