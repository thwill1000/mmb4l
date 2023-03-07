/*-*****************************************************************************

MMBasic for Linux (MMB4L)

path.h

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

#if !defined(MMB4L_PATH_H)
#define MMB4L_PATH_H

#include <stdbool.h>

#include "mmresult.h"

/**
 * @brief Creates a directory including any intermediate directories.
 *
 * @param  path  path to the directory to be created.
 * @return       kOk on success, including if it is a pre-existing directory.
 */
MmResult path_mkdir(const char *path);

/** Does the path exist? */
bool path_exists(const char *path);

/** Is the path a directory, or a symbolic link to a directory? */
bool path_is_directory(const char *path);

/** Is the path empty? */
bool path_is_empty(const char *path);

/** Is the path a regular file, or a symbolic link to a regular file? */
bool path_is_regular(const char *path);

/** Does the filename have a given suffix? */
bool path_has_suffix(
        const char *path, const char *suffix, bool case_insensitive);

/**
 * Gets the canonicalized absolute pathname.
 *
 * @param  path            original path to be converted.
 * @param  canonical_path  canonical path is returned in this buffer.
 * @param  sz              size of the 'canonical_path' buffer.
 * @return                 kOk on success.
 */
MmResult path_get_canonical(const char *path, char *canonical_path, size_t sz);

/**
 * Is the path absolute?
 *
 * @param  path  path to check.
 * @return       true if the path is absolute, otherwise false.
 */
bool path_is_absolute(const char *path);

/**
 * Gets the parent of the given path.
 *
 * @param  path         original path to get the parent of.
 * @param  parent_path  parent path is returned in this buffer.
 * @param  sz           size of the 'parent_path' buffer.
 * @return              kOk on success.
 */
MmResult path_get_parent(const char *path, char *parent_path, size_t sz);

/**
 * Appends one path to another.
 *
 * @param  head    path being appended to.
 * @param  tail    path being appended.
 * @param  result  result is returned in this buffer.
 * @param  sz      size of the 'result' buffer.
 * @return         kOk on success.
 */
MmResult path_append(const char *head, const char *tail, char *result, size_t sz);

/**
 * Transforms path by:
 *  - removing any DOS style drive specified, e.g. A:
 *  - replacing leading ~ by user's HOME directory
 *  - replacing any '\' with '/'
 *  - resolving any . and .. elements
 *  - removing any duplicate file separators '/'
 *
 * @param  original_path  path to be transformed.
 * @param  new_path       transformed path is returned in this buffer.
 * @param  sz             size of the 'new_path' buffer.
 * @return                kOk on success.
 */
MmResult path_munge(const char *original_path, char *new_path, size_t sz);

/**
 * Gets the file-extension, if any from a path.
 *
 * @param   path  the path.
 * @return  pointer to the start of the file-extension within 'path', or
 *          pointer to '\0' at the end of 'path' if it has not file-extension.
 */
const char *path_get_extension(const char *path);

/**
 * @brief Gets an autocompletion for the given path.
 *
 * @param path  the path.
 * @param out   buffer that on exit will contain the autocompletion to append
 *              to the path. Will be the empty string if there is no
 *              autocompletion or an error occurred.
 * @param sz    size of the \p out buffer.
 * @return      kOk on success.
 */
MmResult path_complete(const char *path, char *out, size_t sz);

#endif
