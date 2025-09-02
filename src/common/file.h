/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file.h

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMB4L_FILE)
#define MMB4L_FILE

#include <linux/limits.h> // For PATH_MAX
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "mmresult.h"

// Maximum number of files returned by file_list()
#define FILE_LIST_MAX  512

// Forward declaration for directory stream structure
struct s_DirStream;

// Opaque handle for directory operations
typedef struct s_DirStream DirStream;

/**
 * Enumeration of different file system entry types
 * Used to classify directory entries when traversing directories
 */
typedef enum {
   kFileTypeBlockDevice,      ///< Block device (e.g., hard drives, SSDs)
   kFileTypeCharacterDevice,  ///< Character device (e.g., terminals, serial ports)
   kFileTypeDirectory,        ///< Directory/folder
   kFileTypeNamedPipe,        ///< Named pipe (FIFO)
   kFileTypeSymbolicLink,     ///< Symbolic link to another file/directory
   kFileTypeRegularFile,      ///< Regular file
   kFileTypeSocket,           ///< Unix domain socket
   kFileTypeUnknown,          ///< Unknown or unsupported entry type
} FileType;

/**
 * Structure representing a directory entry
 * Contains the name and type information for files/directories
 */
typedef struct {
    char name[STRINGSIZE];  ///< Name of the file/directory (null-terminated string)
    FileType type;          ///< Type of the entry (file, directory, etc.)
} DirEntry;

/**
 * Structure containing information about a file or directory entry.
 * Used to store metadata retrieved from filesystem queries.
 */
typedef struct {
    bool exists;    ///< True if the file/directory exists, false otherwise
    FileType type;  ///< Type of entry (file, directory, symbolic link, etc.)
    off_t size;     ///< Size of the file in bytes
    time_t mtime;   ///< Last modification time as Unix timestamp
} FileInfo;

/**
 * Enumeration of file sorting options for directory listings
 * Determines the order in which files are returned by file_list()
 */
typedef enum {
    kFileSortByName,       ///< Sort alphabetically by filename
    kFileSortBySize,       ///< Sort by file size (smallest to largest)
    kFileSortByTime,       ///< Sort by modification time (oldest to newest)
    kFileSortByExtension,  ///< Sort by file extension, then by name
} FileSort;

typedef struct {
    char      *name;  ///< Pointer to filename (in FileList#buf)
    FileInfo  info;   ///< File size, modification time/date, type, etc.
} FileMatch;

typedef struct {
    char      directory[PATH_MAX];   ///< The directory path
    FileMatch files[FILE_LIST_MAX];  ///< The matched files
    size_t    count;                 ///< Number of matched files, may be > FILE_LIST_MAX
    bool      buf_full;              ///< True if `buf` is full
    uint64_t  free_space;            ///< Remaining free space on the drive in bytes
    char buf[32 * FILE_LIST_MAX];    ///< Storage for file names
} FileList;

/**
 * Checks if a named regulat file exists in the filesystem.
 *
 * @param[in]  filename  Path to the file to check
 * @return               true if file exists and is a regular file, false otherwise
 */
bool file_exists_regular(const char *filename);

/**
 * Checks if a named directory exists in the filesystem.
 *
 * @param[in]  dirname  Path to the directory to check
 * @return              true if file exists and is a directory, false otherwise
 */
bool file_exists_dir(const char *dirname);

/**
 * Gets the amount of free space on the filesystem containing the specified path.
 * 
 * This function uses the statvfs() system call to query filesystem statistics
 * and returns the number of bytes available to non-privileged users. The path
 * can refer to either a file or directory - the function will determine the
 * filesystem containing that path.
 *
 * @param[in]  path        Path to check (can be file or directory, relative or absolute)
 * @param[out] free_space  Pointer to store the free space in bytes
 * @return                 kOk on success, error code on failure
 * 
 * @note The returned value represents space available to non-privileged users
 *       (f_bavail), which may be less than the total free space (f_bfree) if
 *       the filesystem reserves space for the superuser.
 * 
 * @note On filesystems that don't support space queries or if the path doesn't
 *       exist, this function will return an appropriate error code.
 * 
 * @example
 * @code
 * uint64_t free_bytes;
 * MmResult result = file_get_free_space("/home/user", &free_bytes);
 * if (result == kOk) {
 *     printf("Free space: %llu bytes\n", (unsigned long long)free_bytes);
 * }
 * @endcode
 */
MmResult file_get_free_space(const char *path, uint64_t *free_space);

/**
 * Gets the size of a file in bytes.
 *
 * @param[in]  path  Path to the file
 * @param[out] size  Pointer to store the size
 * @return           kOk on success, error code on failure
 */
MmResult file_size(const char *path, off_t *size);

/**
 * Extracts the basename (filename without directory) from a path.
 *
 * @param[in]  path  File path (may be modified by the function)
 * @return           Pointer to the basename portion of the path
 */
char *file_basename(char *path);

/**
 * Changes the current working directory.
 *
 * @param[in]  dirname  Path to the new working directory
 * @return              kOk on success, error code on failure
 */
MmResult file_chdir(const char *dirname);

/**
 * Closes a directory stream.
 *
 * @param[in]  stream  Directory stream to close
 * @return             kOk on success, error code on failure
 */
MmResult file_closedir(DirStream *stream);

/**
 * Deletes a file from the filesystem.
 *
 * @param[in]  filename  Path to the file to delete
 * @return               kOk on success, error code on failure
 */
MmResult file_delete(const char *filename);

/**
 * Extracts the directory name (path without filename) from a path.
 *
 * @param[in]  path  File path (may be modified by the function)
 * @return           Pointer to the directory portion of the path
 */
char *file_dirname(char *path);

/**
 * Checks if end-of-file has been reached.
 *
 * @param[in]  fnbr  File number to check
 * @return           1 if at EOF, 0 if not at EOF, 0 on error
 */
int file_eof(int fnbr);

/**
 * Reads a single character from a file.
 *
 * @param[in]  fnbr  File number to read from (0 for console input)
 * @return           Character read (0-255), or -1 on EOF/error
 */
int file_getc(int fnbr);

/**
 * Gets the current working directory.
 *
 * @param[out] buf   Buffer to store the directory path
 * @param[in]  size  Size of the buffer
 * @return           kOk on success, error code on failure
 */
MmResult file_getcwd(char *buf, size_t size);

/**
 * Gets information about a file.
 */
MmResult file_info(const char *filename, FileInfo *info);

/**
 * Does the path exist and correspond to a symbolic link?
 *
 * @param[in]  path  Path to check
 * @return           true if path exists and corresponds to a symbolic link
 */
bool file_exists_symlink(const char *path);

/**
 * Gets sorted list of files matching a specification.
 *
 * @param[in]  fspec  File/path specification, e.g.
 *                      *        find all entries
 *                      *.txt    find all entries with an extension of .txt
 *                      E*.*     find all entries starting with E
 *                      x?x.*    find all three letter file names starting and ending with x
 *                      mydir/ * find all entries in directory mydir
 * @param[in]  sort   Sort order
 * @param[out] list   Pointer to store the file list
 */
MmResult file_list(const char *fspec, FileSort sort, FileList *list);

/**
 * Gets the current file position (1-based).
 * For serial ports, returns the number of bytes in the receive queue.
 *
 * @param[in]  fnbr  File number
 * @return           Current position (1-based), or -1 on error
 */
int file_loc(int fnbr);

/**
 * Gets the length of file in bytes.
 * For serial ports, always returns 0 (unbuffered).
 *
 * @param[in]  fnbr  File number
 * @return           File length in bytes, or -1 on error
 */
int file_lof(int fnbr);

/**
 * Creates a new directory
 *
 * @param[in]  dirname  Path to the directory to create
 * @return              kOk on success, error code on failure
 */
MmResult file_mkdir(const char *dirname);

/**
 * Creates a new empty file
 *
 * @param[in]  filename  Path to the file to create
 * @return               kOk on success, error code on failure
 */
MmResult file_mkfile(const char *filename);

/**
 * Opens a directory for reading.
 *
 * @param[in]  dirname  Path to the directory to open
 * @param[out] stream   Pointer to store the directory stream handle
 * @return              kOk on success, error code on failure
 */
MmResult file_opendir(const char *dirname, DirStream **stream);

/**
 * Reads the next entry from a directory stream.
 *
 * @param[in]  stream  Directory stream to read from
 * @param[out] entry   Pointer to store the directory entry (NULL if end of directory)
 * @return             kOk on success, error code on failure
 */
MmResult file_readdir(DirStream *stream, DirEntry **entry);

/**
 * Reads the target of a symbolic link.
 *
 * @param[in]    path     Path to the symbolic link
 * @param[out]   buf      Buffer to store the link target
 * @param[in,out] bufsiz  Input: buffer size, Output: actual bytes read
 * @return                kOk on success, error code on failure
 */
MmResult file_readlink(const char *path, char *buf, size_t *bufsiz);

/**
 * Renames a file or directory.
 *
 * @param[in]  old_filename  Current name/path
 * @param[in]  new_filename  New name/path
 * @return                   kOk on success, error code on failure
 */
MmResult file_rename(const char *old_filename, const char *new_filename);

/**
 * Removes an empty directory.
 *
 * @param[in]  dirname  Path to the directory to remove
 * @return              kOk on success, error code on failure
 */
MmResult file_rmdir(const char *dirname);

#endif // #if !defined(MMB4L_FILE)
