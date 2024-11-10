/*-*****************************************************************************

MMBasic for Linux (MMB4L)

queue.h

Copyright 2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMB4L_QUEUE_H)
#define MMB4L_QUEUE_H

#include <stdbool.h>

#include "mmresult.h"

typedef void *QueueElement;

typedef struct {
  size_t element_size;  // Size (in bytes) of elements to be held in the queue.
  size_t capacity;      // Maximum number of elements.
  char *storage;        // Storage for the queue, allocated by queue_init(),
                        // deallocated by queue_term()
  char *back;           // Pointer to the insertion point for new elements.
  char *front;          // Pointer to the extraction point for the element at the front of the
                        // queue.
  size_t count;         // Number of elements in the queue.
} Queue;

/**
 * Initialises a Queue structure including allocating storage.
 *
 * @param  q         Pointer to the Queue structure to initialise.
 * @param  type      Element type, e.g. uint8_t, int32_t, void *
 * @param  capacity  Maximum number of elements that the queue should be able to hold.
 */
#define queue_init(q, type, capacity)  queue_init_internal(q, sizeof(type), capacity)

static inline MmResult queue_init_internal(Queue *q, size_t element_size, size_t capacity) {
    q->element_size = element_size;
    q->capacity = capacity;
    q->storage = (char *) malloc(capacity * element_size);
    if (!q->storage) return kOutOfMemory;
    q->back = q->storage;
    q->front = q->storage;
    q->count = 0;
    return kOk;
}

/** Terminates a Queue and deallocates its storage. */
static inline MmResult queue_term(Queue *q) {
    free(q->storage);
    memset(q, 0x0, sizeof(Queue));
    return kOk;
}

/** Removes all elements from a queue. */
static inline void queue_clear(Queue *q) {
    q->back = q->storage;
    q->front = q->storage;
    q->count = 0;
}

/**
 * Adds an element to the end of the queue.
 *
 * @param[in]  q  Pointer to the Queue.
 * @param[in]  e  The element to enque.
 * @return        kContainerFull if the queue is full.
 */
#define queue_enqueue(q, e)  queue_enqueue_internal(q, (QueueElement) &(e))

static inline MmResult queue_enqueue_internal(Queue *q, const QueueElement element) {
    if (q->count == q->capacity) return kContainerFull;
    memcpy(q->back, element, q->element_size);
    q->back += q->element_size;
    if (q->back == q->storage + q->capacity * q->element_size) q->back = q->storage;
    q->count++;
    return kOk;
}

/**
 * Gets an element from the front of the queue.
 *
 * @param[in]   q  Pointer to the Queue.
 * @param[out]  e  On exit contains a shallow copy of the (now removed) element at the front of the
 *                 queue.
 * @return         kContainerEmpty if the queue is empty.
 */
static inline MmResult queue_dequeue(Queue *q, QueueElement e) {
    if (q->count == 0) return kContainerEmpty;
    memcpy(e, q->front, q->element_size);
    q->front += q->element_size;
    if (q->front == q->storage + q->capacity * q->element_size) q->front = q->storage;
    q->count--;
    return kOk;
}

static inline bool queue_is_full(Queue *q) {
    return q->count == q->capacity;
}

static inline size_t queue_size(Queue *q) {
    return q->count;
}

static inline bool queue_is_empty(Queue *q) {
    return q->count == 0;
}

#endif // #if !defined(MMB4L_QUEUE_H)
