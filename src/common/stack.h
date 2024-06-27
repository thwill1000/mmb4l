/*-*****************************************************************************

MMBasic for Linux (MMB4L)

stack.h

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMB4L_STACK_H)
#define MMB4L_STACK_H

#include "mmresult.h"

#include <stdlib.h>

typedef int64_t StackElement;

typedef struct {
  size_t element_size;
  char *storage;
  char *top;   // Insertion point for next element.
  char *limit; // Cannot insert once 'top' reaches this.
} Stack;

/**
 * Initialises a Stack structure including allocating storage.
 *
 * @param  s         Pointer to the Stack structure to initialise.
 * @param  type      Element type, e.g. uint8_t, int32_t, void *
 * @param  capacity  Maximum number of elements that the stack should be able to hold.
 */
#define stack_init(s, type, capacity)  stack_init_internal(s, sizeof(type), capacity)

static MmResult stack_init_internal(Stack *s, size_t element_size, size_t capacity) {
    s->element_size = element_size;
    s->storage = (char *) malloc(capacity * element_size);
    if (!s->storage) return kOutOfMemory;
    s->top = s->storage;
    s->limit = s->storage + capacity * element_size;
    return kOk;
}

/**
 * Terminates a Stack structure freeing its internal storage.
 *
 * @param  s  Pointer to the Stack to terminate.
 */
static MmResult stack_term(Stack *s) {
    free(s->storage);
    s->storage = NULL;
    s->top = NULL;
    return kOk;
}

/**
 * Gets the top element of the stack WITHOUT removing it.
 *
 * @param[in]  s  Pointer to the Stack.
 * @param[out] e  On exit contains the value of the top element.
 * @return        kStackEmpty if the stack is empty.
 */
#define stack_peek(s, e)  stack_peek_internal(s, (StackElement *) (e))

static inline MmResult stack_peek_internal(Stack *s, StackElement *element) {
    if (s->top == s->storage) return kStackEmpty;
    memcpy(element, s->top - s->element_size, s->element_size);
    return kOk;
}

/**
 * Gets the top element of the stack AND removes it.
 *
 * @param[in]  s  Pointer to the Stack.
 * @param[out] e  On exit contains the value of the (now removed) top element.
 * @return        kStackEmpty if the stack is empty.
 */
#define stack_pop(s, e)  stack_pop_internal(s, (StackElement *) (e))

static inline MmResult stack_pop_internal(Stack *s, StackElement *element) {
    if (s->top == s->storage) return kStackEmpty;
    s->top -= s->element_size;
    memcpy(element, s->top, s->element_size);
    return kOk;
}

/**
 * Adds an element to the stop of the stack.
 *
 * @param[in]  s  Pointer to the Stack.
 * @param[in]  e  The element to add.
 * @return        kStackFull if the stack is full.
 */
#define stack_push(s, e)  stack_push_internal(s, (StackElement) (e))

static inline MmResult stack_push_internal(Stack *s, StackElement element) {
    if (s->top == s->limit) return kStackFull;
    memcpy(s->top, &element, s->element_size);
    s->top += s->element_size;
    return kOk;
}

/**
 * Gets an element by 0-based index from the stack.
 *
 * @param[in]  s  Pointer to the Stack.
 * @param[in]  i  Index of the element to get.
 * @param[out] e  On exit contains the value of the indexed element.
 * @return        kStackIndexOutOfBounds if i >= the number of elements in the stack.
 */
#define stack_get(s, i, e)  stack_get_internal(s, i, (StackElement *) (e))

static inline MmResult stack_get_internal(Stack *s, size_t idx, StackElement *element) {
    if (s->storage + idx * s->element_size >= s->top) return kStackIndexOutOfBounds;
    memcpy(element, s->storage + idx * s->element_size, s->element_size);
    return kOk;
}

/**
 * Removes the first matching element (starting from the top of the stack).
 *
 * @param[in]  s  Pointer to the Stack.
 * @param[in]  e  The element to remove.
 * @return        kStackElementNotFound if no matching element is found.
 */
#define stack_remove(s, e)  stack_remove_internal(s, (StackElement) (e))

static inline MmResult stack_remove_internal(Stack *s, StackElement element) {
    for (char *p = s->top - s->element_size;
         p >= s->storage;
         p -= s->element_size) {
        if (memcmp(p, &element, s->element_size) == 0) {
            s->top -= s->element_size;
            memmove(p, p + s->element_size, s->top - p);
            return kOk;
        }
    }
    return kStackElementNotFound;
}

/**
 * Replaces the first matching element (starting from the top of the stack).
 *
 * @param[in]  s  Pointer to the Stack.
 * @param[in]  f  The element to find.
 * @param[in]  r  The element to replace it with.
 * @return        kStackElementNotFound if no matching element is found.
 */
#define stack_replace(s, f, r) \
    stack_replace_internal(s, (StackElement) (f), (StackElement) (r))

static inline MmResult stack_replace_internal(Stack *s, StackElement find, StackElement replace)
{
    for (char *p = s->top - s->element_size;
         p >= s->storage;
         p -= s->element_size) {
        if (memcmp(p, &find, s->element_size) == 0) {
            memcpy(p, &replace, s->element_size);
            return kOk;
        }
    }
    return kStackElementNotFound;
}

/**
 * Gets the size of the stack.
 *
 * @param[in]  s    Pointer to the Stack.
 * @param[out] size On exit contains the size of the stack.
 */
static inline MmResult stack_size(Stack *s, size_t *size) {
    *size = (s->top - s->storage) / s->element_size;
    return kOk;
}

/**
 * Dumps the contents of the stack to STDOUT.
 */
static void stack_dump(Stack *s) {
    printf("--- BASE ---\n");
    size_t size = (s->top - s->storage) / s->element_size;
    StackElement element;
    for (size_t idx = 0; idx < size; ++idx) {
        (void) stack_get(s, idx, &element);
        printf("  %lx\n", element);
    }
    printf("--- TOP  ---\n");
}

#endif // #if !defined(MMB4L_STACK_H)
