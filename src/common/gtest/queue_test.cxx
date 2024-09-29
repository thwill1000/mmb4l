/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../queue.h"

} // extern "C"

#define NUM_ELEMENTS  10

class UInt8QueueTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, queue_init(&queue, uint8_t, NUM_ELEMENTS));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, queue_term(&queue));
    }

    void GivenQueueFull() {
        for (uint8_t element_in = 0; element_in < NUM_ELEMENTS; ++element_in) {
            EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        }
    }

    void GivenQueueHas3Elements() {
        uint8_t element_in = 42;
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        element_in = 55;
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        element_in = 99;
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    }

    Queue queue;
};

TEST_F(UInt8QueueTest, IsEmpty_GivenEmpty_ReturnsTrue) {
    EXPECT_EQ(true, queue_is_empty(&queue));
}

TEST_F(UInt8QueueTest, IsEmpty_GivenPartiallyFull_ReturnsFalse) {
    GivenQueueHas3Elements();

    EXPECT_EQ(false, queue_is_empty(&queue));
}

TEST_F(UInt8QueueTest, IsEmpty_GivenFull_ReturnsFalse) {
    GivenQueueFull();

    EXPECT_EQ(false, queue_is_empty(&queue));
}

TEST_F(UInt8QueueTest, IsFull_GivenEmpty_ReturnsFalse) {
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(UInt8QueueTest, IsFull_GivenPartiallyFull_ReturnsFalse) {
    GivenQueueHas3Elements();

    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(UInt8QueueTest, IsFull_GivenFull_ReturnsTrue) {
    GivenQueueFull();

    EXPECT_EQ(true, queue_is_full(&queue));
}

TEST_F(UInt8QueueTest, Enqueue_AddsItemToQueue) {
    uint8_t element_in = 0;

    element_in = 42;
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    EXPECT_EQ(42, *queue.storage);

    element_in = 55;
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    EXPECT_EQ(55, *(queue.storage + 1));

    element_in = 99;
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    EXPECT_EQ(99, *(queue.storage + 2));
}

TEST_F(UInt8QueueTest, Enqueue_GivenFull_Fails) {
    GivenQueueFull();

    uint8_t  element_in = 42;
    EXPECT_EQ(kContainerFull, queue_enqueue(&queue, element_in));
}

TEST_F(UInt8QueueTest, Dequeue_GetsItemFromQueue) {
    uint8_t element_out;
    GivenQueueHas3Elements();

    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    EXPECT_EQ(42, element_out);

    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    EXPECT_EQ(55, element_out);

    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    EXPECT_EQ(99, element_out);

    EXPECT_EQ(kContainerEmpty, queue_dequeue(&queue, &element_out));
}

TEST_F(UInt8QueueTest, Size_GivenEmpty_ReturnsZero) {
    EXPECT_EQ(0, queue_size(&queue));
}

TEST_F(UInt8QueueTest, Size_GivenPartiallyFull_ReturnsSizeOfQueue) {
    GivenQueueHas3Elements();

    EXPECT_EQ(3, queue_size(&queue));
}

TEST_F(UInt8QueueTest, Size_GivenFull_ReturnsSizeOfQueue) {
    GivenQueueFull();

    EXPECT_EQ(NUM_ELEMENTS, queue_size(&queue));
}

TEST_F(UInt8QueueTest, Clear_GivenEmpty_Succeeds) {
    queue_clear(&queue);
    EXPECT_EQ(0, queue_size(&queue));
    EXPECT_EQ(true, queue_is_empty(&queue));
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(UInt8QueueTest, Clear_GivenPartiallyFull_Succeeds) {
    GivenQueueHas3Elements();

    queue_clear(&queue);
    EXPECT_EQ(0, queue_size(&queue));
    EXPECT_EQ(true, queue_is_empty(&queue));
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(UInt8QueueTest, Clear_GivenFull_Succeeds) {
    GivenQueueFull();

    queue_clear(&queue);
    EXPECT_EQ(0, queue_size(&queue));
    EXPECT_EQ(true, queue_is_empty(&queue));
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(UInt8QueueTest, QueueWillNotFillIfEmptiedAtSameRate) {
    for (uint8_t element_in = 0; element_in < 100; ++element_in) {
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        uint8_t element_out;
        EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
        EXPECT_EQ(element_in, element_out);
    }
}

TEST_F(UInt8QueueTest, QueueWrapsAround) {
    GivenQueueFull();

    // Dequeue the first element.
    uint8_t element_out;
    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    EXPECT_EQ(0, element_out);

    // Enqueue another element which should cause the back pointer to wrap around.
    uint8_t element_in = 10;
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));

    // Trying to enqueue another element should fail.
    EXPECT_EQ(kContainerFull, queue_enqueue(&queue, element_in));

    // Should be 10 elements to dequeue.
    for (uint8_t ii = 1; ii <= 10; ++ii) {
        EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
        EXPECT_EQ(ii, element_out);
    }

    // Queue should now be empty.
    EXPECT_EQ(kContainerEmpty, queue_dequeue(&queue, &element_out));

    // Trying to enqueue another element should now succeed.
    element_in = 11;
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));

    // And we should be able to dequeue that element.
    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    EXPECT_EQ(11, element_out);

    // Queue should now be empty.
    EXPECT_EQ(kContainerEmpty, queue_dequeue(&queue, &element_out));
}

typedef struct {
    int a;
    double b;
} MyStruct;

#define MY_STRUCT_VALUE(x)  { .a = (x), .b = 2 * (double) (x) / 3 }

#define EXPECT_MY_STRUCT_EQ(expected, actual) \
    EXPECT_EQ((expected).a, (actual).a); \
    EXPECT_EQ((expected).b, (actual).b)

class StructQueueTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_EQ(kOk, queue_init(&queue, MyStruct, NUM_ELEMENTS));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, queue_term(&queue));
    }

    void GivenQueueFull() {
        for (uint8_t ii = 0; ii < NUM_ELEMENTS; ++ii) {
            MyStruct element_in = MY_STRUCT_VALUE(ii);
            EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        }
    }

    void GivenQueueHas3Elements() {
        MyStruct element_in = MY_STRUCT_VALUE(42);
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        element_in = MY_STRUCT_VALUE(55);
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        element_in = MY_STRUCT_VALUE(99);
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    }

    Queue queue;
};

TEST_F(StructQueueTest, IsEmpty_GivenEmpty_ReturnsTrue) {
    EXPECT_EQ(true, queue_is_empty(&queue));
}

TEST_F(StructQueueTest, IsEmpty_GivenPartiallyFull_ReturnsFalse) {
    GivenQueueHas3Elements();

    EXPECT_EQ(false, queue_is_empty(&queue));
}

TEST_F(StructQueueTest, IsEmpty_GivenFull_ReturnsFalse) {
    GivenQueueFull();

    EXPECT_EQ(false, queue_is_empty(&queue));
}

TEST_F(StructQueueTest, IsFull_GivenEmpty_ReturnsFalse) {
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(StructQueueTest, IsFull_GivenPartiallyFull_ReturnsFalse) {
    GivenQueueHas3Elements();

    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(StructQueueTest, IsFull_GivenFull_ReturnsTrue) {
    GivenQueueFull();

    EXPECT_EQ(true, queue_is_full(&queue));
}

TEST_F(StructQueueTest, Enqueue_AddsItemToQueue) {
    MyStruct element_in;
    MyStruct *stored;

    element_in = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    stored = (MyStruct *) queue.storage;
    EXPECT_MY_STRUCT_EQ(element_in, *stored);

    element_in = MY_STRUCT_VALUE(55);
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    stored = ((MyStruct *) queue.storage) + 1;
    EXPECT_MY_STRUCT_EQ(element_in, *stored);

    element_in = MY_STRUCT_VALUE(99);
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
    stored = ((MyStruct *) queue.storage) + 2;
    EXPECT_MY_STRUCT_EQ(element_in, *stored);
}

TEST_F(StructQueueTest, Enqueue_GivenFull_Fails) {
    GivenQueueFull();

    MyStruct element_in = MY_STRUCT_VALUE(42);
    EXPECT_EQ(kContainerFull, queue_enqueue(&queue, element_in));
}

TEST_F(StructQueueTest, Dequeue_GetsItemFromQueue) {
    MyStruct element_out;
    MyStruct element_expected;
    GivenQueueHas3Elements();

    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    element_expected = MY_STRUCT_VALUE(42);
    EXPECT_MY_STRUCT_EQ(element_expected, element_out);

    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    element_expected = MY_STRUCT_VALUE(55);
    EXPECT_MY_STRUCT_EQ(element_expected, element_out);

    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    element_expected = MY_STRUCT_VALUE(99);
    EXPECT_MY_STRUCT_EQ(element_expected, element_out);

    EXPECT_EQ(kContainerEmpty, queue_dequeue(&queue, &element_out));
}

TEST_F(StructQueueTest, Size_GivenEmpty_ReturnsZero) {
    EXPECT_EQ(0, queue_size(&queue));
}

TEST_F(StructQueueTest, Size_GivenPartiallyFull_ReturnsSizeOfQueue) {
    GivenQueueHas3Elements();

    EXPECT_EQ(3, queue_size(&queue));
}

TEST_F(StructQueueTest, Size_GivenFull_ReturnsSizeOfQueue) {
    GivenQueueFull();

    EXPECT_EQ(NUM_ELEMENTS, queue_size(&queue));
}

TEST_F(StructQueueTest, Clear_GivenEmpty_Succeeds) {
    queue_clear(&queue);
    EXPECT_EQ(0, queue_size(&queue));
    EXPECT_EQ(true, queue_is_empty(&queue));
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(StructQueueTest, Clear_GivenPartiallyFull_Succeeds) {
    GivenQueueHas3Elements();

    queue_clear(&queue);
    EXPECT_EQ(0, queue_size(&queue));
    EXPECT_EQ(true, queue_is_empty(&queue));
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(StructQueueTest, Clear_GivenFull_Succeeds) {
    GivenQueueFull();

    queue_clear(&queue);
    EXPECT_EQ(0, queue_size(&queue));
    EXPECT_EQ(true, queue_is_empty(&queue));
    EXPECT_EQ(false, queue_is_full(&queue));
}

TEST_F(StructQueueTest, QueueWillNotFillIfEmptiedAtSameRate) {
    for (uint8_t ii = 0; ii < 100; ++ii) {
        MyStruct element_in = MY_STRUCT_VALUE(ii);
        EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));
        MyStruct element_out;
        EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
        EXPECT_MY_STRUCT_EQ(element_in, element_out);
    }
}

TEST_F(StructQueueTest, QueueWrapsAround) {
    GivenQueueFull();

    // Dequeue the first element.
    MyStruct element_out;
    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    MyStruct element_expected = MY_STRUCT_VALUE(0);
    EXPECT_MY_STRUCT_EQ(element_expected, element_out);

    // Enqueue another element which should cause the back pointer to wrap around.
    MyStruct element_in = MY_STRUCT_VALUE(10);
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));

    // Trying to enqueue another element should fail.
    EXPECT_EQ(kContainerFull, queue_enqueue(&queue, element_in));

    // Should be 10 elements to dequeue.
    for (uint8_t ii = 1; ii <= 10; ++ii) {
        EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
        element_expected = MY_STRUCT_VALUE(ii);
        EXPECT_MY_STRUCT_EQ(element_expected, element_out);
    }

    // Queue should now be empty.
    EXPECT_EQ(kContainerEmpty, queue_dequeue(&queue, &element_out));

    // Trying to enqueue another element should now succeed.
    element_in = MY_STRUCT_VALUE(11);
    EXPECT_EQ(kOk, queue_enqueue(&queue, element_in));

    // And we should be able to dequeue that element.
    EXPECT_EQ(kOk, queue_dequeue(&queue, &element_out));
    element_expected = MY_STRUCT_VALUE(11);
    EXPECT_MY_STRUCT_EQ(element_expected, element_out);

    // Queue should now be empty.
    EXPECT_EQ(kContainerEmpty, queue_dequeue(&queue, &element_out));
}
