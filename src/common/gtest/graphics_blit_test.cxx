/*
 * Copyright (c) 2024-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gmock/gmock.h>  // Needed for EXPECT_THAT.
#include <gtest/gtest.h>

extern "C" {

#include <SDL.h>

#include "../features.h"
#include "../graphics.h"
#include "../logger.h"
#include "../memory.h"

extern Features mmb_features;

}  // extern "C"

#define EXPECT_PIXELS_EQ(expected, actual, width, height) \
    EXPECT_EQ(0, memcmp(expected, actual, sizeof(uint32_t) * width * height)) \
        << "Actual pixels:   " << std::endl << format_pixels(actual, width, height) << std::endl \
        << "Expected pixels: " << std::endl << format_pixels(expected, width, height)

// clang-format off
static const uint32_t DEFAULT_SRC_PIXELS[] = {
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    4, 4, 4, 5, 2, 2, 2,
    0, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 3, 0, 0, 0 };

static const uint32_t DEFAULT_DST_PIXELS[] = {
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9 };

const uint32_t SRC_2_PIXELS[] = {
    1, 2,
    3, 4 };
// clang-format on

static std::string format_pixels(const uint32_t *pixels, uint32_t width, uint32_t height) {
    std::stringstream ss;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            ss << *pixels++ << " ";
        }
        if (y != height - 1) ss << std::endl;
    }
    return ss.str();
}

class GraphicsBlitTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(kOk, memory_init());

        // logger_init("");
        ASSERT_EQ(kOk, graphics_init());
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);

        const MmSurfaceId srcId = 1;
        EXPECT_EQ(kOk, graphics_buffer_create(srcId, 7, 9));
        src = &graphics_surfaces[srcId];
        memcpy(src->pixels, DEFAULT_SRC_PIXELS, sizeof(DEFAULT_SRC_PIXELS));

        const MmSurfaceId dstId = 2;
        EXPECT_EQ(kOk, graphics_buffer_create(dstId, 7, 9));
        dst = &graphics_surfaces[dstId];
        memcpy(dst->pixels, DEFAULT_DST_PIXELS, sizeof(DEFAULT_DST_PIXELS));

        const MmSurfaceId src2Id = 3;
        EXPECT_EQ(kOk, graphics_buffer_create(src2Id, 2, 5));
        src2 = &graphics_surfaces[src2Id];
        memcpy(src2->pixels, SRC_2_PIXELS, sizeof(SRC_2_PIXELS));
    }

    void TearDown() override {
        ASSERT_EQ(kOk, graphics_term());
        ASSERT_EQ(kOk, memory_term());
    }

    MmSurface *dst;
    MmSurface *src;
    MmSurface *src2;
};

TEST_F(GraphicsBlitTest, GivenNormal) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    const uint32_t *expected = DEFAULT_SRC_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitHorizontalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        2, 2, 2, 5, 4, 4, 4,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        4, 4, 4, 5, 2, 2, 2,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip) {
    EXPECT_EQ(
        kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        2, 2, 2, 5, 4, 4, 4,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenWithTransparency) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        4, 4, 4, 5, 2, 2, 2,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenWithTransparency_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        2, 2, 2, 5, 4, 4, 4,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenWithTransparency_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst,
                                 kBlitVerticalFlip | kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        4, 4, 4, 5, 2, 2, 2,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenWithTransparency_AndHorizontalFlip_AndVerticalFlip) {
    EXPECT_EQ(kOk,
              graphics_blit(0, 0, 0, 0, 7, 9, src, dst,
                            kBlitHorizontalFlip | kBlitVerticalFlip | kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        2, 2, 2, 5, 4, 4, 4,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenNegativeSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(-2, -3, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 4, 4, 4, 5, 2,
        9, 9, 0, 0, 0, 3, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndNegativeSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(-2, -3, 0, 0, 7, 9, src, dst, kBlitHorizontalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        2, 5, 4, 4, 4, 9, 9,
        0, 3, 0, 0, 0, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenVerticalFlip_AndNegativeSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(-2, -3, 0, 0, 7, 9, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 4, 4, 4, 5, 2,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndNegativeSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(-2, -3, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 3, 0, 0, 0, 9, 9,
        2, 5, 4, 4, 4, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        0, 1, 0, 0, 0, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenPositiveSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(1, 2, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        4, 4, 5, 2, 2, 2, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndPositiveSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(1, 2, 0, 0, 7, 9, src, dst, kBlitHorizontalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 0, 0, 0, 1, 0, 0,
        9, 0, 0, 0, 1, 0, 0,
        9, 2, 2, 2, 5, 4, 4,
        9, 0, 0, 0, 3, 0, 0,
        9, 0, 0, 0, 3, 0, 0,
        9, 0, 0, 0, 3, 0, 0,
        9, 0, 0, 0, 3, 0, 0,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenVerticalFlip_AndPositiveSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(1, 2, 0, 0, 7, 9, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        4, 4, 5, 2, 2, 2, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndPositiveSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(1, 2, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 0, 0, 0, 3, 0, 0,
        9, 0, 0, 0, 3, 0, 0,
        9, 0, 0, 0, 3, 0, 0,
        9, 0, 0, 0, 3, 0, 0,
        9, 2, 2, 2, 5, 4, 4,
        9, 0, 0, 0, 1, 0, 0,
        9, 0, 0, 0, 1, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenNegativeDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, -2, 7, 9, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        4, 4, 5, 2, 2, 2, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndNegativeDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, -2, 7, 9, src, dst, kBlitHorizontalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        2, 2, 5, 4, 4, 4, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenVerticalFlip_AndNegativeDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, -2, 7, 9, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        4, 4, 5, 2, 2, 2, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndNegativeDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, -2, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 3, 0, 0, 0, 9,
        0, 0, 3, 0, 0, 0, 9,
        2, 2, 5, 4, 4, 4, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        0, 0, 1, 0, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenPositiveDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 3, 7, 9, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 4, 4, 4, 5, 2,
        9, 9, 0, 0, 0, 3, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndPositiveDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 3, 7, 9, src, dst, kBlitHorizontalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 0, 0, 0, 1, 0,
        9, 9, 2, 2, 2, 5, 4,
        9, 9, 0, 0, 0, 3, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenVerticalFlip_AndPositiveDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 3, 7, 9, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 4, 4, 4, 5, 2,
        9, 9, 0, 0, 0, 1, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndPositiveDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 3, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 0, 0, 0, 3, 0,
        9, 9, 2, 2, 2, 5, 4,
        9, 9, 0, 0, 0, 1, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSourceAreaOffTopLeftOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(-10, -20, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndSourceAreaOffTopLeftOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(-10, -20, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSourceAreaOffBottomRightOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(10, 20, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndSourceAreaOffBottomRightOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(10, 20, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenDestinationAreaOffTopLeftOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -10, -20, 7, 9, src, dst, kBlitNormal, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndDestinationAreaOffTopLeftOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -10, -20, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenDestinationAreaOffBottomRightOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 10, 20, 7, 9, src, dst, kBlitNormal, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndDestinationAreaOffBottomRightOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 10, 20, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    const uint32_t *expected = DEFAULT_DST_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenPartialSurface) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 2, 3, 4, 5, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 1, 0, 9,
        9, 9, 0, 0, 1, 0, 9,
        9, 9, 0, 0, 1, 0, 9,
        9, 9, 4, 4, 5, 2, 9,
        9, 9, 0, 0, 3, 0, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenWithTransparency_AndPartialSurface) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 2, 3, 4, 5, src, dst, kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 1, 9, 9,
        9, 9, 9, 9, 1, 9, 9,
        9, 9, 9, 9, 1, 9, 9,
        9, 9, 4, 4, 5, 2, 9,
        9, 9, 9, 9, 3, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndPartialSurface) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 2, 3, 4, 5, src, dst, kBlitHorizontalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 1, 0, 0, 9,
        9, 9, 0, 1, 0, 0, 9,
        9, 9, 0, 1, 0, 0, 9,
        9, 9, 2, 5, 4, 4, 9,
        9, 9, 0, 3, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenVerticalFlip_AndPartialSurface) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 2, 3, 4, 5, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 0, 3, 0, 9,
        9, 9, 4, 4, 5, 2, 9,
        9, 9, 0, 0, 1, 0, 9,
        9, 9, 0, 0, 1, 0, 9,
        9, 9, 0, 0, 1, 0, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizontalFlip_AndVerticalFlip_AndPartialSurface) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 2, 3, 4, 5, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 0, 3, 0, 0, 9,
        9, 9, 2, 5, 4, 4, 9,
        9, 9, 0, 1, 0, 0, 9,
        9, 9, 0, 1, 0, 0, 9,
        9, 9, 0, 1, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenWholeSurface_GivenSameSurface) {
    graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0);

    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, dst, dst, kBlitNormal, 0));

    const uint32_t *expected = DEFAULT_SRC_PIXELS;
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizonalFlip_AndVerticalFlip_AndWholeSurface_AndSameSurface) {
    graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0);

    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, dst, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        2, 2, 2, 5, 4, 4, 4,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizonalFlip_AndVerticalFlip_AndPartialSurface_AndSameSurface) {
    graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0);

    EXPECT_EQ(kOk, graphics_blit(2, 2, 2, 2, 3, 5, dst, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        4, 4, 2, 5, 4, 2, 2,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenHorizonalFlip_AndVerticalFlip_AndPositiveSourceOffset_AndSameSurface) {
    graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0);
    EXPECT_EQ(kOk, graphics_blit(1, 2, 0, 0, 7, 9, dst, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0,
        4, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0,
        0, 2, 2, 2, 5, 4, 4,
        0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtEastEdge) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 5, 3, 2, 2, src2, dst, 0x0, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 1, 2,
        9, 9, 9, 9, 9, 3, 4,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtEastEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 5, 3, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 2, 1,
        9, 9, 9, 9, 9, 4, 3,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingEastEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 6, 3, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 2,
        9, 9, 9, 9, 9, 9, 4,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondEastEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 7, 3, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtEastEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 5, 3, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 3, 4,
        9, 9, 9, 9, 9, 1, 2,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingEastEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 6, 3, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 3,
        9, 9, 9, 9, 9, 9, 1,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondEastEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 7, 3, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtWestEdge) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 3, 2, 2, src2, dst, 0x0, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        1, 2, 9, 9, 9, 9, 9,
        3, 4, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtWestEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 3, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        2, 1, 9, 9, 9, 9, 9,
        4, 3, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingWestEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, 3, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        1, 9, 9, 9, 9, 9, 9,
        3, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondWestEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -2, 3, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtWestEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 3, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        3, 4, 9, 9, 9, 9, 9,
        1, 2, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingWestEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, 3, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        4, 9, 9, 9, 9, 9, 9,
        2, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondWestEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -2, 3, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtNorthEdge) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 0, 2, 2, src2, dst, 0x0, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 1, 2, 9, 9, 9,
        9, 9, 3, 4, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtNorthEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 0, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 2, 1, 9, 9, 9,
        9, 9, 4, 3, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingNorthEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, -1, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 4, 3, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondNorthEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, -2, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtNorthEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 0, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 3, 4, 9, 9, 9,
        9, 9, 1, 2, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingNorthEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, -1, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 1, 2, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondNorthEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, -2, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtSouthEdge) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 7, 2, 2, src2, dst, 0x0, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 1, 2, 9, 9, 9,
        9, 9, 3, 4, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtSouthEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 7, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 2, 1, 9, 9, 9,
        9, 9, 4, 3, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingSouthEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 8, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 2, 1, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondSouthEdge_AndHorizontalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 9, 2, 2, src2, dst, kBlitHorizontalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_AtSouthEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 7, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 3, 4, 9, 9, 9,
        9, 9, 1, 2, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_OverlappingSouthEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 8, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 3, 4, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsBlitTest, GivenSmallSource_BeyondSouthEdge_AndVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 2, 9, 2, 2, src2, dst, kBlitVerticalFlip, -1));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

// Define inverted color constants
// Inversion formula: inverted = color ^ 0xFFFFFF (inverts RGB only, preserves alpha)
#define RGB_INVERTED_0  0x00FFFFFF  // 0 inverted (RGB only)
#define RGB_INVERTED_1  0x00FFFFFE  // 1 inverted (RGB only)
#define RGB_INVERTED_2  0x00FFFFFD  // 2 inverted (RGB only)
#define RGB_INVERTED_3  0x00FFFFFC  // 3 inverted (RGB only)
#define RGB_INVERTED_4  0x00FFFFFB  // 4 inverted (RGB only)
#define RGB_INVERTED_5  0x00FFFFFA  // 5 inverted (RGB only)
#define RGB_INVERTED_9  0x00FFFFF6  // 9 inverted (RGB only)

// Test the standard XOR inversion behavior
TEST_F(GraphicsBlitTest, GivenInvert_AndSameSurface) {
    // Save current colours
    const MmGraphicsColour saved_bcolour = graphics_bcolour;
    const MmGraphicsColour saved_fcolour = graphics_fcolour;

    // Set foreground/background to values NOT in our test data to avoid swap behavior
    // This tests pure XOR inversion
    graphics_fcolour = 99;
    graphics_bcolour = 88;

    // Copy source to destination first
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    // Now invert in place
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, dst, dst, kBlitInvert, 0));

    // clang-format off
    // Expected result assumes graphics_fcolour and graphics_bcolour are NOT
    // equal to any of the pixel values (0, 1, 2, 3, 4, 5) in our test data.
    // If they are, these values would need to be adjusted for the swap behavior.
    const uint32_t expected[] = {
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_4, RGB_INVERTED_4, RGB_INVERTED_4, RGB_INVERTED_5, RGB_INVERTED_2, RGB_INVERTED_2, RGB_INVERTED_2,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0 };
    // clang-format on

    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);

    // Restore original colours
    graphics_bcolour = saved_bcolour;
    graphics_fcolour = saved_fcolour;
}

// Test the special swap behavior when pixels match graphics_fcolour or graphics_bcolour
TEST_F(GraphicsBlitTest, GivenInvert_WithForegroundBackgroundSwap) {
    // Save current colours
    MmGraphicsColour saved_fcolour = graphics_fcolour;
    MmGraphicsColour saved_bcolour = graphics_bcolour;

    // Set foreground/background to values that exist in our test data
    // DEFAULT_SRC_PIXELS contains: 0, 1, 2, 3, 4, 5
    graphics_fcolour = 1;  // Foreground color
    graphics_bcolour = 3;  // Background color

    // Perform the invert blit
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitInvert, 0));

    // clang-format off
    // Expected behavior:
    // - All pixels with value 1 (fcolour) become 3 (bcolour)
    // - All pixels with value 3 (bcolour) become 1 (fcolour)
    // - All other pixels (0, 2, 4, 5) get XOR inverted
    const uint32_t expected[] = {
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 3, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_4, RGB_INVERTED_4, RGB_INVERTED_4, RGB_INVERTED_5, RGB_INVERTED_2, RGB_INVERTED_2, RGB_INVERTED_2,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0,
        RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, 1, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);

    // Restore original colours
    graphics_bcolour = saved_bcolour;
    graphics_fcolour = saved_fcolour;
}

// Test inversion with both flips and translation (1 pixel right, 2 pixels down)
// This ensures the invert logic works correctly with all the transformation flags
TEST_F(GraphicsBlitTest, GivenInvert_AndHorizontalFlip_AndVerticalFlip_AndTranslation) {
    // Save current colours
    MmGraphicsColour saved_fcolour = graphics_fcolour;
    MmGraphicsColour saved_bcolour = graphics_bcolour;

    // Set foreground/background to values NOT in our test data to avoid swap behavior
    // This tests pure XOR inversion
    graphics_fcolour = 99;
    graphics_bcolour = 88;

    // Blit with translation (1 right, 2 down), both flips, and inversion
    EXPECT_EQ(kOk, graphics_blit(0, 0, 1, 2, 7, 9, src, dst,
                                 kBlitInvert | kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    // clang-format off
    // Expected: source is flipped horizontally and vertically, translated to (1, 2),
    // and all pixels are XOR inverted
    //
    // Original source (column 3 emphasized):
    //   0 0 0 | 1 | 0 0 0    row 0
    //   0 0 0 | 1 | 0 0 0    row 1
    //   0 0 0 | 1 | 0 0 0    row 2
    //   0 0 0 | 1 | 0 0 0    row 3
    //   4 4 4 | 5 | 2 2 2    row 4 (middle)
    //   0 0 0 | 3 | 0 0 0    row 5
    //   0 0 0 | 3 | 0 0 0    row 6
    //   0 0 0 | 3 | 0 0 0    row 7
    //   0 0 0 | 3 | 0 0 0    row 8
    //
    // After horizontal flip (mirror left-right):
    //   0 0 0 | 1 | 0 0 0
    //   0 0 0 | 1 | 0 0 0
    //   0 0 0 | 1 | 0 0 0
    //   0 0 0 | 1 | 0 0 0
    //   2 2 2 | 5 | 4 4 4
    //   0 0 0 | 3 | 0 0 0
    //   0 0 0 | 3 | 0 0 0
    //   0 0 0 | 3 | 0 0 0
    //   0 0 0 | 3 | 0 0 0
    //
    // After vertical flip (mirror top-bottom):
    //   0 0 0 | 3 | 0 0 0
    //   0 0 0 | 3 | 0 0 0
    //   0 0 0 | 3 | 0 0 0
    //   0 0 0 | 3 | 0 0 0
    //   2 2 2 | 5 | 4 4 4
    //   0 0 0 | 1 | 0 0 0
    //   0 0 0 | 1 | 0 0 0
    //   0 0 0 | 1 | 0 0 0
    //   0 0 0 | 1 | 0 0 0
    //
    // After translation by (1, 2) and XOR inversion:
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,  // row 0: untouched (above translation)
        9, 9, 9, 9, 9, 9, 9,  // row 1: untouched (above translation)
        9, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0,  // row 2: first row of flipped data
        9, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0,  // row 3
        9, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0,  // row 4
        9, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_3, RGB_INVERTED_0, RGB_INVERTED_0,  // row 5
        9, RGB_INVERTED_2, RGB_INVERTED_2, RGB_INVERTED_2, RGB_INVERTED_5, RGB_INVERTED_4, RGB_INVERTED_4,  // row 6: middle row
        9, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_1, RGB_INVERTED_0, RGB_INVERTED_0,  // row 7
        9, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_0, RGB_INVERTED_1, RGB_INVERTED_0, RGB_INVERTED_0   // row 8
    };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);

    // Restore original colours
    graphics_fcolour = saved_fcolour;
    graphics_bcolour = saved_bcolour;
}

class GraphicsBlitResizeTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(kOk, memory_init());
        ASSERT_EQ(kOk, graphics_init());
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);

        // 7x9 source surface with a cross-hair pattern of distinct values.
        const MmSurfaceId srcId = 1;
        ASSERT_EQ(kOk, graphics_buffer_create(srcId, 7, 9));
        src = &graphics_surfaces[srcId];
        memcpy(src->pixels, DEFAULT_SRC_PIXELS, sizeof(DEFAULT_SRC_PIXELS));

        // 7x9 destination surface pre-filled with 9s so untouched pixels are
        // easy to distinguish from copied ones.
        const MmSurfaceId dstId = 2;
        ASSERT_EQ(kOk, graphics_buffer_create(dstId, 7, 9));
        dst = &graphics_surfaces[dstId];
        memcpy(dst->pixels, DEFAULT_DST_PIXELS, sizeof(DEFAULT_DST_PIXELS));

        // Small 2x2 surface for scale-up tests.
        const MmSurfaceId src2Id = 3;
        ASSERT_EQ(kOk, graphics_buffer_create(src2Id, 2, 2));
        src2 = &graphics_surfaces[src2Id];
        memcpy(src2->pixels, SRC_2_PIXELS, sizeof(SRC_2_PIXELS));
    }

    void TearDown() override {
        ASSERT_EQ(kOk, graphics_term());
        ASSERT_EQ(kOk, memory_term());
    }

    MmSurface *dst;
    MmSurface *src;
    MmSurface *src2;
};

// Blit with identical src/dst dimensions — should be a pixel-exact copy.
TEST_F(GraphicsBlitResizeTest, GivenSameDimensions_CopiesExactly) {
    EXPECT_EQ(kOk, graphics_blit_resize(
        src, 0, 0, 7, 9,
        dst, 0, 0, 7, 9,
        NO_TRANSPARENCY));

    EXPECT_PIXELS_EQ(DEFAULT_SRC_PIXELS, dst->pixels, dst->width, dst->height);
}

// Scale a 2x2 source up to 4x4 using nearest-neighbour sampling.
TEST_F(GraphicsBlitResizeTest, GivenScaleUp_UsesNearestNeighbour) {
    const MmSurfaceId outId = 4;
    ASSERT_EQ(kOk, graphics_buffer_create(outId, 4, 4));
    MmSurface *out = &graphics_surfaces[outId];

    EXPECT_EQ(kOk, graphics_blit_resize(
        src2, 0, 0, 2, 2,
        out,  0, 0, 4, 4,
        NO_TRANSPARENCY));

    // clang-format off
    const uint32_t expected[] = {
        1, 1, 2, 2,
        1, 1, 2, 2,
        3, 3, 4, 4,
        3, 3, 4, 4 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, out->pixels, out->width, out->height);
}

// Scale the full 7x9 source down to 3x4 — nearest-neighbour sampling should
// preserve the cross-hair pattern of distinct values.
TEST_F(GraphicsBlitResizeTest, GivenScaleDown_SamplesCorrectly) {
    const MmSurfaceId outId = 4;
    ASSERT_EQ(kOk, graphics_buffer_create(outId, 3, 4));
    MmSurface *out = &graphics_surfaces[outId];

    EXPECT_EQ(kOk, graphics_blit_resize(
        src, 0, 0, 7, 9,
        out, 0, 0, 3, 4,
        NO_TRANSPARENCY));

    // With nearest-neighbour, output pixel (x, y) samples source pixel
    // (floor(x * 7/3), floor(y * 9/4)):
    //   (0,0) -> (0,0)=0  (1,0) -> (2,0)=0  (2,0) -> (4,0)=0
    //   (0,1) -> (0,2)=0  (1,1) -> (2,2)=0  (2,1) -> (4,2)=0
    //   (0,2) -> (0,4)=4  (1,2) -> (2,4)=4  (2,2) -> (4,4)=2
    //   (0,3) -> (0,6)=0  (1,3) -> (2,6)=0  (2,3) -> (4,6)=0
    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0,
        0, 0, 0,
        4, 4, 2,
        0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, out->pixels, out->width, out->height);
}

// Pixels matching the transparent colour must not overwrite the destination.
TEST_F(GraphicsBlitResizeTest, GivenTransparency_SkipsMatchingPixels) {
    EXPECT_EQ(kOk, graphics_blit_resize(
        src, 0, 0, 7, 9,
        dst, 0, 0, 7, 9,
        0 /* transparent colour = 0 */));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        9, 9, 9, 1, 9, 9, 9,
        4, 4, 4, 5, 2, 2, 2,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9,
        9, 9, 9, 3, 9, 9, 9 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, dst->pixels, dst->width, dst->height);
}

// Blit a sub-region of the source rather than the whole surface.
TEST_F(GraphicsBlitResizeTest, GivenSourceSubRegion_CopiesCorrectRegion) {
    const MmSurfaceId outId = 4;
    ASSERT_EQ(kOk, graphics_buffer_create(outId, 3, 4));
    MmSurface *out = &graphics_surfaces[outId];

    // Grab the right half of DEFAULT_SRC_PIXELS (columns 4-6, rows 5-8),
    // which is all 0s except for the first column which is 2.
    EXPECT_EQ(kOk, graphics_blit_resize(
        src, 4, 5, 3, 4,
        out, 0, 0, 3, 4,
        NO_TRANSPARENCY));

    // clang-format off
    const uint32_t expected_corrected[] = {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected_corrected, out->pixels, out->width, out->height);
}

// When src and dst are the same surface and regions do not overlap,
// the result should still be correct.
TEST_F(GraphicsBlitResizeTest, GivenSameSurface_NonOverlapping_CopiesCorrectly) {
    // Copy the left half (columns 0-2) of src into its right half (columns 4-6),
    // same height — no overlap.
    EXPECT_EQ(kOk, graphics_blit_resize(
        src, 0, 0, 3, 9,
        src, 4, 0, 3, 9,
        NO_TRANSPARENCY));

    // Column 3 (the vertical bar of 1s/3s/5) is untouched.
    // Columns 4-6 should now mirror columns 0-2 (all 0s except row 4 = 4,4,4).
    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        4, 4, 4, 5, 4, 4, 4,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, src->pixels, src->width, src->height);
}

// When src and dst are the same surface and regions overlap, the temporary
// surface path should still produce a correct result.
TEST_F(GraphicsBlitResizeTest, GivenSameSurface_Overlapping_CopiesCorrectly) {
    // Shift the entire surface one column to the right (overlapping by 6 cols).
    EXPECT_EQ(kOk, graphics_blit_resize(
        src, 0, 0, 6, 9,
        src, 1, 0, 6, 9,
        NO_TRANSPARENCY));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0,
        4, 4, 4, 4, 5, 2, 2,
        0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0 };
    // clang-format on
    EXPECT_PIXELS_EQ(expected, src->pixels, src->width, src->height);
}
