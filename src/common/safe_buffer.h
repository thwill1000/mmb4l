/*
 * Copyright (c) 2024-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(MMB4L_SAFE_BUFFER_H)
#define MMB4L_SAFE_BUFFER_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "utility.h"

/**
 * @brief Provides a wrapper around a byte buffer, which provided it is only
 * mutated via the safe_buffer_xxx() functions should never overrun.
 */
typedef struct {
   char *base;      /**< Pointer to the beginning of the buffer */
   char *end;       /**< Pointer to the last byte + 1 written to the buffer */
   char *limit;     /**< Pointer to the last valid byte + 1 in the buffer */
   char *pos;       /**< Current position pointer for read/write operations */
   bool overrun;    /**< Set true after an operation that would cause the buffer
                         to overrun */
} SafeBuffer;

static inline void safe_buffer_reset(SafeBuffer *sb);

/**
 * @brief Initialises a SafeBuffer.
 *
 * Sets up the SafeBuffer structure with the provided raw buffer and calculates
 * the limit pointer based on the buffer size.
 *
 * @param[out] sb      Pointer to the SafeBuffer to initialize
 * @param[in]  raw     Pointer to the buffer to guard
 * @param[in]  raw_sz  Size of `raw` in bytes
 */
static inline void safe_buffer_init(SafeBuffer *sb, char *raw, size_t raw_sz) {
   sb->base = raw;
   sb->end = sb->base;
   sb->limit = raw + raw_sz;
   safe_buffer_reset(sb);
}

/**
 * @brief Resets a SafeBuffer's append position and overrun flag.
 *
 * Resets the current position pointer to the beginning of the buffer and
 * clears the overrun flag, allowing the buffer to be reused.
 *
 * @param[in,out] sb Pointer to the SafeBuffer to reset
 */
static inline void safe_buffer_reset(SafeBuffer *sb) {
    sb->pos = sb->base;
    sb->overrun = false;
}

/**
 * @brief Reads up to `count` bytes from a SafeBuffer.
 *
 * Reads bytes from the current position in the buffer, advancing the position
 * pointer. If there are insufficient bytes available, reads as many as possible
 * and sets the overrun flag.
 *
 * @param[in,out] sb     Pointer to the SafeBuffer
 * @param[out]    dst    Buffer to store the read bytes
 * @param[in]     count  Number of bytes to read
 * @return               The number of bytes actually read.
 *                       If this is < `count` then the `overrun` flag will also
 *                       have been set.
 */
static inline size_t safe_buffer_read(SafeBuffer *sb, char *dst, size_t count) {
    if (sb->overrun) return 0;

    const size_t available = sb->end - sb->pos;
    const size_t n = (count < available) ? count : available;
    memcpy(dst, sb->pos, n);
    sb->pos += n;
    if (n < count) sb->overrun = true;
    return n;
}

/**
 * @brief Reads up to `count` objects from a SafeBuffer.
 *
 * Reads objects of a specified size from the current position in the buffer.
 * This is a convenience wrapper around safe_buffer_read() for structured data.
 *
 * @param[in,out] sb     Pointer to the SafeBuffer
 * @param[out]    dst    Buffer to store the read objects
 * @param[in]     size   Size of each object in bytes
 * @param[in]     count  Number of objects to read
 * @return               The number of objects actually read.
 *                       If this is < `count` then the `overrun` flag will also
 *                       have been set and a partial object may have been read.
 */
static inline size_t safe_buffer_read_objects(SafeBuffer *sb, void *dst,
                                              size_t size, size_t count) {
    return safe_buffer_read(sb, (char *) dst, size * count) / size;
}

/**
 * @brief Writes up to `count` bytes to a SafeBuffer.
 *
 * Writes bytes to the current position in the buffer, advancing the position
 * pointer. If there is insufficient space available, writes as many bytes as
 * possible and sets the overrun flag.
 *
 * @param[in,out] sb     Pointer to the SafeBuffer
 * @param[in]     src    Buffer containing the bytes to write
 * @param[in]     count  Number of bytes to write
 * @return               The number of bytes actually written.
 *                       If this is < `count` then the `overrun` flag will also
 *                       have been set.
 */
static inline size_t safe_buffer_write(SafeBuffer *sb, const char *src,
                                       size_t count) {
    if (sb->overrun) return 0;
    const size_t free_space = sb->limit - sb->pos;
    const size_t n = (count < free_space) ? count : free_space;
    memcpy(sb->pos, src, n);
    sb->pos += n;
    sb->end = sb->pos;
    if (n < count) sb->overrun = true;
    return n;
}

/**
 * @brief Writes up to `count` objects to a SafeBuffer.
 *
 * Writes objects of a specified size to the current position in the buffer.
 * This is a convenience wrapper around safe_buffer_write() for structured data.
 *
 * @param[in,out] sb     Pointer to the SafeBuffer
 * @param[in]     src    Buffer containing the objects to write
 * @param[in]     size   Size of each object in bytes
 * @param[in]     count  Number of objects to write
 * @return               The number of objects actually written.
 *                       If this is < `count` then the `overrun` flag will also
 *                       have been set and a partial object may have been
 *                       written.
 */
static inline size_t safe_buffer_write_objects(SafeBuffer *sb, const void *src,
                                               size_t size, size_t count) {
    return safe_buffer_write(sb, (const char *) src, size * count) / size;
}

/**
 * @brief Appends a single character to a SafeBuffer.
 *
 * Appends one character to the current position in the buffer. If the buffer
 * would overrun, the overrun flag is set instead.
 *
 * @param[in,out] sb  Pointer to the SafeBuffer
 * @param[in]     c   Character to append
 * @return            0 on success,
 *                   -1 if the buffer has already overrun, or would overrun
 */
static inline int safe_buffer_append(SafeBuffer *sb, char c) {
    return safe_buffer_write_objects(sb, &c, 1, 1) == 1 ? 0 : -1;
}

/**
 * @brief Appends multiple bytes to a SafeBuffer.
 *
 * Appends the specified number of bytes to the current position in the buffer.
 * If the buffer would overrun, appends as many bytes as possible and sets the
 * overrun flag.
 *
 * @param[in,out] sb      Pointer to the SafeBuffer
 * @param[in]     buf     Buffer containing bytes to append
 * @param[in]     buf_sz  Number of bytes to append
 * @return                0 on success,
 *                       -1 if the buffer has already overrun, or would overrun
 */
static inline int safe_buffer_append_bytes(SafeBuffer *sb, const char *buf,
                                           size_t buf_sz) {
    return safe_buffer_write(sb, buf, buf_sz) == buf_sz ? 0 : -1;
}

/**
 * @brief Appends a C-string including trailing null terminator to a SafeBuffer.
 *
 * Appends a null-terminated string to the current position in the buffer,
 * including the null terminator. If the buffer would overrun, appends as many
 * bytes as possible and sets the overrun flag.
 *
 * @param[in,out] sb  Pointer to the SafeBuffer
 * @param[in]     s   Null-terminated string to append
 * @return            0 on success,
 *                   -1 if the buffer has already overrun, or would overrun
 */
static inline int safe_buffer_append_string(SafeBuffer *sb, const char *s) {
    const size_t count = strlen(s) + 1;
    return safe_buffer_write(sb, s, count) == count ? 0 : -1;
}

/**
 * @brief Checks if the SafeBuffer is full.
 *
 * Determines whether the current position pointer has reached the buffer's
 * limit, indicating that the buffer is full.
 *
 * @param[in] sb Pointer to the SafeBuffer to check
 * @return       true if the buffer is full, false otherwise
 */
static inline bool safe_buffer_is_full(SafeBuffer *sb) {
    return sb->end == sb->limit;
}

/**
 * @brief Increments the SafeBuffer's current position pointer.
 *
 * Moves the current position pointer by the specified number of bytes AND
 * clears the `overrun` flag. The increment can be negative to move backwards in
 * the buffer. Validates that the new position remains within the buffer bounds.
 *
 * @param[in,out] sb        Pointer to the SafeBuffer
 * @param[in]     increment Number of bytes to increment (may be negative)
 * @return                  0 on success,
 *                         -1 if the increment would take the position pointer
 *                          outside the buffer's bounds
 */
static inline int safe_buffer_inc_pos(SafeBuffer *sb, int increment) {
    if (sb->pos + increment < sb->base || sb->pos + increment > sb->limit) {
        return -1;
    } else {
        sb->pos += increment;
        sb->overrun = false;
        return 0;
    }
}

/**
 * @brief Sets the SafeBuffer's current position pointer to a specific location.
 *
 * Directly sets the current position pointer to the specified location within
 * the buffer AND clears the `overrun` flag. Validates that the new position is
 * within the buffer bounds.
 *
 * @param[in,out] sb  Pointer to the SafeBuffer
 * @param[in]     ptr New position pointer value
 * @return            0 on success,
 *                   -1 if `pos` is outside the buffer's bounds
 */
static inline int safe_buffer_set_pos(SafeBuffer *sb, char *pos) {
    if (pos < sb->base || pos > sb->limit) {
        return -1;
    } else {
        sb->pos = pos;
        sb->overrun = false;
        return 0;
    }
}

#endif // #if !defined(MMB4L_SAFE_BUFFER_H)
