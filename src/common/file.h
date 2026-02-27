/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file.h

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#ifdef _WIN32
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 260
#define NAME_MAX 255
#endif
#else
#include <linux/limits.h>
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

#include "mmresult.h"

// Define the path separator based on the platform
#ifdef _WIN32
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"
#else
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
#endif

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
 * Appends a path element to a parent path with proper separator handling.
 *
 * @param parent    The base path to append to (modified in place)
 * @param element   The path element to append
 * @param size      Size of the parent buffer
 * @return          kOk on success, error code on failure
 */
MmResult file_append_path(char *parent, const char *element, size_t size);

/**
 * Closes an open file handle.
 *
 * @param fnbr  The file number/handle to close
 * @return      kOk on success, error code on failure
 */
MmResult file_close(int fnbr);

/**
 * Checks if a named regular file exists in the filesystem.
 *
 * @param[in]  filename  Path to the file to check
 * @return               true if file exists and is a regular file, false otherwise
 */
bool file_exists_regular(const char *path);

/**
 * Checks if a named directory exists in the filesystem.
 *
 * @param[in]  path  Path to the directory to check
 * @return           true if file exists and is a directory, false otherwise
 */
bool file_exists_dir(const char *path);

/**
 * Gets the directory to store use-specific application configuration.
 *
 * @param[out] buf   Buffer to store the directory path
 * @param[in]  size  Size of the buffer
 * @return           kOk on success, error code on failure
 *
 * @note Exposed as a function pointer so it can be mocked in unit-tests.
 */
extern MmResult (*file_get_config_dir)(char *buf, size_t size);

/**
 * Gets the current user's home directory.
 *
 * @param[out] buf   Buffer to store the directory path
 * @param[in]  size  Size of the buffer
 * @return           kOk on success, error code on failure
 */
MmResult file_get_home(char *buf, size_t size);

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
 * Extracts the final component of a path, equivalent to POSIX basename().
 *
 * Special cases match POSIX behaviour:
 *   - Empty string or NULL path produces "."
 *   - A path of "/" produces "/"
 *   - Trailing slashes are ignored
 *
 * @param[in]  path    Null-terminated path string.
 * @param[out] buf     Buffer to receive the null-terminated basename.
 * @param[in]  buf_sz  Size of buf in bytes.
 * @return             kOk on success, error code on failure
 */
MmResult file_basename(const char *path, char *buf, size_t buf_sz);

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
 * Compares two file paths for equality, accounting for platform-specific
 * case sensitivity and path normalization.
 *
 * @param[in]  path1  First file path to compare
 * @param[in]  path2  Second file path to compare
 * @return            true if the paths refer to the same location, false otherwise
 */
bool file_compare_path(const char *path1, const char *path2);

/**
 * Deletes a file from the filesystem.
 *
 * @param[in]  filename  Path to the file to delete
 * @return               kOk on success, error code on failure
 */
MmResult file_delete(const char *filename);

/**
 * Extracts the directory component of a path, equivalent to POSIX dirname().
 *
 * Special cases match POSIX behaviour:
 *   - Empty string or NULL path produces "."
 *   - A path of "/" produces "/"
 *   - Trailing slashes are ignored
 *   - A path with no directory component produces "."
 *
 * @param[in]  path    Null-terminated path string.
 * @param[out] buf     Buffer to receive the null-terminated dirname.
 * @param[in]  buf_sz  Size of buf in bytes.
 *
 * @return  kOk              on success.
 *          kInternalFault   if path or buf is NULL.
 *          kFilenameTooLong if the dirname exceeds buf_sz - 1 characters.
 */
MmResult file_dirname(const char *path, char *buf, size_t buf_sz);

/**
 * Tests whether a string matches a wildcard pattern, equivalent to POSIX fnmatch().
 *
 * Supports the following pattern elements:
 *   - '*'  matches any sequence of characters including empty
 *   - '?'  matches any single character
 *   - '['  introduces a character class, e.g. [abc] or [a-z]
 *
 * @param[in]  pattern  Null-terminated wildcard pattern string.
 * @param[in]  str      Null-terminated string to test.
 * @param[out] match    Set to true if str matches pattern, false otherwise.
 *
 * @return  kOk            on success.
 *          kInternalFault if pattern, str, or match is NULL.
 */
MmResult file_fnmatch(const char *pattern, const char *str, bool *match);

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
 * Checks if the end of a file has been reached.
 *
 * @param[in]  fnbr  File number to check
 * @return           true if end of file has been reached, false otherwise
 */
int file_eof(int fnbr);

/**
 * Does the path exist and correspond to a symbolic link?
 *
 * @param[in]  path  Path to check
 * @return           true if path exists and corresponds to a symbolic link
 */
bool file_exists_symlink(const char *path);

/**
 * Flushes a file's output buffer.
 *
 * @param[in]  fnbr  File number to flush
 * @return           kOk on success, error code on failure
 */
MmResult file_flush(int fnbr);

/**
 * Flushes a file's in-memory state to the underlying storage device.
 *
 * Wraps the POSIX fsync() call, which ensures that all modified data and
 * metadata for the file descriptor have been written to the device.
 *
 * @param[in]  fd  File descriptor to sync
 * @return         0 on success, -1 on failure (errno set appropriately)
 */
int file_fsync(int fd);

/**
 * Checks if a path is absolute.
 *
 * @param[in]  path  Path to check
 * @return           true if the path is absolute, false otherwise
 */
bool file_is_absolute(const char *path);

/**
 * Checks if a character is a path separator either '/' or '\'.
 *
 * @param[in]  c   Character to check
 * @return         true if the character is a path separator, false otherwise
 */
static inline bool file_is_separator(char c) {
    return c == '/' || c == '\\';
}

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
 * @param[in]  contents  Initial contents of the file (can be NULL for empty file)
 * @return               kOk on success, error code on failure
 */
MmResult file_mkfile(const char *filename, const char *contents);

/**
 * Creates a new symbolic link
 *
 * @param[in]  target  Path to the target of the symbolic link
 * @param[in]  link    Path to the symbolic link to create
 * @return             kOk on success, error code on failure
 */
MmResult file_mksymlink(const char *target, const char *link);

/**
 * Normalizes path separators in a path string to the UNIX path separator.
 *
 * This function takes an input path string and replaces all occurrences of
 * both '/' and '\' with the UNIX path separator.
 *
 * @param[in]  path      Input path string to normalize
 * @param[out] buf       Buffer to store the normalized path
 * @param[in]  buf_sz    Size of the output buffer in bytes
 * @return               kOk on success, error code on failure
 */
MmResult file_normalize_separators(const char *path, char *buf, size_t buf_sz);

/**
 * Opens a file for reading or writing.
 *
 * @param[in]  path   Path to the file to open
 * @param[in]  mode   File open mode (e.g., "r", "w", "a", etc.)
 * @param[in]  fnbr   File number to associate with the opened file
 * @return            kOk on success, error code on failure
 */
MmResult file_open(const char *path, const char *mode, int fnbr);

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

/**
 * Gets a character from a file.
 *
 * @param[in]  fnbr  File number to read from
 * @return           The character read as an unsigned char cast to an int,
 *                   -1 on end of file, or longjmp()s on error
 */
int file_getc(int fnbr);

/**
 * Writes a character to a file.
 *
 * @param[in]  fnbr  File number to write to
 * @param[in]  ch    Character to write
 * @return           The character written as an unsigned char cast to an int,
 *                   or longjmp()s on error
 */
int file_putc(int fnbr, char ch);

/**
 * Reads data from a file into a buffer.
 *
 * @param[in]  fnbr    File number to read from
 * @param[out] buf     Buffer to store the read data
 * @param[in]  buf_sz  Number of bytes to read
 * @return             Number of bytes actually read, or longjmp()s on error
 */
size_t file_read(int fnbr, char *buf, size_t buf_sz);

/**
 * Writes data from a buffer to a file.
 *
 * @param[in]  fnbr    File number to write to
 * @param[in]  buf     Buffer containing the data to write
 * @param[in]  buf_sz  Number of bytes to write
 * @return             Number of bytes actually written, or longjmp()s on error
 */
size_t file_write(int fnbr, const char *buf, size_t buf_sz);

#endif // #if !defined(MMB4L_FILE)
