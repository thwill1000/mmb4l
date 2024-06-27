/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../stack.h"

} // extern "C"

class UInt8StackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, uint8_t, 10));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    Stack stack;
};

TEST_F(UInt8StackTest, Peek_Fails_GivenStackEmpty) {
    size_t size;
    uint8_t element;

    EXPECT_EQ(kStackEmpty, stack_peek(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
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

TEST_F(UInt8StackTest, Pop_Fails_GivenStackEmpty) {
    uint8_t element;
    size_t size;

    EXPECT_EQ(kStackEmpty, stack_pop(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(UInt8StackTest, Push_Succeeds_GivenStackNotFull) {
    size_t size;

    EXPECT_EQ(kOk, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(UInt8StackTest, Peek_Succeeds_GivenStackNotEmpty) {
    size_t size;
    uint8_t element = 0;

    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_peek(&stack, &element));
    EXPECT_EQ(42, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
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

TEST_F(UInt8StackTest, FillAndThenEmpty) {
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

class Int32StackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, int32_t, 10));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    Stack stack;
};

TEST_F(Int32StackTest, Peek_Fails_GivenStackEmpty) {
    size_t size;
    int32_t element;

    EXPECT_EQ(kStackEmpty, stack_peek(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
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

TEST_F(Int32StackTest, Pop_Fails_GivenStackEmpty) {
    size_t size;
    int32_t element;

    EXPECT_EQ(kStackEmpty, stack_pop(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}

TEST_F(Int32StackTest, Push_Succeeds_GivenStackNotFull) {
    size_t size;

    EXPECT_EQ(kOk, stack_push(&stack, 42));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
}

TEST_F(Int32StackTest, Peek_Succeeds_GivenStackNotEmpty) {
    size_t size;
    int32_t element = 0;

    EXPECT_EQ(kOk, stack_push(&stack, 42));

    EXPECT_EQ(kOk, stack_peek(&stack, &element));
    EXPECT_EQ(42, element);
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(1, size);
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

TEST_F(Int32StackTest, FillAndThenEmpty) {
    size_t size;
    int32_t element = 0;

    int32_t test_elements[] = { INT32_MAX, INT32_MIN, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
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

class PointerStackTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, stack_init(&stack, void *, 10));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, stack_term(&stack));
    }

    Stack stack;
};

TEST_F(PointerStackTest, Peek_Fails_GivenStackEmpty) {
    size_t size;
    void *element;

    EXPECT_EQ(kStackEmpty, stack_peek(&stack, &element));
    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
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

TEST_F(PointerStackTest, Pop_Fails_GivenStackEmpty) {
    size_t size;
    void *element;

    EXPECT_EQ(kStackEmpty, stack_pop(&stack, &element));
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

TEST_F(PointerStackTest, FillAndThenEmpty) {
    size_t size;
    void *element;

    int32_t test_elements[] = { INT32_MAX, INT32_MIN, 3, 4, 5, 6, 7, 8, INT32_MAX, INT32_MIN };
    for (int32_t ii = 0; ii < 10; ++ii) {
        EXPECT_EQ(kOk, stack_push(&stack, &test_elements[ii]));
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(10, size);

    for (int32_t ii = 9; ii >= 0; --ii) {
        EXPECT_EQ(kOk, stack_peek(&stack, &element));
        EXPECT_EQ(&test_elements[ii], element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii + 1, size);
        EXPECT_EQ(kOk, stack_pop(&stack, &element));
        EXPECT_EQ(&test_elements[ii], element);
        EXPECT_EQ(kOk, stack_size(&stack, &size));
        EXPECT_EQ(ii, size);
    }

    EXPECT_EQ(kOk, stack_size(&stack, &size));
    EXPECT_EQ(0, size);
}
