/*-*****************************************************************************

MMBasic for Linux (MMB4L)

logger.h

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

#if !defined(MMB4L_LOGGER_H)
#define MMB4L_LOGGER_H

#include "mmresult.h"

typedef enum {
    kLoggerLevelDebug = 0,
    kLoggerLevelInfo,
    kLoggerLevelWarning,
    kLoggerLevelError,
    kLoggerLevelFatal
} LoggerLevel;

/**
 * Initialises the logger.
 *
 * @param filename The name of the log file to append to.
 *                 If NULL or an empty string, logs will be written to stdout.
 */
MmResult logger_init(const char *filename);

/** Terminates the logger. */
MmResult logger_term(void);

/** Writes a message to the log. */
void logger_write(LoggerLevel level, const char *file, unsigned line, const char *format, ...);

#define LOG_INFO(...)   logger_write(kLoggerLevelInfo, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)   logger_write(kLoggerLevelWarning, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)  logger_write(kLoggerLevelError, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...)  logger_write(kLoggerLevelFatal, __FILE__, __LINE__, __VA_ARGS__)

#if defined(NDEBUG)
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...)  logger_write(kLoggerLevelDebug, __FILE__, __LINE__, __VA_ARGS__)
#endif // NDEBUG

#define LOG_FN_ENTRY(fmt, ...) \
    LOG_INFO("Entering %s() at %s:%d - " fmt, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_FN_EXIT(fmt, ...) \
    LOG_INFO("Exiting %s() at %s:%d - " fmt, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

#endif // MMB4L_LOGGER_H
