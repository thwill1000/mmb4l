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
        element_type element_out; \
        EXPECT_EQ(expected_num, stack_size(&stack)); \
        for (uint8_t ii = 0; ii < expected_num; ++ii) { \
            EXPECT_EQ(kOk, stack_get(&stack, ii, &element_out)); \
            EXPECT_EQ(expected_elements[ii], element_out); \
        } \
    }

class UInt8StackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, uint8_t, 10, NULL));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    void GivenStackFull() {
        for (uint8_t element_in = 1; element_in <= 10; ++element_in) {
            EXPECT_EQ(kOk, stack_push(&stack, element_in));
        }
    }

    Stack stack;
};

TEST_F(UInt8StackTest, Contains_GivenElementPresent_ReturnsTrue) {
    GivenStackFull();

    const uint8_t element_in = 1;
    EXPECT_TRUE(stack_contains(&stack, element_in));
}

TEST_F(UInt8StackTest, Contains_GivenElementNotPresent_ReturnsFalse) {
    GivenStackFull();

    const uint8_t element_in = 42;
    EXPECT_FALSE(stack_contains(&stack, element_in));
}

TEST_F(UInt8StackTest, Peek_Succeeds_GivenStackNotEmpty) {
    const uint8_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    uint8_t element_out;
    EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
    EXPECT_EQ(element_in, element_out);
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(UInt8StackTest, Peek_Fails_GivenStackEmpty) {
    uint8_t element_out;

    EXPECT_EQ(kContainerEmpty, stack_peek(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(UInt8StackTest, Push_Succeeds_GivenStackNotFull) {
    const uint8_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(UInt8StackTest, Push_Fails_GivenStackFull) {
    for (uint8_t element_in = 0; element_in < 10; ++element_in) {
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    const uint8_t element_in = 10;
    EXPECT_EQ(kContainerFull, stack_push(&stack, element_in));
    EXPECT_EQ(10, stack_size(&stack));
}

TEST_F(UInt8StackTest, Pop_Succeeds_GivenStackNotEmpty) {
    const uint8_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    uint8_t element_out;
    EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
    EXPECT_EQ(element_in, element_out);
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(UInt8StackTest, Pop_Fails_GivenStackEmpty) {
    uint8_t element_out;
    EXPECT_EQ(kContainerEmpty, stack_pop(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(UInt8StackTest, FillAndThenEmpty_Succeeds) {
    for (uint8_t element_in = 1; element_in <= 10; ++element_in) {
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    EXPECT_EQ(10, stack_size(&stack));

    uint8_t element_out;
    for (uint8_t ii = 10; ii >= 1; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
        EXPECT_EQ(ii, element_out);
        EXPECT_EQ(ii, stack_size(&stack));
        EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
        EXPECT_EQ(ii, element_out);
        EXPECT_EQ(ii - 1, stack_size(&stack));
    }

    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const uint8_t element_remove = 5;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    uint8_t expected[] = { 1, 2, 3, 4, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 9);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    const uint8_t element_remove = 10;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    EXPECTED_ELEMENTS(uint8_t, expected, 9);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const uint8_t element_remove = 1;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    uint8_t expected[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 9);
}

TEST_F(UInt8StackTest, Remove_Succeeds_GivenOnlyElement) {
    const uint8_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    const uint8_t element_remove = element_in;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    uint8_t expected[] = { };
    EXPECTED_ELEMENTS(uint8_t, expected, 0);
}

TEST_F(UInt8StackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    const uint8_t element_remove = 42;
    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, element_remove));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Get_Fails_GivenIndexOutOfBounds) {
    GivenStackFull();

    uint8_t element_out;
    EXPECT_EQ(kStackIndexOutOfBounds, stack_get(&stack, 10, &element_out));
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const uint8_t element_find = 5;
    const uint8_t element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    uint8_t expected[] = { 1, 2, 3, 4, 42, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    const uint8_t element_find = 10;
    const uint8_t element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 42 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const uint8_t element_find = 1;
    const uint8_t element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    uint8_t expected[] = { 42, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Succeeds_GivenOnlyElement) {
    const uint8_t element_find = 99;
    EXPECT_EQ(kOk, stack_push(&stack, element_find));

    const uint8_t element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    uint8_t expected[] = { 42 };
    EXPECTED_ELEMENTS(uint8_t, expected, 1);
}

TEST_F(UInt8StackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    const uint8_t element_find = 99;
    const uint8_t element_replace = 42;
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    uint8_t expected[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECTED_ELEMENTS(uint8_t, expected, 10);
}

TEST_F(UInt8StackTest, Replace_Fails_GivenEmptyStack) {
    const uint8_t element_find = 99;
    const uint8_t element_replace = 42;
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    uint8_t expected[] = { };
    EXPECTED_ELEMENTS(uint8_t, expected, 0);
}

class Int32StackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, int32_t, 10, NULL));
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

TEST_F(Int32StackTest, Contains_GivenElementPresent_ReturnsTrue) {
    GivenStackFull();

    const int32_t element_in = 3;
    EXPECT_TRUE(stack_contains(&stack, element_in));
}

TEST_F(Int32StackTest, Contains_GivenElementNotPresent_ReturnsFalse) {
    GivenStackFull();

    const int32_t element_in = 42;
    EXPECT_FALSE(stack_contains(&stack, element_in));
}

TEST_F(Int32StackTest, Peek_Succeeds_GivenStackNotEmpty) {
    const int32_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    int32_t element_out;
    EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
    EXPECT_EQ(element_in, element_out);
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(Int32StackTest, Peek_Fails_GivenStackEmpty) {
    int32_t element_out;

    EXPECT_EQ(kContainerEmpty, stack_peek(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(Int32StackTest, Push_Succeeds_GivenStackNotFull) {
    const int32_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(Int32StackTest, Push_Fails_GivenStackFull) {
    for (size_t element_in = 0; element_in < 10; ++element_in) {
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    const size_t element_in = 10;
    EXPECT_EQ(kContainerFull, stack_push(&stack, element_in));
    EXPECT_EQ(10, stack_size(&stack));
}

TEST_F(Int32StackTest, Pop_Succeeds_GivenStackNotEmpty) {
    const int32_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    int32_t element_out;
    EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
    EXPECT_EQ(element_in, element_out);
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(Int32StackTest, Pop_Fails_GivenStackEmpty) {
    int32_t element_out;

    EXPECT_EQ(kContainerEmpty, stack_pop(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(Int32StackTest, FillAndThenEmpty_Succeeds) {
    int32_t element_out;

    int32_t test_elements[] =
            { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    for (int32_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, test_elements[ii]));
    }

    EXPECT_EQ(10, stack_size(&stack));

    for (int32_t ii = 9; ii >= 0; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
        EXPECT_EQ(test_elements[ii], element_out);
        EXPECT_EQ(ii + 1, stack_size(&stack));
        EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
        EXPECT_EQ(test_elements[ii], element_out);
        EXPECT_EQ(ii, stack_size(&stack));
    }

    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const int32_t element_remove = 5;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 9);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    const int32_t element_remove = INT32_MIN;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX };
    EXPECTED_ELEMENTS(int32_t, expected, 9);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const int32_t element_remove = INT32_MAX - 1;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    int32_t expected[] = { INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 9);
}

TEST_F(Int32StackTest, Remove_Succeeds_GivenOnlyElement) {
    const int32_t element_in = 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    const int32_t element_remove = element_in;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    int32_t expected[] = { };
    EXPECTED_ELEMENTS(int32_t, expected, 0);
}

TEST_F(Int32StackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    const int32_t element_remove = 42;
    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, element_remove));

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

    const int element_find = 5;
    const int element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 42, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    const int element_find = INT32_MIN;
    const int element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, 42 };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const int element_find = INT32_MAX - 1;
    const int element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    int32_t expected[] = { 42, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Succeeds_GivenOnlyElement) {
    const int32_t element_in = 99;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    const int element_find = 99;
    const int element_replace = 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    int32_t expected[] = { 42 };
    EXPECTED_ELEMENTS(int32_t, expected, 1);
}

TEST_F(Int32StackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    const int element_find = 99;
    const int element_replace = 42;
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    int32_t expected[] = { INT32_MAX - 1, INT32_MIN + 1, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    EXPECTED_ELEMENTS(int32_t, expected, 10);
}

TEST_F(Int32StackTest, Replace_Fails_GivenEmptyStack) {
    const int element_find = 99;
    const int element_replace = 42;
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    int32_t expected[] = { };
    EXPECTED_ELEMENTS(int32_t, expected, 0);
}

class PointerStackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, uint64_t *, 10, NULL));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    void GivenStackFull() {
        for (size_t ii = 0; ii < 10; ++ii) {
            uint64_t *element_in = (uint64_t *) &stack + ii;
            EXPECT_EQ(kOk, stack_push(&stack, element_in));
        }
    }

    Stack stack;
};

TEST_F(PointerStackTest, Contains_GivenElementPresent_ReturnsTrue) {
    GivenStackFull();

    const uint64_t *element_in = (uint64_t *) &stack + 1;
    EXPECT_TRUE(stack_contains(&stack, element_in));
}

TEST_F(PointerStackTest, Contains_GivenElementNotPresent_ReturnsFalse) {
    GivenStackFull();

    const uint64_t *element_in = (uint64_t *) &stack + 42;
    EXPECT_FALSE(stack_contains(&stack, element_in));
}

TEST_F(PointerStackTest, Peek_Succeeds_GivenStackNotEmpty) {
    uint64_t data = 42;
    const uint64_t *element_in = &data;

    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    uint64_t *element_out;
    EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
    EXPECT_EQ(&data, element_out);
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(PointerStackTest, Peek_Fails_GivenStackEmpty) {
    uint64_t *element_out;

    EXPECT_EQ(kContainerEmpty, stack_peek(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(PointerStackTest, Push_Succeeds_GivenStackNotFull) {
    uint64_t data = 42;
    const uint64_t *element_in = &data;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(PointerStackTest, Push_Fails_GivenStackFull) {
    for (size_t ii = 0; ii < 10; ++ii) {
        const uint64_t *element_in = (uint64_t *) &stack + ii;
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    uint64_t data = 42;
    const uint64_t *element_in = &data;
    EXPECT_EQ(kContainerFull, stack_push(&stack, element_in));
    EXPECT_EQ(10, stack_size(&stack));
}

TEST_F(PointerStackTest, Pop_Succeeds_GivenStackNotEmpty) {
    uint64_t data = 42;
    const uint64_t *element_in = &data;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    uint64_t *element_out;
    EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
    EXPECT_EQ(&data, element_out);
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(PointerStackTest, Pop_Fails_GivenStackEmpty) {
    uint64_t *element_out;

    EXPECT_EQ(kContainerEmpty, stack_pop(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(PointerStackTest, FillAndThenEmpty_Succeeds) {
    for (size_t ii = 0; ii < 10; ++ii) {
        const uint64_t *element_in = (uint64_t *) &stack + ii;
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    EXPECT_EQ(10, stack_size(&stack));

    uint64_t *element_out;
    for (int32_t ii = 9; ii >= 0; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
        EXPECT_EQ((uint64_t *) &stack + ii, element_out);
        EXPECT_EQ(ii + 1, stack_size(&stack));
        EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
        EXPECT_EQ((uint64_t *) &stack + ii, element_out);
        EXPECT_EQ(ii, stack_size(&stack));
    }

    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const uint64_t *element_remove = (uint64_t *) &stack + 5;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 9
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 9);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    const uint64_t *element_remove = (uint64_t *) &stack + 9;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 5,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 9);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const uint64_t *element_remove = (uint64_t *) &stack;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 5,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 9
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 9);
}

TEST_F(PointerStackTest, Remove_Succeeds_GivenOnlyElement) {
    const uint64_t *element_in = (uint64_t *) &stack + 42;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    const uint64_t *element_remove = element_in;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    uint64_t *expected[] = { };
    EXPECTED_ELEMENTS(uint64_t *, expected, 0);
}

TEST_F(PointerStackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    const uint64_t *element_remove = (uint64_t *) &stack + 42;
    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, element_remove));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 5,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 9
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 10);
}

TEST_F(PointerStackTest, Get_Fails_GivenIndexOutOfBounds) {
    GivenStackFull();

    uint64_t *element_out;
    EXPECT_EQ(kStackIndexOutOfBounds, stack_get(&stack, 10, &element_out));
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const uint64_t *element_find = (uint64_t *) &stack + 5;
    const uint64_t *element_replace = (uint64_t *) &stack + 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 42,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 9
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    const uint64_t *element_find = (uint64_t *) &stack + 9;
    const uint64_t *element_replace = (uint64_t *) &stack + 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 5,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 42
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const uint64_t *element_find = (uint64_t *) &stack;
    const uint64_t *element_replace = (uint64_t *) &stack + 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack + 42,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 5,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 9
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Succeeds_GivenOnlyElement) {
    const uint64_t *element_in = (uint64_t *) &stack + 99;
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    const uint64_t *element_find = (uint64_t *) &stack + 99;
    const uint64_t *element_replace = (uint64_t *) &stack + 42;
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    uint64_t *expected[] = { (uint64_t *) &stack + 42 };
    EXPECTED_ELEMENTS(uint64_t *, expected, 1);
}

TEST_F(PointerStackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    const uint64_t *element_find = (uint64_t *) &stack + 99;
    const uint64_t *element_replace = (uint64_t *) &stack + 42;
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    uint64_t *expected[] = {
        (uint64_t *) &stack,
        (uint64_t *) &stack + 1,
        (uint64_t *) &stack + 2,
        (uint64_t *) &stack + 3,
        (uint64_t *) &stack + 4,
        (uint64_t *) &stack + 5,
        (uint64_t *) &stack + 6,
        (uint64_t *) &stack + 7,
        (uint64_t *) &stack + 8,
        (uint64_t *) &stack + 9
    };
    // clang-format on
    EXPECTED_ELEMENTS(uint64_t *, expected, 10);
}

TEST_F(PointerStackTest, Replace_Fails_GivenEmptyStack) {
    const uint64_t *element_find = (uint64_t *) &stack + 99;
    const uint64_t *element_replace = (uint64_t *) &stack + 42;
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    uint64_t *expected[] = { };
    EXPECTED_ELEMENTS(uint64_t *, expected, 0);
}

typedef struct {
    int a;
    double b;
} MyStruct;

#define MY_STRUCT_VALUE(x)  { .a = (x), .b = 2 * (double) (x) / 3 }

#define EXPECT_MY_STRUCT_EQ(expected, actual) \
    EXPECT_EQ((expected).a, (actual).a); \
    EXPECT_EQ((expected).b, (actual).b)

bool MyStructEquals(const void *one, const void *two) {
    const MyStruct *s1 = (MyStruct *) one;
    const MyStruct *s2 = (MyStruct *) two;
    return (s1->a == s2->a) && (s1->b == s2->b);
}

class StructStackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, MyStruct, 10, MyStructEquals));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    void GivenStackFull() {
        for (int ii = 1; ii <= 10; ++ii) {
            MyStruct element_in = MY_STRUCT_VALUE(ii);
            EXPECT_EQ(kOk, stack_push(&stack, element_in));
        }
    }

    Stack stack;
};

TEST_F(StructStackTest, Contains_GivenElementPresent_ReturnsTrue) {
    GivenStackFull();

    const MyStruct element_in = MY_STRUCT_VALUE(1);
    EXPECT_TRUE(stack_contains(&stack, element_in));
}

TEST_F(StructStackTest, Contains_GivenElementNotPresent_ReturnsFalse) {
    GivenStackFull();

    const MyStruct element_in = MY_STRUCT_VALUE(42);
    EXPECT_FALSE(stack_contains(&stack, element_in));
}

TEST_F(StructStackTest, Peek_Succeeds_GivenStackNotEmpty) {
    const MyStruct element_in = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    MyStruct element_out;
    EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
    EXPECT_MY_STRUCT_EQ(element_in, element_out);
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(StructStackTest, Peek_Fails_GivenStackEmpty) {
    MyStruct element_out;

    EXPECT_EQ(kContainerEmpty, stack_peek(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(StructStackTest, Push_Succeeds_GivenStackNotFull) {
    const MyStruct element_in = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_push(&stack, element_in));
    EXPECT_EQ(1, stack_size(&stack));
}

TEST_F(StructStackTest, Push_Fails_GivenStackFull) {
    for (uint8_t ii = 0; ii < 10; ++ii) {
        MyStruct element_in = MY_STRUCT_VALUE(ii);
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    const MyStruct element_in = MY_STRUCT_VALUE(10);
    EXPECT_EQ(kContainerFull, stack_push(&stack, element_in));
    EXPECT_EQ(10, stack_size(&stack));
}

TEST_F(StructStackTest, Pop_Succeeds_GivenStackNotEmpty) {
    const MyStruct element_in = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    MyStruct element_out;
    EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
    EXPECT_MY_STRUCT_EQ(element_in, element_out);
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(StructStackTest, Pop_Fails_GivenStackEmpty) {
    MyStruct element_out;
    EXPECT_EQ(kContainerEmpty, stack_pop(&stack, &element_out));
    EXPECT_EQ(0, stack_size(&stack));
}

TEST_F(StructStackTest, FillAndThenEmpty_Succeeds) {
    for (uint8_t ii = 1; ii <= 10; ++ii) {
        MyStruct element_in = MY_STRUCT_VALUE(ii);
        EXPECT_EQ(kOk, stack_push(&stack, element_in));
    }

    EXPECT_EQ(10, stack_size(&stack));

    MyStruct element_out;
    for (uint8_t ii = 10; ii >= 1; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element_out));
        const MyStruct expected = MY_STRUCT_VALUE(ii);
        EXPECT_MY_STRUCT_EQ(expected, element_out);
        EXPECT_EQ(ii, stack_size(&stack));
        EXPECT_EQ(kOk, stack_pop(&stack, &element_out));
        EXPECT_MY_STRUCT_EQ(expected, element_out);
        EXPECT_EQ(ii - 1, stack_size(&stack));
    }

    EXPECT_EQ(0, stack_size(&stack));
}

#define EXPECTED_MY_STRUCT_ELEMENTS(expected_elements, expected_num) \
    { \
        MyStruct element_out; \
        EXPECT_EQ(expected_num, stack_size(&stack)); \
        for (uint8_t ii = 0; ii < expected_num; ++ii) { \
            EXPECT_EQ(kOk, stack_get(&stack, ii, &element_out)); \
            EXPECT_MY_STRUCT_EQ(expected_elements[ii], element_out); \
        } \
    }

TEST_F(StructStackTest, Remove_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const MyStruct element_remove = MY_STRUCT_VALUE(5);
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(1),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(10)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 9);
}

TEST_F(StructStackTest, Remove_Succeeds_GivenTopElement) {
    GivenStackFull();

    const MyStruct element_remove = MY_STRUCT_VALUE(10);
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(1),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(5),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 9);
}

TEST_F(StructStackTest, Remove_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const MyStruct element_remove = MY_STRUCT_VALUE(1);
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(5),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(10)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 9);
}

TEST_F(StructStackTest, Remove_Succeeds_GivenOnlyElement) {
    const MyStruct element_in = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_push(&stack, element_in));

    const MyStruct element_remove = element_in;
    EXPECT_EQ(kOk, stack_remove(&stack, element_remove));

    MyStruct expected[] = { };
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 0);
}

TEST_F(StructStackTest, Remove_Fails_GivenElementNotFound) {
    GivenStackFull();

    const MyStruct element_remove = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kStackElementNotFound, stack_remove(&stack, element_remove));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(1),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(5),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(10)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 10);
}

TEST_F(StructStackTest, Get_Fails_GivenIndexOutOfBounds) {
    GivenStackFull();

    MyStruct element_out;
    EXPECT_EQ(kStackIndexOutOfBounds, stack_get(&stack, 10, &element_out));
}

TEST_F(StructStackTest, Replace_Succeeds_GivenElementPresent) {
    GivenStackFull();

    const MyStruct element_find = MY_STRUCT_VALUE(5);
    const MyStruct element_replace = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(1),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(42),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(10)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 10);
}

TEST_F(StructStackTest, Replace_Succeeds_GivenTopElement) {
    GivenStackFull();

    const MyStruct element_find = MY_STRUCT_VALUE(10);
    const MyStruct element_replace = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(1),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(5),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(42)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 10);
}

TEST_F(StructStackTest, Replace_Succeeds_GivenBaseElement) {
    GivenStackFull();

    const MyStruct element_find = MY_STRUCT_VALUE(1);
    const MyStruct element_replace = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(42),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(5),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(10)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 10);
}

TEST_F(StructStackTest, Replace_Succeeds_GivenOnlyElement) {
    const MyStruct element_find = MY_STRUCT_VALUE(99);
    EXPECT_EQ(kOk, stack_push(&stack, element_find));

    const MyStruct element_replace = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, stack_replace(&stack, element_find, element_replace));

    MyStruct expected[] = { MY_STRUCT_VALUE(42) };
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 1);
}

TEST_F(StructStackTest, Replace_Fails_GivenElementNotFound) {
    GivenStackFull();

    const MyStruct element_find = MY_STRUCT_VALUE(99);
    const MyStruct element_replace = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    // clang-format off
    MyStruct expected[] = {
        MY_STRUCT_VALUE(1),
        MY_STRUCT_VALUE(2),
        MY_STRUCT_VALUE(3),
        MY_STRUCT_VALUE(4),
        MY_STRUCT_VALUE(5),
        MY_STRUCT_VALUE(6),
        MY_STRUCT_VALUE(7),
        MY_STRUCT_VALUE(8),
        MY_STRUCT_VALUE(9),
        MY_STRUCT_VALUE(10)
    };
    // clang-format on
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 10);
}

TEST_F(StructStackTest, Replace_Fails_GivenEmptyStack) {
    const MyStruct element_find = MY_STRUCT_VALUE(99);
    const MyStruct element_replace = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kStackElementNotFound, stack_replace(&stack, element_find, element_replace));

    MyStruct expected[] = { };
    EXPECTED_MY_STRUCT_ELEMENTS(expected, 0);
}
