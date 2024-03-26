/*-*****************************************************************************

MMBasic for Linux (MMB4L)

program.h

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#if !defined(MMB4L_PROGRAM_H)
#define MMB4L_PROGRAM_H

#include "../Configuration.h"
#include "mmresult.h"

#define EDIT_BUFFER_SIZE  512 * 1024

extern char CurrentFile[STRINGSIZE];

void ListNewLine(int *ListCnt, int all); // MMBasic/Commands.c

/** @brief Dumps contents of \p ProgMemory to STDOUT. */
void program_dump_memory();

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
 * @return           kOk on success.
 */
MmResult program_get_bas_file(const char *filename, char *out);

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
 * @return             kOk on success.
 */
MmResult program_get_inc_file(const char *parent_file, const char *filename, char *out);

/** @return  0 on success, -1 on error.*/
int program_load_file(char *filename);

void program_list_csubs(int all);

/**
 * @brief Pre-process a file into the edit buffer.
 *
 * @param          file_path    The full path to the file to pre-process.
 * @param[in,out]  p            Pointer to the insertion point in the edit buffer.
 * @param          edit_buffer  Pointer to the start of the edit buffer.
 * @return                      kOk on success.
 */
MmResult program_process_file(const char *file_path, char **p, char *edit_buffer);

/**
 * @brief Pre-process a single line of the program (in place).
 *
 * @param[in,out]  line  The line to pre-process.
 * @return               kOk on success.
 */
MmResult program_process_line(char *line);

/**
 * @brief Initialises/allocates the string replacement map.
 */
void program_init_defines();

/**
 * @brief Terminates/frees the string replacement map.
 */
void program_term_defines();

/**
 * @brief Add an entry to the string replacement map.
 *
 * @param  from  The 'from' string.
 * @param  to    The 'to' string.
 * @return       kOk on success,
 *               kTooManyDefines if there are too many #DEFINEs.
 */
MmResult program_add_define(const char *from, const char *to);

/**
 * @brief Gets the number of entries in the string replacement map.
 */
int program_get_num_defines();

/**
 * @brief Gets an entry from the string replacement map.
 *
 * @param       idx   Index of the entry to get.
 * @param[out]  from  On exit points to the entry's 'from' property.
 * @param[out]  to    On exit points to the entry's 'from' property.
 * @return            kOk on success.
 */
MmResult program_get_define(size_t idx, const char **from, const char **to);

#endif
