/*-*****************************************************************************

MMBasic for Linux (MMB4L)

logger.c

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

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cstring.h"
#include "file.h"
#include "mmtime.h"

#if defined(__ANDROID__)
#include <android/log.h>
#define LOG_TAG "MMB4A"
#endif

#include "logger.h"

typedef struct {
    const char *name;
    int ordinal;
} NameOrdinalPair;

LoggerLevel logger_min_level = kLoggerLevelNone;

static FILE *logger = NULL;

static const NameOrdinalPair logger_level_map[] = {
    { "Uninitialised", kLoggerLevelUninitialised },
    { "Debug", kLoggerLevelDebug },
    { "Info",  kLoggerLevelInfo },
    { "Warning", kLoggerLevelWarning },
    { "Error", kLoggerLevelError},
    { "Fatal", kLoggerLevelFatal},
    { "None",  kLoggerLevelNone },
    { NULL,    -1 }
};

LoggerLevel logger_level_from_string(const char *s) {
    assert(s != NULL);
    for (const NameOrdinalPair *entry = logger_level_map; entry->name; ++entry) {
        if (cstring_casecmp(s, entry->name) == 0) {
            return entry->ordinal;
        }
    }
    return kLoggerLevelUninitialised;
}

MmResult logger_level_as_string(LoggerLevel level, char *buf, size_t buf_sz) {
    if (level < kLoggerLevelUninitialised || level > kLoggerLevelNone) {
        return mmresult_ex(kInvalidValue, "Invalid logger level: %d", level);
    }
    return cstring_cpy(buf, logger_level_map[level].name, buf_sz) == 0
            ? kOk
            : kStringTooLong;
}

MmResult logger_set_min_level(LoggerLevel level) {
    if (level < kLoggerLevelDebug || level > kLoggerLevelNone) {
        return mmresult_ex(kInvalidValue, "Invalid logger level: %d", level);
    }
    logger_min_level = level;
    return kOk;
}

const char *logger_fmt_string(const unsigned char *src, ptrdiff_t src_len) {
    static const char hex[] = "0123456789abcdef";
    static const char kNull[] = "<null>";
    static char buf[4][1024];       // ring buffer of 4 slots
    static int buf_index = 0;

    buf_index = (buf_index + 1) % 4;
    char *dst = buf[buf_index];
    const size_t dst_size = sizeof(buf[0]);

    if (!src) return kNull;

    if (src_len == 0 || src_len < -1) {
        *dst = '\0';
        return dst;
    }

    char *out = dst;
    const char *const out_end = dst + dst_size - 1;
    const bool is_cstring = (src_len == -1);
    for (ptrdiff_t i = 0; out < out_end; i++) {
        if (!is_cstring && i >= src_len) break;
        const unsigned char ch = src[i];
        if (is_cstring && ch == '\0') break;

        if (ch >= 0x20 && ch <= 0x7E) {
            *out++ = (char) ch;
        } else {
            if (out_end - out < 4) break;
            *out++ = '[';
            *out++ = hex[(ch >> 4) & 0x0F];
            *out++ = hex[ch & 0x0F];
            *out++ = ']';
        }
    }

    *out = '\0';
    return dst;
}

MmResult logger_init(const char *filename) {
    if (filename && filename[0] != '\0') {
        // If the log file already exists, rotate it by renaming it to
        // <base>_<last_modified_datetime>.<ext> before opening a fresh one.
        FileInfo info;
        if (file_info(filename, &info) == kOk && info.exists) {
            struct tm *tm_info = localtime(&info.mtime);
            char ts[20];  // "YYYYMMDD_HHMMSS\0"
            strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", tm_info);

            // Split filename into base and extension, being careful to ignore
            // dots that appear only in directory components of the path.
            const char *dot = strrchr(filename, '.');
            const char *slash = strrchr(filename, PATH_SEPARATOR);
            char rotated[PATH_MAX];
            if (dot && (!slash || dot > slash)) {
                // Has an extension: <base>_<timestamp>.<ext>
                size_t base_len = (size_t)(dot - filename);
                const char *ext = dot + 1;  // skip the '.'
                snprintf(rotated, sizeof(rotated), "%.*s_%s.%s",
                         (int)base_len, filename, ts, ext);
            } else {
                // No extension: <filename>_<timestamp>
                snprintf(rotated, sizeof(rotated), "%s_%s", filename, ts);
            }
            file_rename(filename, rotated);
        }

        // Open a fresh log file.
        logger = fopen(filename, "w");
        if (!logger) {
            return mmresult_ex(errno, "Failed to open log file '%s': %s\n", filename,
                               strerror(errno));
        }
    } else {
        // Use stdout for logging.
        logger = stdout;
    }
    return kOk;
}

MmResult logger_term(void) {
    if (logger && logger != stdout) {
        fclose(logger);
        logger = NULL;
    }
    return kOk;
}

#if defined(__ANDROID__)

void logger_write(LoggerLevel level, const char *file, unsigned line, const char *function,
                  const char *format, ...) {
    if (!logger_will_log(level)) return;

    va_list args;
    va_start(args, format);
    int prio;
    switch (level) {
        case kLoggerLevelDebug:
            prio = ANDROID_LOG_DEBUG;
            break;
        case kLoggerLevelInfo:
            prio = ANDROID_LOG_INFO;
            break;
        case kLoggerLevelWarning:
            prio = ANDROID_LOG_WARN;
            break;
        case kLoggerLevelError:
            prio = ANDROID_LOG_ERROR;
            break;
        case kLoggerLevelFatal:
            prio = ANDROID_LOG_FATAL;
            break;
        default:
            prio = ANDROID_LOG_FATAL;
            break;
    }

    // Get the last element of the file path to avoid printing the full path.
    char filename[NAME_MAX];
    if (file_basename(file, filename, sizeof(filename)) != kOk) {
        cstring_cpy(filename, file, sizeof(filename));
    }
    
    // Format the message with file, line, and function prefix
    char prefix_buffer[256];
    snprintf(prefix_buffer, sizeof(prefix_buffer), "%s:%u:%s  ", filename, line, function);
    
    // Create the full message by concatenating prefix and format
    char full_format[512];
    snprintf(full_format, sizeof(full_format), "%-40s%s", prefix_buffer, format);

    __android_log_vprint(prio, LOG_TAG, full_format, args);
    va_end(args);
}

#else

void logger_write(LoggerLevel level, const char *file, unsigned line, const char *function,
                  const char *format, ...) {
    if (!logger) return;
    if (!logger_will_log(level)) return;

    // Get a timestamp for the log entry.
    time_t now = (time_t) NANOSECONDS_TO_SECONDS(mmtime_now_ns());
    struct tm *tm_info = localtime(&now);
    char time_buffer[26];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(logger, "[%s] ", time_buffer);

    // Get the last element of the file path to avoid printing the full path.
    char filename[NAME_MAX];
    if (file_basename(file, filename, sizeof(filename)) != kOk) {
        cstring_cpy(filename, file, sizeof(filename));
    }

    // Prefix the message with file, line, and function
    char prefix_buffer[NAME_MAX * 2];
    snprintf(prefix_buffer, sizeof(prefix_buffer), "%s:%u:%s  ", filename, line, function);
    fprintf(logger, "%-40s", prefix_buffer);

    switch (level) {
        case kLoggerLevelUninitialised:
            // A message should never actually be logged with this level
            fprintf(logger, "UNINITIALISED:   ");
            break;
        case kLoggerLevelDebug:
            fprintf(logger, "DEBUG:   ");
            break;
        case kLoggerLevelInfo:
            fprintf(logger, "INFO:    ");
            break;
        case kLoggerLevelWarning:
            fprintf(logger, "WARNING: ");
            break;
        case kLoggerLevelError:
            fprintf(logger, "ERROR:   ");
            break;
        case kLoggerLevelFatal:
            fprintf(logger, "FATAL:   ");
            break;
        case kLoggerLevelNone:
            fprintf(logger, "NONE:    ");
            break;
    }
    va_list args;
    va_start(args, format);
    vfprintf(logger, format, args);
    va_end(args);
    fprintf(logger, "\n");
    fflush(logger);
}

#endif
