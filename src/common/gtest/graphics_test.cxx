/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gmock/gmock.h>  // Needed for EXPECT_THAT.
#include <gtest/gtest.h>

extern "C" {

#include <SDL.h>

#include "../error.h"
#include "../graphics.h"
#include "../../third_party/spbmp.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
uint8_t mmb_exit_code = 0;

void CheckAbort(void) {}
int MMgetchar(void) { return 0; }
void MMgetline(int fnbr, char *p) {}

// Defined in "common/audio.c"
const char *audio_last_error() { return NULL; }

// Defined in "common/console.c"
int console_kbhit(void) { return 0; }
char console_putc(char c) { return c; }
void console_puts(const char *s) {}
void console_set_title(const char *title) {}
size_t console_write(const char *buf, size_t sz) { return 0; }

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];

// Defined in "common/serial.c"
MmResult serial_close(int fnbr) { return kError; }
int serial_eof(int fnbr) { return -1; }
int serial_getc(int fnbr) { return -1; }
int serial_putc(int fnbr, int ch) { return -1; }
int serial_rx_queue_size(int fnbr) { return -1; }
int serial_write(int fnbr, const char *buf, size_t sz) { return -1; }

// Defined in "common/spbmp.c"
void spbmp_init(
    SpBmpFileReadCb file_read_cb,
    SpBmpSetPixelCb set_pixel_cb,
    SpBmpAbortCheckCb abort_check_cb) {}
SpBmpResult spbmp_load(void *userdata, int x, int y, void *file) { return kSpBmpError; }

// Defined in "common/sprite.c"
MmResult sprite_hide(MmSurface *sprite) { return kOk; }
MmResult sprite_init() { return kOk; }
MmResult sprite_term() { return kOk; }

// Defined in "core/MMBasic.c"
const char *CurrentLinePtr = NULL;
int LocalIndex = 0;

long long int getinteger(char *p) { return 0; }
int getint(char *p, int min, int max) { return 0; }
void makeargs(const char **tp, int maxargs, char *argbuf, char *argv[], int *argc,
              const char *delim) {}

}  // extern "C"

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

static std::string format_pixels(uint32_t *pixels, uint32_t width, uint32_t height) {
    std::stringstream ss;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            ss << *pixels++ << " ";
        }
        if (y != height - 1) ss << std::endl;
    }
    return ss.str();
}

class GraphicsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        graphics_init();
        mmb_options.simulate = kSimulateMmb4l;

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

TEST_F(GraphicsTest, SpriteCreate_WithIdZero_Fails) {
    EXPECT_EQ(kGraphicsInvalidSpriteIdZero, graphics_sprite_create(0, 100, 100));
}

TEST_F(GraphicsTest, Blit_GivenNormal) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_SRC_PIXELS,
                                            sizeof(DEFAULT_SRC_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency_AndHorizontalFlip) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency_AndVerticalFlip) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency_AndHorizontalFlip_AndVerticalFlip) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenNegativeSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndNegativeSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip_AndNegativeSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndNegativeSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenPositiveSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndPositiveSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip_AndPositiveSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndPositiveSourceOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenNegativeDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndNegativeDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip_AndNegativeDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndNegativeDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenPositiveDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndPositiveDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip_AndPositiveDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndPositiveDestinationOffset) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenSourceAreaOffTopLeftOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(-10, -20, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndSourceAreaOffTopLeftOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(-10, -20, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenSourceAreaOffBottomRightOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(10, 20, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndSourceAreaOffBottomRightOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(10, 20, 0, 0, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenDestinationAreaOffTopLeftOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -10, -20, 7, 9, src, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndDestinationAreaOffTopLeftOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -10, -20, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenDestinationAreaOffBottomRightOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 10, 20, 7, 9, src, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndDestinationAreaOffBottomRightOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 10, 20, 7, 9, src, dst,
                                 kBlitHorizontalFlip | kBlitVerticalFlip, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenPartialSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency_AndPartialSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndPartialSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip_AndPartialSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizontalFlip_AndVerticalFlip_AndPartialSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWholeSurface_GivenSameSurface) {
    graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitNormal, 0);

    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, dst, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_SRC_PIXELS,
                                            sizeof(DEFAULT_SRC_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizonalFlip_AndVerticalFlip_AndWholeSurface_AndSameSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizonalFlip_AndVerticalFlip_AndPartialSurface_AndSameSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenHorizonalFlip_AndVerticalFlip_AndPositiveSourceOffset_AndSameSurface) {
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
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, GetDefaultWindowTitle_GivenNoCurrentFile) {
    char title[STRINGSIZE];
    CurrentFile[0] = '\0';

    mmb_options.simulate = kSimulateMmb4l;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 0", title);
    EXPECT_EQ(kOk, graphics_get_default_window_title(1, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 1", title);

    mmb_options.simulate = kSimulateCmm2;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Colour Maximite 2 - Mode 2", title);

    mmb_options.simulate = kSimulateMmb4w;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic for Windows - Mode 2", title);

    mmb_options.simulate = kSimulateGameMite;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Game*Mite", title);

    mmb_options.simulate = kSimulatePicoMiteVga;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("PicoMiteVGA - Mode 2", title);
}

TEST_F(GraphicsTest, GetDefaultWindowTitle_GivenCurrentFile) {
    char title[STRINGSIZE];
    snprintf(CurrentFile, STRINGSIZE, "foo/bar");

    mmb_options.simulate = kSimulateMmb4l;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 0: foo/bar", title);
    EXPECT_EQ(kOk, graphics_get_default_window_title(1, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 1: foo/bar", title);

    mmb_options.simulate = kSimulateCmm2;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Colour Maximite 2 - Mode 2: foo/bar", title);

    mmb_options.simulate = kSimulateMmb4w;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic for Windows - Mode 2: foo/bar", title);

    mmb_options.simulate = kSimulateGameMite;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Game*Mite: foo/bar", title);

    mmb_options.simulate = kSimulatePicoMiteVga;
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("PicoMiteVGA - Mode 2: foo/bar", title);

    // Title will exactly fill the buffer.
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, 30));
    EXPECT_STREQ("PicoMiteVGA - Mode 2: foo/bar", title);

    // Title is longer than the buffer.
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, 29));
    EXPECT_STREQ("PicoMiteVGA - Mode 2: foo/ba", title);
}

#define EXPECT_SURFACE_TYPE(id, type_, expected) \
    { \
        char out[STRINGSIZE]; \
        graphics_surfaces[id].type = type_; \
        EXPECT_EQ(kOk, graphics_type_as_string(&graphics_surfaces[id], out, STRINGSIZE)); \
        EXPECT_STREQ(expected, out); \
    }

TEST_F(GraphicsTest, TypeAsString_GivenNotSimulatingOther) {
    mmb_options.simulate = kSimulateMmb4l;

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_SURFACE_TYPE(id, kGraphicsNone, "None");
        EXPECT_SURFACE_TYPE(id, kGraphicsWindow, "Window");
        EXPECT_SURFACE_TYPE(id, kGraphicsBuffer, "Buffer");
        EXPECT_SURFACE_TYPE(id, kGraphicsSprite, "Sprite (Active)");
        EXPECT_SURFACE_TYPE(id, kGraphicsInactiveSprite, "Sprite (Inactive)");
    }
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingCmm2) {
    mmb_options.simulate = kSimulateCmm2;

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsWindow, "Page 63");

    // Window with id >= 64 is a "Window"
    EXPECT_SURFACE_TYPE(64, kGraphicsWindow, "Window");

    // Buffer with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsBuffer, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Page 63");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingMmb4w) {
    mmb_options.simulate = kSimulateMmb4w;

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsWindow, "Page 63");

    // Window with id >= 64 is a "Window"
    EXPECT_SURFACE_TYPE(64, kGraphicsWindow, "Window");

    // Buffer with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsBuffer, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Page 63");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingGameMite) {
    mmb_options.simulate = kSimulateGameMite;

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with id == 0 is the "Display"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Display");

    // Buffer with id == 1 is "Buffer N"
    EXPECT_SURFACE_TYPE(1, kGraphicsBuffer, "Buffer N");

    // Buffer with id == 2 is "Buffer F"
    EXPECT_SURFACE_TYPE(2, kGraphicsBuffer, "Buffer F");

    // Buffer with id == 3 is "Buffer L"
    EXPECT_SURFACE_TYPE(3, kGraphicsBuffer, "Buffer L");

    // Window with id >= 1 is a "Window"
    EXPECT_SURFACE_TYPE(1, kGraphicsWindow, "Window");

    // Buffer with 4 <= id <= 63 is a "Buffer"
    EXPECT_SURFACE_TYPE(4, kGraphicsBuffer, "Buffer");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Buffer");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingPicoMiteVga) {
    mmb_options.simulate = kSimulatePicoMiteVga;

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with id == 0 is the "Display"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Display");

    // Buffer with id == 1 is "Buffer N"
    EXPECT_SURFACE_TYPE(1, kGraphicsBuffer, "Buffer N");

    // Buffer with id == 2 is "Buffer F"
    EXPECT_SURFACE_TYPE(2, kGraphicsBuffer, "Buffer F");

    // Buffer with id == 3 is "Buffer L"
    EXPECT_SURFACE_TYPE(3, kGraphicsBuffer, "Buffer L");

    // Window with id >= 1 is a "Window"
    EXPECT_SURFACE_TYPE(1, kGraphicsWindow, "Window");

    // Buffer with 4 <= id <= 63 is a "Buffer"
    EXPECT_SURFACE_TYPE(4, kGraphicsBuffer, "Buffer");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Buffer");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}
