/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../stack.h"

} // extern "C"

#define EXPECTED_ELEMENTS(element_type, expected_elements, expected_num) \
    { \
        size_t size = 0; \
        element_type element = 0; \
        EXPECT_EQ(kOk, stack_size(&stack, &size)); \
        EXPECT_EQ(expected_num, size); \
        for (uint8_t ii = 0; ii < expected_num; ++ii) { \
            EXPECT_EQ(kOk, stack_get(&stack, ii, &element)); \
            EXPECT_EQ(expected_elements[ii], element); \
        } \
    }

class UInt8StackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, uint8_t, 10));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    void GivenStackFull() {
        for (uint8_t ii = 1; ii <= 10; ++ii) {
            EXPECT_EQ(kOk, stack_push(&stack, ii));
        }
    }

    Stack stack;
};

TEST_F(UInt8StackTest, Peek_Succeeds_GivenStackNotEmpty) {
    size_t size;
    uint8_t element = 0;

    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_peek(&stack, &element));
    EXPECT_EQ(42, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(UInt8StackTest, Peek_Fails_GivenStackEmpty) {
    size_t size;
    uint8_t element;

    EXPECT_EQ(kStackEmpty, stack_peek(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(UInt8StackTest, Push_Succeeds_GivenStackNotFull) {
    size_t size;

    EXPECT_EQ(kOk, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(UInt8StackTest, Push_Fails_GivenStackFull) {
    size_t size;

    for (size_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, 42));
    }

    EXPECT_EQ(kStackFull, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);
}

TEST_F(UInt8StackTest, Pop_Succeeds_GivenStackNotEmpty) {
    size_t size;
    uint8_t element = 0;

    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_pop(&stack, &element));
    EXPECT_EQ(42, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(UInt8StackTest, Pop_Fails_GivenStackEmpty) {
    uint8_t element;
    size_t size;

    EXPECT_EQ(kStackEmpty, stack_pop(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(UInt8StackTest, FillAndThenEmpty_Succeeds) {
    size_t size;
    uint8_t element = 0;

    for (uint8_t ii = 1; ii <= 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, ii));
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);

    for (uint8_t ii = 10; ii >= 1; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element));
        EXPECT_EQ(ii, element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii, size);
        EXPECT_EQ(kOk, stack_pop(&stack, &element));
        EXPECT_EQ(ii, element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii - 1, size);
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, 5));

    uint8_t expected[] = { 1, 2, 3, 4, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 9);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, 10));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    EXPECTED_ELEMENTS(uint8_t, expected, 9);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, 1));

    uint8_t expected[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 9);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenOnlyElement) {
    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_remove(&stack, 42));

    uint8_t expected[] = { };
    EXPECTED_ELEMENTS(uint8_t, expected, 0);
}

TEST_F(UInt8StackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, 42));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Get_Fails_GivenIndexOutOfBounds) {
    GivenStackFull();

    uint8_t element = 0;
    EXPECT_EQ(kStackIndexOutOfBounds, stack_get(&stack, 10, &element));
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenElementPresent) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, 5, 42));

    uint8_t expected[] = { 1, 2, 3, 4, 42, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, 10, 42));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 42 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, 1, 42));

    uint8_t expected[] = { 42, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenOnlyElement) {
    EXPECT_EQ(kOk, stack_push(&stack, 99));

    EXPECT_EQ(kOk, stack_replace(&stack, 99, 42));

    uint8_t expected[] = { 42 };
    EXPECTED_ELEMENTS(uint8_t, expected, 1);
}

TEST_F(UInt8StackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, 99, 42));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Fails_GivenEmptyStack) {
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, 99, 42));

    uint8_t expected[] = { };
    EXPECTED_ELEMENTS(uint8_t, expected, 0);
}

class Int32StackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, int32_t, 10));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    void GivenStackFull() {
        int32_t test_elements[] =
                { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
        for (size_t ii = 0; ii < 10; ++ii) {
            EXPECT_EQ(kOk, stack_push(&stack, test_elements[ii]));
        }
    }

    Stack stack;
};

TEST_F(Int32StackTest, Peek_Succeeds_GivenStackNotEmpty) {
    size_t size;
    int32_t element = 0;

    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_peek(&stack, &element));
    EXPECT_EQ(42, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(Int32StackTest, Peek_Fails_GivenStackEmpty) {
    size_t size;
    int32_t element;

    EXPECT_EQ(kStackEmpty, stack_peek(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(Int32StackTest, Push_Succeeds_GivenStackNotFull) {
    size_t size;

    EXPECT_EQ(kOk, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(Int32StackTest, Push_Fails_GivenStackFull) {
    size_t size;

    for (size_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, 42));
    }

    EXPECT_EQ(kStackFull, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);
}

TEST_F(Int32StackTest, Pop_Succeeds_GivenStackNotEmpty) {
    size_t size;
    int32_t element = 0;

    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_pop(&stack, &element));
    EXPECT_EQ(42, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(Int32StackTest, Pop_Fails_GivenStackEmpty) {
    size_t size;
    int32_t element;

    EXPECT_EQ(kStackEmpty, stack_pop(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(Int32StackTest, FillAndThenEmpty_Succeeds) {
    size_t size;
    int32_t element = 0;

    int32_t test_elements[] =
            { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    for (int32_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, test_elements[ii]));
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);

    for (int32_t ii = 9; ii >= 0; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element));
        EXPECT_EQ(test_elements[ii], element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii + 1, size);
        EXPECT_EQ(kOk, stack_pop(&stack, &element));
        EXPECT_EQ(test_elements[ii], element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii, size);
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, 5));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 9);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, INT32_MIN));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX };
    EXPECTED_ELEMENTS(int32_t, expected, 9);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, INT32_MAX - 1));

    int32_t expected[] = { INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 9);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenOnlyElement) {
    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_remove(&stack, 42));

    int32_t expected[] = { };
    EXPECTED_ELEMENTS(int32_t, expected, 0);
}

TEST_F(Int32StackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, 42));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Get_Fails_GivenIndexOutOfBounds) {
    GivenStackFull();

    int32_t element = 0;
    EXPECT_EQ(kStackIndexOutOfBounds, stack_get(&stack, 10, &element));
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenElementPresent) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, 5, 42));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 42, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, INT32_MIN, 42));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, 42 };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, INT32_MAX - 1, 42));

    int32_t expected[] = { 42, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenOnlyElement) {
    EXPECT_EQ(kOk, stack_push(&stack, 99));

    EXPECT_EQ(kOk, stack_replace(&stack, 99, 42));

    int32_t expected[] = { 42 };
    EXPECTED_ELEMENTS(int32_t, expected, 1);
}

TEST_F(Int32StackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, 99, 42));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Fails_GivenEmptyStack) {
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, 99, 42));

    int32_t expected[] = { };
    EXPECTED_ELEMENTS(int32_t, expected, 0);
}

class PointerStackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, void *, 10));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    void GivenStackFull() {
        for (size_t ii = 0; ii < 10; ++ii) {
            EXPECT_EQ(kOk, stack_push(&stack, &stack + ii));
        }
    }

    Stack stack;
};

TEST_F(PointerStackTest, Peek_Succeeds_GivenStackNotEmpty) {
    int data = 0;
    size_t size;
    void *element;

    EXPECT_EQ(kOk, stack_push(&stack, &data));

    EXPECT_EQ(kOk, stack_peek(&stack, &element));
    EXPECT_EQ(&data, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(PointerStackTest, Peek_Fails_GivenStackEmpty) {
    size_t size;
    void *element;

    EXPECT_EQ(kStackEmpty, stack_peek(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(PointerStackTest, Push_Succeeds_GivenStackNotFull) {
    int data = 0;
    size_t size;

    EXPECT_EQ(kOk, stack_push(&stack, &data));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(PointerStackTest, Push_Fails_GivenStackFull) {
    size_t size;

    for (size_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, 42));
    }

    EXPECT_EQ(kStackFull, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);
}

TEST_F(PointerStackTest, Pop_Succeeds_GivenStackNotEmpty) {
    int data = 0;
    size_t size;
    void *element;

    EXPECT_EQ(kOk, stack_push(&stack, &data));

    EXPECT_EQ(kOk, stack_pop(&stack, &element));
    EXPECT_EQ(&data, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(PointerStackTest, Pop_Fails_GivenStackEmpty) {
    size_t size;
    void *element;

    EXPECT_EQ(kStackEmpty, stack_pop(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(PointerStackTest, FillAndThenEmpty_Succeeds) {
    size_t size;
    void *element;

    for (size_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, &stack + ii));
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);

    for (int32_t ii = 9; ii >= 0; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element));
        EXPECT_EQ((&stack + ii), element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii + 1, size);
        EXPECT_EQ(kOk, stack_pop(&stack, &element));
        EXPECT_EQ((&stack + ii), element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii, size);
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, &stack + 5));

    void *expected[] = { &stack, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 6, &stack + 7, &stack + 8, &stack + 9 };
    EXPECTED_ELEMENTS(void *, expected, 9);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, &stack + 9));

    void *expected[] = { &stack, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 5, &stack + 6, &stack + 7, &stack + 8 };
    EXPECTED_ELEMENTS(void *, expected, 9);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_remove(&stack, &stack));

    void *expected[] = { &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 5, &stack + 6, &stack + 7, &stack + 8, &stack + 9 };
    EXPECTED_ELEMENTS(void *, expected, 9);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenOnlyElement) {
    EXPECT_EQ(kOk, stack_push(&stack, &stack + 42));

    EXPECT_EQ(kOk, stack_remove(&stack, &stack + 42));

    void *expected[] = { };
    EXPECTED_ELEMENTS(void *, expected, 0);
}

TEST_F(PointerStackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, &stack + 42));

    void *expected[] = { &stack, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 5, &stack + 6, &stack + 7, &stack + 8, &stack + 9 };
    EXPECTED_ELEMENTS(void *, expected, 10);
}

TEST_F(PointerStackTest, Get_Fails_GivenIndexOutOfBounds) {
    GivenStackFull();

    void *element = NULL;
    EXPECT_EQ(kStackIndexOutOfBounds, stack_get(&stack, 10, &element));
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenElementPresent) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, &stack + 5, &stack + 42));

    void *expected[] = { &stack, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 42, &stack + 6, &stack + 7, &stack + 8, &stack + 9 };
    EXPECTED_ELEMENTS(void *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, &stack + 9, &stack + 42));

    void *expected[] = { &stack, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 5, &stack + 6, &stack + 7, &stack + 8, &stack + 42 };
    EXPECTED_ELEMENTS(void *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    EXPECT_EQ(kOk, stack_replace(&stack, &stack, &stack + 42));

    void *expected[] = { &stack + 42, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 5, &stack + 6, &stack + 7, &stack + 8, &stack + 9 };
    EXPECTED_ELEMENTS(void *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenOnlyElement) {
    EXPECT_EQ(kOk, stack_push(&stack, &stack + 99));

    EXPECT_EQ(kOk, stack_replace(&stack, &stack + 99, &stack + 42));

    void *expected[] = { &stack + 42 };
    EXPECTED_ELEMENTS(void *, expected, 1);
}

TEST_F(PointerStackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, &stack + 99, &stack + 42));

    void *expected[] = { &stack, &stack + 1, &stack + 2, &stack + 3, &stack + 4,
                         &stack + 5, &stack + 6, &stack + 7, &stack + 8, &stack + 9 };
    EXPECTED_ELEMENTS(void *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Fails_GivenEmptyStack) {
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, &stack + 99, &stack + 42));

    void *expected[] = { };
    EXPECTED_ELEMENTS(void *, expected, 0);
}
