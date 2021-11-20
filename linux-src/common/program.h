#if !defined(MMB4L_PROGRAM_H)
#define MMB4L_PROGRAM_H

#define EDIT_BUFFER_SIZE  512 * 1024

void ListNewLine(int *ListCnt, int all); // MMBasic/Commands.c

/** @return  0 on success, -1 on error.*/
int program_load_file(char *filename);

void program_list_csubs(int all);

#endif
