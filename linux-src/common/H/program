#if !defined(MMB4L_PROGRAM_H)
#define MMB4L_PROGRAM_H

#include "../Configuration.h"

#define EDIT_BUFFER_SIZE  512 * 1024

extern char CurrentFile[STRINGSIZE];

void ListNewLine(int *ListCnt, int all); // MMBasic/Commands.c

/**
 * @brief Gets the absolute canonical path to an MMBasic program file (.bas)
 *
 * If 'filename' does not have a (".bas", ".BAS", "Bas") file extension then
 * one will be appended to the result. This will try to match the
 * capitalisation of an existing file in the order (".bas", ".BAS", "Bas") if
 * one isn't present then will append ".bas".
 *
 * @param filename   unprocessed filename.
 * @param file_path  absolute canonical path to file is returned in this buffer.
 * @return           the value of 'file_path' on success,
 *                   otherwise sets 'errno' and returns NULL.
 */
char *program_get_bas_file(const char *filename, char *out);

/**
 * @brief Gets the absolute canonical path to an MMBasic include file (.inc)
 *
 * If 'filename' does not have a (".inc", ".INC", "Inc") file extension then
 * one will be appended to the result. This will try to match the
 * capitalisation of an existing file in the order (".inc", ".INC", "Inc") if
 * one isn't present then will append ".inc".
 *
 * @param parent_file  path to the MMBasic program file that is including the file.
 * @param filename     unprocessed filename.
 * @param out          absolute canonical path to file is returned in this buffer.
 * @return             the value of 'out' on success,
 *                     otherwise sets 'errno' and returns NULL.
 */
char *program_get_inc_file(const char *parent_file, const char *filename, char *out);

/** @return  0 on success, -1 on error.*/
int program_load_file(char *filename);

void program_list_csubs(int all);

#endif
