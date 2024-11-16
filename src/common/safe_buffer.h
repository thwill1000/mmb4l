/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(MMB4L_SAFE_BUFFER_H)
#define MMB4L_SAFE_BUFFER_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "utility.h"

/**
 * Provides a wrapper around a byte buffer, which provided it is only mutated via the
 * safe_buffer_xxx() functions should never overrun.
 */
typedef struct {
   char *base;
   char *limit;
   char *ptr;
   bool overrun;  // Set true after an operation that would cause the buffer to overrun.
} SafeBuffer;

static inline void safe_buffer_reset(SafeBuffer *b);

/**
 * Initialises a SafeBuffer.
 *
 * @param  raw     Pointer to the buffer to guard.
 * @param  raw_sz  Size of \p buf in bytes.
 */
static inline void safe_buffer_init(SafeBuffer *b, char *raw, size_t raw_sz) {
   b->base = raw;
   b->limit = raw + raw_sz - 1;
   safe_buffer_reset(b);
}

/** Resets a SafeBuffer's append pointer and overrun flag. */
static inline void safe_buffer_reset(SafeBuffer *b) {
    b->ptr = b->base;
    b->overrun = false;
}

/**
 * Appends a char to a SafeBuffer.
 * If the buffer would overrun then instead sets the 'overrun' field.
 *
 * @param  b  Pointer to the SafeBuffer.
 * @param  c  Character to append.
 * @return     0 on success,
 *            -1 if the buffer has already has, or would overrun.
 */
static inline int safe_buffer_append(SafeBuffer *b, char c) {
    if (b->overrun) {
        return -1;
    } else if (b->ptr > b->limit) {
        b->overrun = true;
        return -1;
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
       *b->ptr++ = c;
#pragma GCC diagnostic pop
        return 0;
    }
}

/**
 * Appends \p sz bytes to a SafeBuffer.
 * If the buffer would overrun then appends as many bytes as possible and sets the 'overrun'
 * field.
 *
 * @param  b       Pointer to the SafeBuffer.
 * @param  buf     Bytes to append.
 * @param  buf_sz  Number of bytes to append.
 * @return          0 on success,
 *                 -1 if the buffer has already has, or would overrun.
 */
static inline int safe_buffer_append_bytes(SafeBuffer *b, const char *buf, size_t buf_sz) {
    for (size_t i = 0; i < buf_sz; ++i) {
        (void) safe_buffer_append(b, buf[i]);
    }
    return b->overrun ? -1 : 0;
}

/**
 * Appends a C-string including trailing '\0' to a SafeBuffer.
 * If the buffer would overrun then appends as many bytes as possible and sets the 'overrun'
 * field.
 *
 * @param  b  Pointer to the SafeBuffer.
 * @param  s  String to append.
 * @return     0 on success,
 *            -1 if the buffer has already has, or would overrun.
 */
static inline int safe_buffer_append_string(SafeBuffer *b, const char *s) {
    for (const char *p = s; *p; ++p) {
        (void) safe_buffer_append(b, *p);
    }
    (void) safe_buffer_append(b, '\0');
    return b->overrun ? -1 : 0;
}

/** Is the SafeBuffer full? */
static inline bool safe_buffer_is_full(SafeBuffer *b) {
    return b->ptr > b->limit;
}

/**
 * Increments the SafeBuffer's append pointer.
 *
 * @param  b          Pointer to the SafeBuffer.
 * @param  increment  Number of bytes to increment, may be negative.
 * @return             0 on success,
 *                    -1 if the buffer has already overrun or the increment would take the append
 *                    pointer out of the buffer's bounds.
 */
static inline int safe_buffer_inc_ptr(SafeBuffer *b, int increment) {
    if (b->overrun || b->ptr + increment < b->base || b->ptr + increment > b->limit) {
        return -1;
    } else {
        b->ptr += increment;
        return 0;
    }
}

/**
 * Sets the SafeBuffer's append pointer.
 *
 * @param  b    Pointer to the SafeBuffer.
 * @param  ptr  New pointer value.
 * @return       0 on success,
 *              -1 if the buffer has already overrun or \p ptr is out of the buffer's bounds.
 */
static inline int safe_buffer_set_ptr(SafeBuffer *b, char *ptr) {
    if (b->overrun || ptr < b->base || ptr > b->limit) {
        return -1;
    } else {
        b->ptr = ptr;
        return 0;
    }
}

/**
 * Gets the last byte appended to the SafeBuffer.
 *
 * @param  b  Pointer to the SafeBuffer
 * @return    The last byte appended,
 *            or -1 if the buffer has overrun or no bytes have been appended.
 */
static inline int safe_buffer_last(SafeBuffer *b) {
    assert(!CHAR_IS_SIGNED);
    if (b->overrun || b->ptr == b->base) return -1;
    return *(b->ptr - 1);
}

#endif // #if !defined(MMB4L_SAFE_BUFFER_H)
