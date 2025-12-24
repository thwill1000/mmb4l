/*-*****************************************************************************

MMBasic for Linux (MMB4L)

streamio.h

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

#if !defined(MMB4L_STREAMIO_H)
#define MMB4L_STREAMIO_H

#include <stdbool.h>
#include <stddef.h>

#include "mmresult.h"

/**
 * Initialises the 'streamio' module.
 * Sets up function pointers for console I/O operations.
 *
 * @param[in]  putc_fn   Function pointer for outputting a single character to console
 * @param[in]  write_fn  Function pointer for writing a buffer of characters to console
 * @return               kOk on success, error code on failure
 */
MmResult streamio_init(MmResult (*putc_fn)(char),
                       MmResult (*write_fn)(const char *, size_t *));

/**
 * Closes an open I/O device.
 *
 * @param[in]  fnbr  File number to close
 * @return           kOk on success, error code on failure
 */
MmResult streamio_close(int fnbr);

/**
 * Closes all open I/O devices.
 * Used for cleanup operations, typically on program exit.
 */
MmResult streamio_close_all(void);

/**
 * Checks if end-of-file has been reached.
 *
 * @param[in]  fnbr  File number to check
 * @return           1 if at EOF, 0 if not at EOF, 0 on error
 */
int streamio_eof(int fnbr);

/**
 * Finds the first available free I/O device number.
 * Scans the file table to locate an unused slot.
 *
 * @return  Available file number (1-MAXOPENFILES), or -1 if none available
 */
int streamio_find_free(void);

/**
 * Reads a single character from an I/O stream.
 *
 * @param[in]  fnbr  File number to read from (0 for console input)
 * @return           Character read (0-255), or -1 on EOF/error
 */
int streamio_getc(int fnbr);

/**
 * Checks if a file number refers to a regular file.
 *
 * @param[in]  fnbr  File number to check
 * @return           true if it's a regular file, false otherwise
 */
bool streamio_is_file(int fnbr);

/**
 * Checks if a file number refers to a serial port.
 *
 * @param[in]  fnbr  File number to check
 * @return           true if it's a serial port, false otherwise
 */
bool streamio_is_serial(int fnbr);

/**
 * Gets the current I/O stream position (1-based).
 * For serial ports, returns the number of bytes in the receive queue.
 *
 * @param[in]  fnbr  File number
 * @return           Current position (1-based), or -1 on error
 */
int streamio_loc(int fnbr);

/**
 * Gets the length of I/O stream in bytes.
 * For serial ports, always returns 0 (unbuffered).
 *
 * @param[in]  fnbr  File number
 * @return           File length in bytes, or -1 on error
 */
int streamio_lof(int fnbr);

/**
 * Opens an I/O device (e.g. file) with the specified mode.
 *
 * @param[in]  path  Path to the device to open
 * @param[in]  mode  File open mode (e.g., "r", "w", "a", "r+", "w+", "x")
 * @param[in]  fnbr  File number to assign (1-MAXOPENFILES)
 * @return           kOk on success, error code on failure
 */
MmResult streamio_open(const char *path, const char *mode, int fnbr);

/**
 * Writes a single character to an I/O stream.
 *
 * @param[in]  fnbr  File number to write to (0 for console output)
 * @param[in]  ch    Character to write
 * @return           Character written, or -1 on error
 */
int streamio_putc(int fnbr, int ch);

/**
 * Reads data from an input stream into a buffer.
 *
 * @param[in]  fnbr  File number to read from
 * @param[out] buf   Buffer to store the data
 * @param[in]  sz    Number of bytes to read
 * @return           Number of bytes actually read
 */
size_t streamio_read(int fnbr, char *buf, size_t sz);

/**
 * Reads a line from an input stream.
 *
 * Reads characters from the specified stream until a line terminator is
 * encountered, the buffer is full, or EOF/error occurs. Line terminators
 * (\r, \n, or \r\n) are consumed but not included in the returned string.
 * The string is always null-terminated.
 *
 * @param fnbr  File number to read from.
 * @param buf   Buffer to store the line. Must not be NULL.
 * @param sz    Size of buffer in bytes, including space for null terminator.
 * @return      kOk on success, or an error code on failure.
 *
 * @note Recognizes three line ending styles: \n (Unix), \r (old Mac), and
 *       \r\n (Windows/DOS). The \r\n sequence is treated as a single line
 *       terminator.
 * @note If the line exceeds sz-1 characters, reading stops and the string is
 *       truncated. The next call will continue reading from where it left off.
 * @note At EOF with no data read, returns an empty string (buf[0] = '\0').
 * @note Line terminators are consumed from the stream but not included in buf.
 */
MmResult streamio_readln(int fnbr, char *buf, size_t sz);

/**
 * Seeks to a specific position in an I/O stream.
 * Position is 1-based (MMBasic convention).
 *
 * @param[in]  fnbr  File number
 * @param[in]  idx   Position to seek to (1-based)
 */
void streamio_seek(int fnbr, int idx);

/**
 * Pushes a character back onto an input stream.
 *
 * Allows one character to be pushed back to be read again by the next call to
 * streamio_getc(). This is primarily useful for lookahead operations.
 *
 * @param fnbr  File number to read from.
 * @param ch    Character to push back onto the stream.
 * @return      kOk on success, or an error code on failure.
 *
 * @note Only one character of pushback is guaranteed. Multiple calls without
 *       an intervening read may fail.
 * @note Not supported for serial ports or console input (fnbr 0).
 * @note The character pushed back doesn't need to be the same as the one
 *       previously read, but for portability it's recommended to only push
 *       back the last character read.
 */
MmResult streamio_ungetc(int fnbr, int ch);

/**
 * Writes data from a buffer to an I/O stream.
 *
 * @param[in]  fnbr  File number to write to (0 for console output)
 * @param[in]  buf   Buffer containing data to write
 * @param[in]  sz    Number of bytes to write
 * @return           Number of bytes actually written
 */
size_t streamio_write(int fnbr, const char *buf, size_t sz);

#endif // #if !defined(MMB4L_STREAMIO_H)
