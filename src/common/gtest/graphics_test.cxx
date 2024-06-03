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

char error_msg[256];

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
char **FontTable;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
uint8_t mmb_exit_code = 0;

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

// Defined in "common/error.c"
void error_init(ErrorState *error_state) {}
MmResult error_throw(MmResult result) { return error_throw_ex(result, mmresult_to_string(result)); }
MmResult error_throw_ex(MmResult result, const char *msg, ...) {
    strcpy(error_msg, msg);
    return result;
}
MmResult error_throw_legacy(const char *msg, ...) { return error_throw_ex(kError, msg); }

// Defined in "common/image_bmp.c"
uint8_t image_bmp_load(MmSurface *surface, int x, int y, int fnbr) { return -1; }

// Defined in "common/interrupt.c"
void interrupt_disable_serial_rx(int fnbr) {}
void interrupt_enable_serial_rx(int fnbr, int64_t count, const char *interrupt_addr) {}
void interrupt_fire_window_close(MmSurfaceId id) {}

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/serial.c"
MmResult serial_close(int fnbr) { return kError; }
int serial_eof(int fnbr) { return -1; }
int serial_getc(int fnbr) { return -1; }
int serial_putc(int fnbr, int ch) { return -1; }
int serial_rx_queue_size(int fnbr) { return -1; }
int serial_write(int fnbr, const char *buf, size_t sz) { return -1; }

// Defined in "common/sprite.c"
MmResult sprite_init() { return kOk; }
MmResult sprite_term() { return kOk; }

// Defined in "core/MMBasic.c"
int LocalIndex = 0;

long long int getinteger(char *p) { return 0; }
int getint(char *p, int min, int max) { return 0; }
void makeargs(const char **tp, int maxargs, char *argbuf, char *argv[], int *argc,
              const char *delim) {}

}  // extern "C"

// clang-format off
static const uint32_t DEFAULT_SRC_PIXELS[] = {
    0, 0, 1, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0,
    0, 0, 3, 0, 0, 0, 0,
    4, 5, 6, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0 };

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

        const MmSurfaceId srcId = 1;
        EXPECT_EQ(kOk, graphics_buffer_create(srcId, 7, 9));
        src = &graphics_surfaces[srcId];
        memcpy(src->pixels, DEFAULT_SRC_PIXELS, sizeof(DEFAULT_SRC_PIXELS));

        const MmSurfaceId dstId = 2;
        EXPECT_EQ(kOk, graphics_buffer_create(dstId, 7, 9));
        dst = &graphics_surfaces[dstId];
        memcpy(dst->pixels, DEFAULT_DST_PIXELS, sizeof(DEFAULT_DST_PIXELS));
    }

    void TearDown() override { graphics_term(); }

    MmSurface *src;
    MmSurface *dst;
};

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
        0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 2, 0, 0,
        0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 6, 5, 4,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0 };
    // clang-format on
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenVerticalFlip) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitVerticalFlip, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        4, 5, 6, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0,
        0, 0, 2, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0 };
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
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 6, 5, 4,
        0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 2, 0, 0,
        0, 0, 0, 0, 1, 0, 0 };
    // clang-format on
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 0, 0, 7, 9, src, dst, kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 1, 9, 9, 9, 9,
        9, 9, 2, 9, 9, 9, 9,
        9, 9, 3, 9, 9, 9, 9,
        4, 5, 6, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
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
        9, 9, 9, 9, 1, 9, 9,
        9, 9, 9, 9, 2, 9, 9,
        9, 9, 9, 9, 3, 9, 9,
        9, 9, 9, 9, 6, 5, 4,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
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
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        4, 5, 6, 9, 9, 9, 9,
        9, 9, 3, 9, 9, 9, 9,
        9, 9, 2, 9, 9, 9, 9,
        9, 9, 1, 9, 9, 9, 9 };
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
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 6, 5, 4,
        9, 9, 9, 9, 3, 9, 9,
        9, 9, 9, 9, 2, 9, 9,
        9, 9, 9, 9, 1, 9, 9 };
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
        9, 9, 0, 0, 1, 0, 0,
        9, 9, 0, 0, 2, 0, 0,
        9, 9, 0, 0, 3, 0, 0,
        9, 9, 4, 5, 6, 0, 0,
        9, 9, 0, 0, 0, 0, 0,
        9, 9, 0, 0, 0, 0, 0 };
    // clang-format on
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenPositiveSourceOffset) {
    EXPECT_EQ(kOk, graphics_blit(1, 2, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 3, 0, 0, 0, 0, 9,
        5, 6, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenNegativeDestinationOffset) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, -1, -2, 7, 9, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        0, 3, 0, 0, 0, 0, 9,
        5, 6, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 0, 0, 9,
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
        9, 9, 0, 0, 1, 0, 0,
        9, 9, 0, 0, 2, 0, 0,
        9, 9, 0, 0, 3, 0, 0,
        9, 9, 4, 5, 6, 0, 0,
        9, 9, 0, 0, 0, 0, 0,
        9, 9, 0, 0, 0, 0, 0 };
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

TEST_F(GraphicsTest, Blit_GivenSourceAreaOffBottomRightOfSourceSurface) {
    EXPECT_EQ(kOk, graphics_blit(10, 20, 0, 0, 7, 9, src, dst, kBlitNormal, 0));

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

TEST_F(GraphicsTest, Blit_GivenDestinationAreaOffBottomRightOfDestinationSurface) {
    EXPECT_EQ(kOk, graphics_blit(0, 0, 10, 20, 7, 9, src, dst, kBlitNormal, 0));

    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(DEFAULT_DST_PIXELS,
                                            sizeof(DEFAULT_DST_PIXELS) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenPartOfSource) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 4, 4, 2, 3, src, dst, kBlitNormal, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 0, 2, 9,
        9, 9, 9, 9, 0, 3, 9,
        9, 9, 9, 9, 5, 6, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}

TEST_F(GraphicsTest, Blit_GivenWithTransparency_AndPartOfSource) {
    EXPECT_EQ(kOk, graphics_blit(1, 1, 4, 4, 2, 3, src, dst, kBlitWithTransparency, 0));

    // clang-format off
    const uint32_t expected[] = {
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 2, 9,
        9, 9, 9, 9, 9, 3, 9,
        9, 9, 9, 9, 5, 6, 9,
        9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9 };
    // clang-format on
    EXPECT_THAT(std::vector<uint32_t>(dst->pixels, dst->pixels + dst->width * dst->height),
                ::testing::ElementsAreArray(expected, sizeof(expected) / sizeof(uint32_t)))
        << format_pixels(dst->pixels, dst->width, dst->height);
}
