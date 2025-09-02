/*-*****************************************************************************

MMBasic for Linux (MMB4L)

iodevice.h

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

#if !defined(MMB4L_IODEVICE_H)
#define MMB4L_IODEVICE_H

#include "mmresult.h"

/**
 * Closes an open I/O device.
 *
 * @param[in]  fnbr  File number to close
 * @return           kOk on success, error code on failure
 */
MmResult iodevice_close(int fnbr);

/**
 * Closes all open I/O devices.
 * Used for cleanup operations, typically on program exit.
 */
void iodevice_close_all(void);

/**
 * Finds the first available free I/O device number.
 * Scans the file table to locate an unused slot.
 *
 * @return  Available file number (1-MAXOPENFILES), or -1 if none available
 */
int iodevice_find_free(void);


/**
 * Checks if a file number refers to a regular file.
 *
 * @param[in]  fnbr  File number to check
 * @return           true if it's a regular file, false otherwise
 */
bool iodevice_is_file(int fnbr);

/**
 * Opens an I/O device (e.g. file) with the specified mode.
 *
 * @param[in]  path  Path to the device to open
 * @param[in]  mode  File open mode (e.g., "r", "w", "a", "r+", "w+", "x")
 * @param[in]  fnbr  File number to assign (1-MAXOPENFILES)
 * @return           kOk on success, error code on failure
 */
MmResult iodevice_open(const char *path, const char *mode, int fnbr);

#endif // #if !defined(MMB4L_IODEVICE_H)
