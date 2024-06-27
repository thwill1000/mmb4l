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

#include <stdint.h>

typedef void* StackElement;

typedef struct {
  size_t capacity;
  size_t element_size;
  char *storage;
  size_t top;
} Stack;

#define stack_init(s, type, capacity)  stack_init_internal(s, sizeof(type), capacity)

static inline MmResult stack_init_internal(Stack *stack, size_t element_size, size_t capacity) {
    stack->capacity = capacity;
    stack->element_size = element_size;
    stack->storage = (char *) malloc(capacity * element_size);
    if (!stack->storage) return kOutOfMemory;
    stack->top = 0;
    return kOk;
}

static inline MmResult stack_term(Stack *stack) {
    free(stack->storage);
    stack->storage = NULL;
    return kOk;
}

#define stack_peek(s, e)  stack_peek_internal(s, (StackElement *) e)

static inline MmResult stack_peek_internal(Stack *stack, StackElement *element) {
    if (stack->top == 0) return kStackEmpty;
    memcpy(element, stack->storage + (stack->top - 1) * stack->element_size, stack->element_size);
    return kOk;
}

#define stack_pop(s, e)  stack_pop_internal(s, (StackElement *) e)

static inline MmResult stack_pop_internal(Stack *stack, StackElement *element) {
    if (stack->top == 0) return kStackEmpty;
    stack->top--;
    memcpy(element, stack->storage + stack->top * stack->element_size, stack->element_size);
    return kOk;
}

#define stack_push(s, e)  stack_push_internal(s, (StackElement) ((intptr_t) e))

static inline MmResult stack_push_internal(Stack *stack, StackElement element) {
    if (stack->top == stack->capacity) return kStackFull;
    memcpy(stack->storage + stack->top * stack->element_size, &element, stack->element_size);
    stack->top++;
    return kOk;
}

static inline MmResult stack_size(Stack *stack, size_t *size) {
    *size = stack->top;
    return kOk;
}

#endif // #if !defined(MMB4L_STACK_H)
