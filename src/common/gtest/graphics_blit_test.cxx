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
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);

        const MmSurfaceId srcId = 1;
        EXPECT_EQ(kOk, graphics_buffer_create(srcId, 7, 9));
        src = &graphics_surfaces[srcId];
        memcpy(src->pixels, DEFAULT_SRC_PIXELS, sizeof(DEFAULT_SRC_PIXELS));

        const MmSurfaceId dstId = 2;
        EXPECT_EQ(kOk, graphics_buffer_create(dstId, 7, 9));
        dst = &graphics_surfaces[dstId];
        memcpy(dst->pixels, DEFAULT_DST_PIXELS, sizeof(DEFAULT_DST_PIXELS));
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }

    MmSurface *src;
    MmSurface *dst;
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
