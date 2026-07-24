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

#include <stdbool.h>
#include <stddef.h>

#include "mmresult.h"

typedef enum {
    kLoggerLevelUninitialised = 0,
    kLoggerLevelDebug,
    kLoggerLevelInfo,
    kLoggerLevelWarning,
    kLoggerLevelError,
    kLoggerLevelFatal,
    kLoggerLevelNone
} LoggerLevel;

#if defined(NDEBUG)
#define LOGGER_DEFAULT_LEVEL  kLoggerLevelNone
#else
#define LOGGER_DEFAULT_LEVEL  kLoggerLevelInfo
#endif

/**
 * Minimum level a message must have to be emitted; messages below this level
 * are cheaply filtered by the LOG_* macros before their arguments are even
 * evaluated, so this is checked outside of logger_write() as well as inside it.
 *
 * Set via logger_set_min_level(), normally from the command line (-l/--log)
 * or the "Log" option once options have been loaded.
 *
 * Not synchronised: reads/writes from multiple threads are not atomic. In
 * practice it is written once early in startup (and occasionally thereafter
 * from the main thread via OPTION LOG), so a torn or stale read by another
 * thread could at worst cause a message to be wrongly included/excluded for
 * one call - not a safety issue, just a note for anyone tempted to rely on
 * it more strictly.
 */
extern LoggerLevel logger_min_level;

static const bool logger_in_function = false;

LoggerLevel logger_level_from_string(const char *s);

MmResult logger_level_as_string(LoggerLevel level, char *buf, size_t buf_sz);

/**
 * Initialises the logger.
 *
 * @param filename The name of the log file to append to.
 *                 If NULL or an empty string, logs will be written to stdout.
 */
MmResult logger_init(const char *filename);

/** Terminates the logger. */
MmResult logger_term(void);

/** Sets the minimum log level emitted at runtime. */
MmResult logger_set_min_level(LoggerLevel level);

/** Writes a message to the log. */
void logger_write(LoggerLevel level, const char *file, unsigned line, const char *function,
                  const char *format, ...);

/**
 * Escapes string/byte data for log output using mixed mode:
 * printable ASCII bytes (0x20-0x7E) are emitted as-is,
 * other bytes are emitted as [NN].
 *
 * Uses an internal 1024-byte static buffer for output.
 *
 * @param src      Source bytes/string.
 * @param src_len  Number of bytes to read, or -1 to treat src as a C-string.
 * @return         Pointer to internal static buffer containing escaped text.
 */
const char *logger_fmt_string(const unsigned char *src, ptrdiff_t src_len);

/**
 * Formats a C-string (NUL terminated) using mixed escaping.
 *
 * @return Pointer to internal static buffer containing escaped text.
 */
#define FMT_CSTRING(src) \
    logger_fmt_string((const unsigned char *)(src), -1)

/**
 * Formats a Pascal/MMBasic string (length in first byte) using mixed escaping.
 *
 * @return Pointer to internal static buffer containing escaped text.
 */
static inline const char *logger_fmt_pstring(const unsigned char *src) {
    return logger_fmt_string(src ? src + 1 : NULL, src ? (ptrdiff_t)src[0] : 0);
}

static inline bool logger_will_log(LoggerLevel level) {
    return level >= logger_min_level;
}

#define FMT_PSTRING(src) \
    logger_fmt_pstring((const unsigned char *)(src))

#define LOG_DEBUG(...) do { \
    if (logger_will_log(kLoggerLevelDebug)) { \
        logger_write(kLoggerLevelDebug, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } \
} while (0)

#define LOG_INFO(...) do { \
    if (logger_will_log(kLoggerLevelInfo)) { \
        logger_write(kLoggerLevelInfo, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } \
} while (0)

#define LOG_WARN(...) do { \
    if (logger_will_log(kLoggerLevelWarning)) { \
        logger_write(kLoggerLevelWarning, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } \
} while (0)

#define LOG_ERROR(...) do { \
    if (logger_will_log(kLoggerLevelError)) { \
        logger_write(kLoggerLevelError, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } \
} while (0)

#define LOG_FATAL(...) do { \
    if (logger_will_log(kLoggerLevelFatal)) { \
        logger_write(kLoggerLevelFatal, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } \
} while (0)

#define LOG_FN_ENTRY(fmt, ...) \
    const bool logger_in_function = true; \
    (void) logger_in_function; \
    LOG_DEBUG("called (" fmt ")", ##__VA_ARGS__)

#define LOG_FN_EXIT(fmt, ...) do { \
    if (logger_in_function) { \
        LOG_DEBUG("return (" fmt ")", ##__VA_ARGS__); \
    } \
} while (0)

#endif // MMB4L_LOGGER_H
