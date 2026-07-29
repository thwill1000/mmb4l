/*
 * Copyright (c) 2026-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {

#include <SDL.h>

#include "../image_private.h"
#include "../features.h"
#include "../system.h"
#include "../../common/gtest/stubs/error_stubs.h"
#include "../../core/Commands.h"
#include "../../core/gtest/command_stubs.h"
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"
#include "../../third_party/picojpeg.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Features mmb_features;
Options mmb_options;
ErrorState mmb_normal_error_state;

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/gpio.c"
MmResult gpio_term() { return kOk; }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/mmgetline.c"
void MMgetline(int fnbr, char *p) {}

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) {
    *ch = -1;
    return kOk;
}

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];

}

// ---------------------------------------------------------------------------
// image_bin_row()
// ---------------------------------------------------------------------------

class ImageBinRowTest : public ::testing::Test {};

TEST_F(ImageBinRowTest, GivenScale1_CopiesPixelsUnchanged) {
    // 3x1 source row, RGB triplets.
    const uint8_t src[9] = { 10, 20, 30,  40, 50, 60,  70, 80, 90 };
    uint8_t out[9] = { 0 };

    image_bin_row(src, /* mcu_row_width */ 9, /* rows_available */ 1,
                 /* src_x_start */ 0, /* src_width */ 3, /* scale */ 1,
                 /* out_width */ 3, out);

    EXPECT_EQ(0, memcmp(src, out, sizeof(src)));
}

TEST_F(ImageBinRowTest, GivenScale2FullBlock_AveragesFourPixels) {
    // 2x2 block: (0,0)=(0,0,0) (1,0)=(10,10,10)
    //            (0,1)=(20,20,20) (1,1)=(30,30,30)
    // Row-major, row stride = 2 pixels * 3 bytes = 6.
    uint8_t src[12] = {
        0, 0, 0,     10, 10, 10,   // row 0
        20, 20, 20,  30, 30, 30,   // row 1
    };
    uint8_t out[3] = { 0 };

    image_bin_row(src, /* mcu_row_width */ 6, /* rows_available */ 2,
                 /* src_x_start */ 0, /* src_width */ 2, /* scale */ 2,
                 /* out_width */ 1, out);

    // Average of 0,10,20,30 = 60/4 = 15 exactly.
    EXPECT_EQ(15, out[0]);
    EXPECT_EQ(15, out[1]);
    EXPECT_EQ(15, out[2]);
}

TEST_F(ImageBinRowTest, RoundsToNearest_NotTruncated) {
    // Average of 0,0,0,1 = 1/4 = 0.25 -> rounds down to 0.
    // Average of 0,1,1,1 = 3/4 = 0.75 -> rounds up to 1.
    uint8_t src_round_down[12] = {
        0, 0, 0,   0, 0, 0,
        0, 0, 0,   1, 1, 1,
    };
    uint8_t src_round_up[12] = {
        0, 0, 0,   1, 1, 1,
        1, 1, 1,   1, 1, 1,
    };
    uint8_t out[3];

    image_bin_row(src_round_down, 6, 2, 0, 2, 2, 1, out);
    EXPECT_EQ(0, out[0]);

    image_bin_row(src_round_up, 6, 2, 0, 2, 2, 1, out);
    EXPECT_EQ(1, out[0]);
}

TEST_F(ImageBinRowTest, GivenSrcWidthClipsRightEdge_AveragesFewerColumns) {
    // scale=2 but src_width=1, so only the left column of each 2x2 block
    // exists; the right column (dx=1) is clipped by src_width.
    uint8_t src[12] = {
        10, 10, 10,   99, 99, 99,  // row 0: col0 real, col1 out-of-range garbage
        30, 30, 30,   99, 99, 99,  // row 1
    };
    uint8_t out[3];

    image_bin_row(src, /* mcu_row_width */ 6, /* rows_available */ 2,
                 /* src_x_start */ 0, /* src_width */ 1, /* scale */ 2,
                 /* out_width */ 1, out);

    // Only (10,10,10) and (30,30,30) contribute -> average = 20.
    EXPECT_EQ(20, out[0]);
    EXPECT_EQ(20, out[1]);
    EXPECT_EQ(20, out[2]);
}

TEST_F(ImageBinRowTest, GivenRowsAvailableClipsBottomEdge_AveragesFewerRows) {
    // scale=2 but rows_available=1, so only row 0 of the block contributes.
    uint8_t src[12] = {
        10, 10, 10,   30, 30, 30,   // row 0 (only this row is "available")
        99, 99, 99,   99, 99, 99,   // row 1 - must not be read
    };
    uint8_t out[3];

    image_bin_row(src, /* mcu_row_width */ 6, /* rows_available */ 1,
                 /* src_x_start */ 0, /* src_width */ 2, /* scale */ 2,
                 /* out_width */ 1, out);

    // Only (10,10,10) and (30,30,30) contribute -> average = 20.
    EXPECT_EQ(20, out[0]);
    EXPECT_EQ(20, out[1]);
    EXPECT_EQ(20, out[2]);
}

TEST_F(ImageBinRowTest, GivenZeroRowsAvailable_LeavesOutputUnchanged) {
    uint8_t src[6] = { 10, 10, 10,  20, 20, 20 };
    uint8_t out[3] = { 111, 222, 333 % 256 };  // sentinel values
    uint8_t expected[3] = { 111, 222, 333 % 256 };

    image_bin_row(src, /* mcu_row_width */ 6, /* rows_available */ 0,
                 /* src_x_start */ 0, /* src_width */ 2, /* scale */ 2,
                 /* out_width */ 1, out);

    EXPECT_EQ(0, memcmp(expected, out, sizeof(out)));
}

TEST_F(ImageBinRowTest, GivenMultipleOutputPixels_EachUsesOwnBlock) {
    // Two side-by-side 2x2 blocks, scale=2, out_width=2.
    uint8_t src[24] = {
        // row 0: block0 (0,0)&(10,10,10), block1 (100,100,100)&(110,110,110)
        0,0,0,   10,10,10,   100,100,100,   110,110,110,
        // row 1: block0 (20,20,20)&(30,30,30), block1 (120,..)&(130,..)
        20,20,20, 30,30,30,  120,120,120,   130,130,130,
    };
    uint8_t out[6];

    image_bin_row(src, /* mcu_row_width */ 12, /* rows_available */ 2,
                 /* src_x_start */ 0, /* src_width */ 4, /* scale */ 2,
                 /* out_width */ 2, out);

    EXPECT_EQ(15, out[0]);   // avg(0,10,20,30)
    EXPECT_EQ(115, out[3]);  // avg(100,110,120,130)
}

TEST_F(ImageBinRowTest, GivenSrcXStartOffset_CropsFromCorrectColumn) {
    // 4 columns wide, crop starting at column 2.
    uint8_t src[12] = {
        99,99,99,  99,99,99,  10,10,10,  30,30,30,
    };
    uint8_t out[3];

    image_bin_row(src, /* mcu_row_width */ 12, /* rows_available */ 1,
                 /* src_x_start */ 2, /* src_width */ 4, /* scale */ 2,
                 /* out_width */ 1, out);

    // Only row 0 available (rows_available=1), columns 2-3 -> (10+30)/2 = 20.
    EXPECT_EQ(20, out[0]);
}

// ---------------------------------------------------------------------------
// image_dither_row()
// ---------------------------------------------------------------------------

class ImageDitherRowTest : public ::testing::Test {
protected:
    static void ClearError(int16_t *err, int row_width) {
        memset(err, 0, sizeof(int16_t) * row_width * 3);
    }
};

TEST_F(ImageDitherRowTest, GivenExactlyRepresentableColour_NoErrorDiffused) {
    // RGB222 levels are 0/85/170/255. Feed an exact level with zero pending
    // error: quantization should be a no-op and diffuse zero error.
    uint8_t row[3] = { 255, 170, 0 };
    int16_t curr_error[3] = { 0, 0, 0 };
    int16_t next_error[3] = { 0, 0, 0 };

    image_dither_row(row, 1, curr_error, next_error, kImageDitherFsRgb222);

    EXPECT_EQ(255, row[0]);
    EXPECT_EQ(170, row[1]);
    EXPECT_EQ(0, row[2]);
    EXPECT_EQ(0, next_error[0]);
    EXPECT_EQ(0, next_error[1]);
    EXPECT_EQ(0, next_error[2]);
}

TEST_F(ImageDitherRowTest, GivenRgb121_QuantizesToNearestOfTwoLevels) {
    // RGB121: red/blue are single-bit (threshold at 128), green is 2-bit.
    uint8_t row[3] = { 200, 10, 100 };  // r>=128->255, g near 0->0, b<128->0
    int16_t curr_error[3] = { 0, 0, 0 };
    int16_t next_error[3] = { 0, 0, 0 };

    image_dither_row(row, 1, curr_error, next_error, kImageDitherFsRgb121);

    EXPECT_EQ(255, row[0]);  // r1 = 1
    EXPECT_EQ(0, row[1]);    // g2 = round(10*3/255) = 0
    EXPECT_EQ(0, row[2]);    // b1 = 0
}

TEST_F(ImageDitherRowTest, Rgb565Mode_PassesThroughUnchangedWithNoErrorDiffusion) {
    // RGB565 is explicitly unimplemented and documented to pass values
    // through untouched, diffusing no error.
    uint8_t row[3] = { 123, 45, 200 };
    int16_t curr_error[3] = { 0, 0, 0 };
    int16_t next_error[3] = { 0, 0, 0 };

    image_dither_row(row, 1, curr_error, next_error, kImageDitherFsRgb565);

    EXPECT_EQ(123, row[0]);
    EXPECT_EQ(45, row[1]);
    EXPECT_EQ(200, row[2]);
    EXPECT_EQ(0, next_error[0]);
    EXPECT_EQ(0, next_error[1]);
    EXPECT_EQ(0, next_error[2]);
}

TEST_F(ImageDitherRowTest, FloydSteinberg_DistributesErrorWithCorrectWeights) {
    // 3-pixel row: target (r=10, not representable in RGB222) sits in the
    // middle, flanked by exactly-representable (0) pixels. This keeps the
    // right-diffusion, "directly below", and "below-left" slots free of
    // cascading contributions from neighbouring pixels' own quantization
    // error (verified by hand). The "below-right" slot *does* pick up a
    // contribution from the trailing pixel's own residual error and so is
    // deliberately not asserted here - that's inherent to error diffusion,
    // not a bug.
    uint8_t row[9] = { 0, 0, 0,  10, 0, 0,  0, 0, 0 };
    int16_t curr_error[9] = { 0 };
    int16_t next_error[9] = { 0 };

    image_dither_row(row, 3, curr_error, next_error, kImageDitherFsRgb222);

    const int16_t err_r = 10;  // 10 - 0 (nearest RGB222 red level)
    EXPECT_EQ(0, row[3]);      // target pixel quantized to 0

    // 7/16 of error diffused right, within the current row.
    EXPECT_EQ((err_r * 7) / 16, curr_error[6]);
    // 3/16 to below-left, 5/16 to directly below.
    EXPECT_EQ((err_r * 3) / 16, next_error[0]);
    EXPECT_EQ((err_r * 5) / 16, next_error[3]);
}

TEST_F(ImageDitherRowTest, Atkinson_DistributesSixEighthsAndDiscardsRemainder) {
    // Atkinson only diffuses 6/8 of the error (1/8 each to 6 neighbours,
    // fewer if clipped at row edges); the rest is discarded.
    uint8_t row[9] = { 10, 0, 0,  0, 0, 0,  0, 0, 0 };  // 3 pixels
    int16_t curr_error[9] = { 0 };
    int16_t next_error[9] = { 0 };

    image_dither_row(row, 3, curr_error, next_error, kImageDitherAtkinsonRgb222);

    const int16_t err_r = 10;
    const int16_t share = err_r / 8;

    // Two neighbours in the current row (x+1, x+2).
    EXPECT_EQ(share, curr_error[3]);
    EXPECT_EQ(share, curr_error[6]);
    // below-left skipped (x==0); below and below-right present.
    EXPECT_EQ(share, next_error[0]);
    EXPECT_EQ(share, next_error[3]);
}

TEST_F(ImageDitherRowTest, GivenPendingError_IsAppliedBeforeQuantization) {
    // Pixel value 0 with a large positive pending error should push it up
    // into a higher RGB222 level than 0 alone would produce.
    uint8_t row[3] = { 0, 0, 0 };
    int16_t curr_error[3] = { 200, 0, 0 };  // pending +200 on red
    int16_t next_error[3] = { 0, 0, 0 };

    image_dither_row(row, 1, curr_error, next_error, kImageDitherFsRgb222);

    // old_r = 0 + 200 = 200, nearest RGB222 red level to 200 is 170.
    EXPECT_EQ(170, row[0]);
}

// ---------------------------------------------------------------------------
// image_load_jpg() - argument validation
// ---------------------------------------------------------------------------
//
// These cover the validation paths that reject bad calls before any file is
// opened or picojpeg is touched. They use the same surface-fixture pattern
// as GraphicsTest (graphics_test.cxx). A nonexistent filename is used
// throughout so that tests asserting "validation passes" fail for a
// distinct reason (file I/O) rather than being masked by validation.

class ImageLoadJpgValidationTest : public ::testing::Test {

protected:

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }
};

TEST_F(ImageLoadJpgValidationTest, GivenNullSurface_Fails) {
    EXPECT_EQ(kGraphicsInvalidWriteSurface,
             image_load_jpg(NULL, (char *) "foo.jpg", 0, 0, kImageDitherNone, 0, 0, 1));
}

TEST_F(ImageLoadJpgValidationTest, GivenSurfaceTypeNone_Fails) {
    MmSurface surface = {};
    surface.type = kGraphicsNone;

    EXPECT_EQ(kGraphicsInvalidWriteSurface,
             image_load_jpg(&surface, (char *) "foo.jpg", 0, 0, kImageDitherNone, 0, 0, 1));
}

TEST_F(ImageLoadJpgValidationTest, GivenRgb565DitherModes_Fails) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    EXPECT_EQ(kInvalidArgument,
             image_load_jpg(&graphics_surfaces[0], (char *) "foo.jpg", 0, 0,
                            kImageDitherFsRgb565, 0, 0, 1));
    EXPECT_EQ(kInvalidArgument,
             image_load_jpg(&graphics_surfaces[0], (char *) "foo.jpg", 0, 0,
                            kImageDitherAtkinsonRgb565, 0, 0, 1));
}

TEST_F(ImageLoadJpgValidationTest, GivenNonRgb565DitherModes_PassesDitherValidation) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    // All non-RGB565 modes, including "no dithering", should get past the
    // mode check and fail later (file not found) rather than with
    // kInvalidArgument.
    static const ImageDitherMode kModes[] = {
        kImageDitherNone,          kImageDitherFsRgb121,       kImageDitherFsRgb222,
        kImageDitherFsRgb332,      kImageDitherAtkinsonRgb121, kImageDitherAtkinsonRgb222,
        kImageDitherAtkinsonRgb332,
    };
    for (ImageDitherMode mode : kModes) {
        const MmResult result = image_load_jpg(&graphics_surfaces[0], (char *) "does_not_exist.jpg",
                                               0, 0, mode, 0, 0, 1);
        EXPECT_NE(kInvalidArgument, result) << "mode=" << mode;
        EXPECT_NE(kGraphicsInvalidWriteSurface, result) << "mode=" << mode;
    }
}

TEST_F(ImageLoadJpgValidationTest, GivenInvalidScale_Fails) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    static const int kInvalidScales[] = { -1, 0, 3, 5, 6, 7, 9, 16 };
    for (int scale : kInvalidScales) {
        EXPECT_EQ(kInvalidArgument,
                 image_load_jpg(&graphics_surfaces[0], (char *) "foo.jpg", 0, 0, kImageDitherNone,
                                0, 0, scale))
            << "scale=" << scale;
    }
}

TEST_F(ImageLoadJpgValidationTest, GivenValidScale_PassesScaleValidation) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    static const int kValidScales[] = { 1, 2, 4, 8 };
    for (int scale : kValidScales) {
        const MmResult result = image_load_jpg(&graphics_surfaces[0], (char *) "does_not_exist.jpg",
                                               0, 0, kImageDitherNone, 0, 0, scale);
        EXPECT_NE(kInvalidArgument, result) << "scale=" << scale;
        EXPECT_NE(kGraphicsInvalidWriteSurface, result) << "scale=" << scale;
    }
}

TEST_F(ImageLoadJpgValidationTest, GivenFileNotFound_ReturnsErrorNotCrash) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    const MmResult result = image_load_jpg(&graphics_surfaces[0], (char *) "does_not_exist.jpg", 0,
                                           0, kImageDitherNone, 0, 0, 1);

    EXPECT_NE(kOk, result);
    EXPECT_NE(kInvalidArgument, result);
    EXPECT_NE(kGraphicsInvalidWriteSurface, result);
}

// ---------------------------------------------------------------------------
// image_load_jpg() - decoding
// ---------------------------------------------------------------------------

// Generated with TooJpeg (src/third_party/toojpeg.{h,cpp}), quality=90, no
// chroma downsampling, from a 16x16 RGB source image with:
//   R(x,y) = x * 17  (0..255 across x=0..15)
//   G(x,y) = y * 17  (0..255 across y=0..15)
//   B(x,y) = 128     (constant)
// 16x16 with no downsampling decodes as 4 MCUs (2x2 grid of 8x8 blocks,
// scanType YH1V1), so this exercises multi-MCU-row decoding. Verified by
// round-tripping through picojpeg: max abs channel error vs. the formula
// above is 7, which is normal JPEG quantization loss, not a bug in the
// fixture.
static const uint8_t kTinyJpeg16x16[] = {
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x84, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03,
    0x03, 0x03, 0x03, 0x04, 0x03, 0x03, 0x04, 0x05, 0x08, 0x05, 0x05, 0x04, 0x04, 0x05, 0x0A, 0x07,
    0x07, 0x06, 0x08, 0x0C, 0x0A, 0x0C, 0x0C, 0x0B, 0x0A, 0x0B, 0x0B, 0x0D, 0x0E, 0x12, 0x10, 0x0D,
    0x0E, 0x11, 0x0E, 0x0B, 0x0B, 0x10, 0x16, 0x10, 0x11, 0x13, 0x14, 0x15, 0x15, 0x15, 0x0C, 0x0F,
    0x17, 0x18, 0x16, 0x14, 0x18, 0x12, 0x14, 0x15, 0x14, 0x01, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05,
    0x09, 0x05, 0x05, 0x09, 0x14, 0x0D, 0x0B, 0x0D, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00,
    0x10, 0x00, 0x10, 0x03, 0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xC4, 0x01,
    0xA2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x00,
    0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D, 0x01,
    0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22,
    0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24,
    0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A,
    0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
    0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,
    0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
    0xFA, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x11, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00,
    0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13,
    0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15,
    0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
    0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4,
    0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
    0xFA, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0xF9,
    0x77, 0xC2, 0x3F, 0x09, 0x3E, 0xE7, 0xEE, 0x7F, 0x4A, 0xFD, 0x6F, 0x1B, 0x9D, 0x6F, 0xA9, 0xC1,
    0xC3, 0x1C, 0x41, 0xF0, 0xFB, 0xC7, 0xB6, 0xF8, 0x47, 0xE1, 0x27, 0xDC, 0xFD, 0xCF, 0xE9, 0x5F,
    0x05, 0x8D, 0xCE, 0xB7, 0xD4, 0xFE, 0xB5, 0xE1, 0x8E, 0x20, 0xF8, 0x7D, 0xE3, 0xDA, 0x3C, 0x23,
    0xF0, 0x93, 0xEE, 0x7E, 0xE7, 0xF4, 0xAF, 0x84, 0xC6, 0xE7, 0x5B, 0xEA, 0x7F, 0x96, 0x5C, 0x31,
    0xC4, 0x1F, 0x0F, 0xBC, 0x7B, 0x67, 0x84, 0x7E, 0x12, 0x7D, 0xCF, 0xDC, 0xFE, 0x95, 0xF0, 0x58,
    0xDC, 0xEB, 0x7D, 0x4F, 0xEB, 0x6E, 0x18, 0xE2, 0x0F, 0x87, 0xDE, 0x3F, 0xFF, 0xD9,
};
static const size_t kTinyJpeg16x16Len = sizeof(kTinyJpeg16x16);

// Expected source pixel formula the fixture above was generated from; used
// to compute expected (possibly cropped/binned) values in the tests below.
static void expected_gradient_rgb(int x, int y, int *r, int *g, int *b) {
    *r = x * 17;
    *g = y * 17;
    *b = 128;
}

static void get_surface_rgb(MmSurface *surface, int x, int y, int *r, int *g, int *b) {
    MmGraphicsColour colour = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(surface, x, y, &colour));
    *r = (colour >> 16) & 0xFF;
    *g = (colour >> 8) & 0xFF;
    *b = colour & 0xFF;
}

class ImageLoadJpgDecodeTest : public ::testing::Test {

protected:

    char filename[STRINGSIZE];

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);

        snprintf_nowarn(filename, sizeof(filename), "/tmp/mmb4l_test_tiny_gradient_%d.jpg", system_getpid());
        FILE *f = fopen(filename, "wb");
        ASSERT_NE(nullptr, f);
        ASSERT_EQ(kTinyJpeg16x16Len, fwrite(kTinyJpeg16x16, 1, kTinyJpeg16x16Len, f));
        fclose(f);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
        remove(filename);
    }
};

TEST_F(ImageLoadJpgDecodeTest, GivenNoDitherNoScale_DecodesGradientPixels) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 16, 16));

    EXPECT_EQ(kOk, image_load_jpg(&graphics_surfaces[0], filename, 0, 0, kImageDitherNone, 0, 0, 1));

    // Allow generous tolerance for JPEG quantization loss (verified up to
    // 7 by round-tripping the fixture through picojpeg separately).
    const int kTolerance = 20;
    static const int kCoords[][2] = { {0, 0}, {15, 0}, {0, 15}, {15, 15}, {7, 7} };
    for (const auto &coord : kCoords) {
        int x = coord[0], y = coord[1];
        int er, eg, eb;
        expected_gradient_rgb(x, y, &er, &eg, &eb);
        int ar, ag, ab;
        get_surface_rgb(&graphics_surfaces[0], x, y, &ar, &ag, &ab);
        EXPECT_NEAR(er, ar, kTolerance) << "at (" << x << "," << y << ") red";
        EXPECT_NEAR(eg, ag, kTolerance) << "at (" << x << "," << y << ") green";
        EXPECT_NEAR(eb, ab, kTolerance) << "at (" << x << "," << y << ") blue";
    }
}

TEST_F(ImageLoadJpgDecodeTest, GivenScale2_BinsPixelsToApproximateBlockAverage) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    EXPECT_EQ(kOk, image_load_jpg(&graphics_surfaces[0], filename, 0, 0, kImageDitherNone, 0, 0, 2));

    // Output pixel (ox,oy) should approximate the average of source block
    // [2*ox, 2*ox+1] x [2*oy, 2*oy+1].
    const int kTolerance = 20;
    static const int kOutCoords[][2] = { {0, 0}, {7, 0}, {0, 7}, {7, 7} };
    for (const auto &coord : kOutCoords) {
        int ox = coord[0], oy = coord[1];
        int r00, g00, b00, r10, g10, b10, r01, g01, b01, r11, g11, b11;
        expected_gradient_rgb(2 * ox, 2 * oy, &r00, &g00, &b00);
        expected_gradient_rgb(2 * ox + 1, 2 * oy, &r10, &g10, &b10);
        expected_gradient_rgb(2 * ox, 2 * oy + 1, &r01, &g01, &b01);
        expected_gradient_rgb(2 * ox + 1, 2 * oy + 1, &r11, &g11, &b11);
        int er = (r00 + r10 + r01 + r11) / 4;
        int eg = (g00 + g10 + g01 + g11) / 4;
        int eb = (b00 + b10 + b01 + b11) / 4;

        int ar, ag, ab;
        get_surface_rgb(&graphics_surfaces[0], ox, oy, &ar, &ag, &ab);
        EXPECT_NEAR(er, ar, kTolerance) << "at (" << ox << "," << oy << ") red";
        EXPECT_NEAR(eg, ag, kTolerance) << "at (" << ox << "," << oy << ") green";
        EXPECT_NEAR(eb, ab, kTolerance) << "at (" << ox << "," << oy << ") blue";
    }
}

TEST_F(ImageLoadJpgDecodeTest, GivenDitherRgb121_QuantizesToDiscreteLevels) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 16, 16));

    EXPECT_EQ(kOk,
             image_load_jpg(&graphics_surfaces[0], filename, 0, 0, kImageDitherFsRgb121, 0, 0, 1));

    // RGB121: red/blue are single-bit (0 or 255), green is one of 4 levels
    // (0, 85, 170, 255). Every decoded pixel must land exactly on one of
    // these levels, regardless of the source gradient value.
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int r, g, b;
            get_surface_rgb(&graphics_surfaces[0], x, y, &r, &g, &b);
            EXPECT_TRUE(r == 0 || r == 255) << "at (" << x << "," << y << ") red=" << r;
            EXPECT_TRUE(g == 0 || g == 85 || g == 170 || g == 255)
                << "at (" << x << "," << y << ") green=" << g;
            EXPECT_TRUE(b == 0 || b == 255) << "at (" << x << "," << y << ") blue=" << b;
        }
    }
}

TEST_F(ImageLoadJpgDecodeTest, GivenCropOffset_StartsFromCorrectSourcePixel) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    // Crop starting at source (8,8); only the bottom-right 8x8 quadrant of
    // the 16x16 source should be drawn, anchored at surface (0,0).
    EXPECT_EQ(kOk,
             image_load_jpg(&graphics_surfaces[0], filename, 0, 0, kImageDitherNone, 8, 8, 1));

    const int kTolerance = 20;
    static const int kCoords[][2] = { {0, 0}, {7, 0}, {0, 7}, {7, 7} };
    for (const auto &coord : kCoords) {
        int x = coord[0], y = coord[1];
        int er, eg, eb;
        expected_gradient_rgb(8 + x, 8 + y, &er, &eg, &eb);
        int ar, ag, ab;
        get_surface_rgb(&graphics_surfaces[0], x, y, &ar, &ag, &ab);
        EXPECT_NEAR(er, ar, kTolerance) << "at (" << x << "," << y << ") red";
        EXPECT_NEAR(eg, ag, kTolerance) << "at (" << x << "," << y << ") green";
        EXPECT_NEAR(eb, ab, kTolerance) << "at (" << x << "," << y << ") blue";
    }
}

TEST_F(ImageLoadJpgDecodeTest, GivenXyPlacement_DrawsAtOffsetLeavingRestUntouched) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 32, 32));

    // Fill the surface with a sentinel colour so we can tell drawn pixels
    // apart from untouched ones.
    const MmGraphicsColour kSentinel = RGB(1, 2, 3, 0xFF);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            graphics_surfaces[0].pixels[y * 32 + x] = kSentinel;
        }
    }

    EXPECT_EQ(kOk,
             image_load_jpg(&graphics_surfaces[0], filename, 4, 4, kImageDitherNone, 0, 0, 1));

    // (4,4) on the surface should correspond to source (0,0) - top-left of
    // the gradient, i.e. dark red/green.
    const int kTolerance = 20;
    int ar, ag, ab;
    get_surface_rgb(&graphics_surfaces[0], 4, 4, &ar, &ag, &ab);
    EXPECT_NEAR(0, ar, kTolerance);
    EXPECT_NEAR(0, ag, kTolerance);
    EXPECT_NEAR(128, ab, kTolerance);

    // (19,19) = (4+15, 4+15) should correspond to source (15,15) - bright
    // red/green corner.
    get_surface_rgb(&graphics_surfaces[0], 19, 19, &ar, &ag, &ab);
    EXPECT_NEAR(255, ar, kTolerance);
    EXPECT_NEAR(255, ag, kTolerance);
    EXPECT_NEAR(128, ab, kTolerance);

    // A pixel outside the drawn 16x16 region should still be the sentinel.
    MmGraphicsColour outside = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(&graphics_surfaces[0], 0, 0, &outside));
    EXPECT_EQ(kSentinel, outside);
}

// ---------------------------------------------------------------------------
// image_save_jpg() - argument validation
// ---------------------------------------------------------------------------
//
// Mirrors ImageLoadJpgValidationTest: same surface-fixture pattern, checking
// the validation paths that reject bad calls before any file I/O or TooJpeg
// encoding happens. A path under /tmp is used throughout (rather than a
// nonexistent one) so that tests asserting "validation passes" fail for a
// distinct file-content reason rather than being masked by a file-open error.

class ImageSaveJpgValidationTest : public ::testing::Test {

protected:

    char filename[STRINGSIZE];

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
        snprintf_nowarn(filename, sizeof(filename), "/tmp/mmb4l_test_save_jpg_validation_%d.jpg",
                system_getpid());
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
        remove(filename);
    }
};

TEST_F(ImageSaveJpgValidationTest, GivenNullSurface_Fails) {
    EXPECT_EQ(kGraphicsInvalidReadSurface,
             image_save_jpg(NULL, filename, 0, 0, 8, 8, 90));
}

TEST_F(ImageSaveJpgValidationTest, GivenSurfaceTypeNone_Fails) {
    MmSurface surface = {};
    surface.type = kGraphicsNone;

    EXPECT_EQ(kGraphicsInvalidReadSurface,
             image_save_jpg(&surface, filename, 0, 0, 8, 8, 90));
}

TEST_F(ImageSaveJpgValidationTest, GivenNonPositiveWidthOrHeight_Fails) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    EXPECT_EQ(kImageTooLarge,
             image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 0, 8, 90));
    EXPECT_EQ(kImageTooLarge,
             image_save_jpg(&graphics_surfaces[0], filename, 0, 0, -1, 8, 90));
    EXPECT_EQ(kImageTooLarge,
             image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 8, 0, 90));
    EXPECT_EQ(kImageTooLarge,
             image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 8, -1, 90));
}

TEST_F(ImageSaveJpgValidationTest, GivenWidthOrHeightTooLarge_Fails) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    EXPECT_EQ(kImageTooLarge,
             image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 65536, 8, 90));
    EXPECT_EQ(kImageTooLarge,
             image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 8, 65536, 90));
}

TEST_F(ImageSaveJpgValidationTest, GivenValidArguments_Succeeds) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 8, 8, 90));

    FileInfo info;
    ASSERT_EQ(kOk, file_info(filename, &info));
    EXPECT_TRUE(info.exists);
}

// ---------------------------------------------------------------------------
// image_save_jpg() - filename extension handling
// ---------------------------------------------------------------------------

class ImageSaveJpgFilenameTest : public ::testing::Test {

protected:

    char base[STRINGSIZE];

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
        ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 4));
        snprintf_nowarn(base, sizeof(base), "/tmp/mmb4l_test_save_jpg_filename_%d",
                        system_getpid());
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }
};

TEST_F(ImageSaveJpgFilenameTest, GivenNoExtension_AppendsDotJpg) {
    char filename[STRINGSIZE];
    snprintf_nowarn(filename, sizeof(filename), "%s", base);
    char expected[STRINGSIZE];
    snprintf_nowarn(expected, sizeof(expected), "%s.jpg", base);
    remove(expected);

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 4, 4, 90));

    FileInfo info;
    ASSERT_EQ(kOk, file_info(expected, &info));
    EXPECT_TRUE(info.exists);

    remove(expected);
}

TEST_F(ImageSaveJpgFilenameTest, GivenDotJpgExtension_DoesNotAppendAgain) {
    char filename[STRINGSIZE];
    snprintf_nowarn(filename, sizeof(filename), "%s.jpg", base);
    remove(filename);

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 4, 4, 90));

    FileInfo info;
    ASSERT_EQ(kOk, file_info(filename, &info));
    EXPECT_TRUE(info.exists);

    remove(filename);
}

TEST_F(ImageSaveJpgFilenameTest, GivenDotJpegExtension_DoesNotAppendDotJpg) {
    char filename[STRINGSIZE];
    snprintf_nowarn(filename, sizeof(filename), "%s.jpeg", base);
    remove(filename);

    // Clean up any stale file from a previous run before asserting on its
    // absence below.
    char wrong[STRINGSIZE];
    snprintf_nowarn(wrong, sizeof(wrong), "%s.jpeg.jpg", base);
    remove(wrong);

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 4, 4, 90));

    FileInfo info;
    ASSERT_EQ(kOk, file_info(filename, &info));
    EXPECT_TRUE(info.exists);

    // Must not also have created "<base>.jpeg.jpg". file_info() returns kOk
    // even when the path doesn't exist (see its .exists field), so the
    // absence check has to be on info.exists, not on the MmResult.
    FileInfo wrong_info = {};
    MmResult wrong_result = file_info(wrong, &wrong_info);
    EXPECT_FALSE(SUCCEEDED(wrong_result) && wrong_info.exists);

    remove(filename);
    remove(wrong);
}

// ---------------------------------------------------------------------------
// image_save_jpg() - encoding
// ---------------------------------------------------------------------------

class ImageSaveJpgEncodeTest : public ::testing::Test {

protected:

    char filename[STRINGSIZE];

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
        snprintf_nowarn(filename, sizeof(filename), "/tmp/mmb4l_test_save_jpg_encode_%d.jpg",
                system_getpid());
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
        remove(filename);
    }

    /** Reads the whole file back into a heap buffer. */
    static std::vector<uint8_t> read_file(const char *path) {
        std::vector<uint8_t> data;
        FILE *f = fopen(path, "rb");
        if (!f) return data;
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (size > 0) {
            data.resize((size_t) size);
            [[maybe_unused]] size_t n = fread(data.data(), 1, (size_t) size, f);
        }
        fclose(f);
        return data;
    }
};

TEST_F(ImageSaveJpgEncodeTest, WritesValidJpegStreamMarkers) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 16, 16));

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 16, 16, 90));

    std::vector<uint8_t> data = read_file(filename);
    ASSERT_GE(data.size(), 4u);

    // SOI marker at the start.
    EXPECT_EQ(0xFF, data[0]);
    EXPECT_EQ(0xD8, data[1]);

    // EOI marker at the end.
    EXPECT_EQ(0xFF, data[data.size() - 2]);
    EXPECT_EQ(0xD9, data[data.size() - 1]);
}

// picojpeg callback that reads sequentially from an in-memory buffer.
struct MemJpegReader {
    const uint8_t *data;
    size_t size;
    size_t offset;
};

static unsigned char mem_jpeg_need_bytes_cb(unsigned char *buf, unsigned char buf_size,
                                            unsigned char *bytes_read, void *userdata) {
    MemJpegReader *reader = (MemJpegReader *) userdata;
    size_t remaining = reader->size - reader->offset;
    size_t n = remaining < buf_size ? remaining : buf_size;
    memcpy(buf, reader->data + reader->offset, n);
    reader->offset += n;
    *bytes_read = (unsigned char) n;
    return 0;
}

TEST_F(ImageSaveJpgEncodeTest, RoundTripsSolidColourViaPicojpeg) {
    // Fill an 8x8 surface with a solid, JPEG-friendly colour (avoids
    // quantization ambiguity for this smoke test).
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));
    const MmGraphicsColour fill = RGB(200, 60, 30, 0xFF);
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            graphics_surfaces[0].pixels[y * 8 + x] = fill;
        }
    }

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 0, 0, 8, 8, 90));

    std::vector<uint8_t> data = read_file(filename);
    ASSERT_FALSE(data.empty());

    ASSERT_EQ(0, picojpeg_alloc(malloc, free));

    MemJpegReader reader = { data.data(), data.size(), 0 };
    pjpeg_image_info_t info;
    ASSERT_EQ(0, pjpeg_decode_init(&info, mem_jpeg_need_bytes_cb, &reader, 0));
    EXPECT_EQ(8, info.m_width);
    EXPECT_EQ(8, info.m_height);

    // Decode every MCU; for a uniform 8x8 image (YH1V1) there's exactly one.
    uint8_t status;
    do {
        status = pjpeg_decode_mcu();
        ASSERT_TRUE(status == 0 || status == PJPG_NO_MORE_BLOCKS);
    } while (status == 0);

    // Spot-check the first decoded pixel is close to the source colour.
    const int kTolerance = 15;
    EXPECT_NEAR(200, info.m_pMCUBufR[0], kTolerance);
    EXPECT_NEAR(60, info.m_pMCUBufG[0], kTolerance);
    EXPECT_NEAR(30, info.m_pMCUBufB[0], kTolerance);

    picojpeg_free(free);
}

TEST_F(ImageSaveJpgEncodeTest, GivenNonZeroOrigin_SavesCorrectRegion) {
    // 16x16 surface; fill left half with one colour, right half with
    // another, then save just the right half via (x, width) offset.
    ASSERT_EQ(kOk, graphics_buffer_create(0, 16, 16));
    const MmGraphicsColour left = RGB(0, 0, 0, 0xFF);
    const MmGraphicsColour right = RGB(255, 255, 255, 0xFF);
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            graphics_surfaces[0].pixels[y * 16 + x] = (x < 8) ? left : right;
        }
    }

    EXPECT_EQ(kOk, image_save_jpg(&graphics_surfaces[0], filename, 8, 0, 8, 8, 90));

    std::vector<uint8_t> data = read_file(filename);
    ASSERT_FALSE(data.empty());

    ASSERT_EQ(0, picojpeg_alloc(malloc, free));
    MemJpegReader reader = { data.data(), data.size(), 0 };
    pjpeg_image_info_t info;
    ASSERT_EQ(0, pjpeg_decode_init(&info, mem_jpeg_need_bytes_cb, &reader, 0));
    EXPECT_EQ(8, info.m_width);
    EXPECT_EQ(8, info.m_height);

    uint8_t status = pjpeg_decode_mcu();
    ASSERT_TRUE(status == 0 || status == PJPG_NO_MORE_BLOCKS);

    // Whole saved region came from the "right" (white) half.
    const int kTolerance = 15;
    EXPECT_NEAR(255, info.m_pMCUBufR[0], kTolerance);
    EXPECT_NEAR(255, info.m_pMCUBufG[0], kTolerance);
    EXPECT_NEAR(255, info.m_pMCUBufB[0], kTolerance);

    picojpeg_free(free);
}

// ---------------------------------------------------------------------------
// image_jpg_get_row_cb()
// ---------------------------------------------------------------------------
//
// Pure(ish) unit tests against a surface fixture, bypassing TooJpeg/file I/O
// entirely - same "extract the testable core" approach as ImageBinRowTest.

class ImageJpgGetRowCbTest : public ::testing::Test {

protected:

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }
};

TEST_F(ImageJpgGetRowCbTest, GivenZeroOrigin_ReadsRowDirectlyFromSurface) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 2));

    // Row 0: four distinct colours; row 1: sentinel that must not be read.
    graphics_surfaces[0].pixels[0] = RGB(10, 20, 30, 0xFF);
    graphics_surfaces[0].pixels[1] = RGB(40, 50, 60, 0xFF);
    graphics_surfaces[0].pixels[2] = RGB(70, 80, 90, 0xFF);
    graphics_surfaces[0].pixels[3] = RGB(100, 110, 120, 0xFF);
    graphics_surfaces[0].pixels[4] = RGB(255, 255, 255, 0xFF);  // row 1, col 0
    graphics_surfaces[0].pixels[5] = RGB(255, 255, 255, 0xFF);

    ImageJpgRowSource source = { &graphics_surfaces[0], /* x */ 0, /* y */ 0, /* width */ 4 };
    uint8_t out[4 * 3];

    image_jpg_get_row_cb(0, out, &source);

    EXPECT_EQ(10, out[0]); EXPECT_EQ(20, out[1]); EXPECT_EQ(30, out[2]);
    EXPECT_EQ(40, out[3]); EXPECT_EQ(50, out[4]); EXPECT_EQ(60, out[5]);
    EXPECT_EQ(70, out[6]); EXPECT_EQ(80, out[7]); EXPECT_EQ(90, out[8]);
    EXPECT_EQ(100, out[9]); EXPECT_EQ(110, out[10]); EXPECT_EQ(120, out[11]);
}

TEST_F(ImageJpgGetRowCbTest, GivenNonZeroRowIndex_OffsetsFromSourceY) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 3));

    graphics_surfaces[0].pixels[0] = RGB(1, 1, 1, 0xFF);    // (0,0)
    graphics_surfaces[0].pixels[1] = RGB(1, 1, 1, 0xFF);    // (1,0)
    graphics_surfaces[0].pixels[2] = RGB(9, 8, 7, 0xFF);    // (0,1) - source y=1
    graphics_surfaces[0].pixels[3] = RGB(6, 5, 4, 0xFF);    // (1,1)
    graphics_surfaces[0].pixels[4] = RGB(2, 2, 2, 0xFF);    // (0,2)
    graphics_surfaces[0].pixels[5] = RGB(2, 2, 2, 0xFF);    // (1,2)

    // source.y = 1, so row index 0 passed to the callback means surface row 1.
    ImageJpgRowSource source = { &graphics_surfaces[0], /* x */ 0, /* y */ 1, /* width */ 2 };
    uint8_t out[2 * 3];

    image_jpg_get_row_cb(0, out, &source);

    EXPECT_EQ(9, out[0]); EXPECT_EQ(8, out[1]); EXPECT_EQ(7, out[2]);
    EXPECT_EQ(6, out[3]); EXPECT_EQ(5, out[4]); EXPECT_EQ(4, out[5]);

    // row index 1 => surface row 2.
    image_jpg_get_row_cb(1, out, &source);

    EXPECT_EQ(2, out[0]); EXPECT_EQ(2, out[1]); EXPECT_EQ(2, out[2]);
    EXPECT_EQ(2, out[3]); EXPECT_EQ(2, out[4]); EXPECT_EQ(2, out[5]);
}

TEST_F(ImageJpgGetRowCbTest, GivenNonZeroXOrigin_OffsetsColumns) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 1));

    graphics_surfaces[0].pixels[0] = RGB(0, 0, 0, 0xFF);
    graphics_surfaces[0].pixels[1] = RGB(0, 0, 0, 0xFF);
    graphics_surfaces[0].pixels[2] = RGB(11, 22, 33, 0xFF);
    graphics_surfaces[0].pixels[3] = RGB(44, 55, 66, 0xFF);

    // Crop starting at surface column 2, width 2.
    ImageJpgRowSource source = { &graphics_surfaces[0], /* x */ 2, /* y */ 0, /* width */ 2 };
    uint8_t out[2 * 3];

    image_jpg_get_row_cb(0, out, &source);

    EXPECT_EQ(11, out[0]); EXPECT_EQ(22, out[1]); EXPECT_EQ(33, out[2]);
    EXPECT_EQ(44, out[3]); EXPECT_EQ(55, out[4]); EXPECT_EQ(66, out[5]);
}

TEST_F(ImageJpgGetRowCbTest, GivenColumnBeyondSurfaceWidth_ReadsBlack) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 1));

    graphics_surfaces[0].pixels[0] = RGB(255, 255, 255, 0xFF);
    graphics_surfaces[0].pixels[1] = RGB(255, 255, 255, 0xFF);

    // width=4 but the surface is only 2 pixels wide: columns 2 and 3 are
    // out of bounds and must come back as RGB_BLACK, not garbage or a crash.
    ImageJpgRowSource source = { &graphics_surfaces[0], /* x */ 0, /* y */ 0, /* width */ 4 };
    uint8_t out[4 * 3];

    image_jpg_get_row_cb(0, out, &source);

    EXPECT_EQ(255, out[0]); EXPECT_EQ(255, out[1]); EXPECT_EQ(255, out[2]);
    EXPECT_EQ(255, out[3]); EXPECT_EQ(255, out[4]); EXPECT_EQ(255, out[5]);
    EXPECT_EQ(0, out[6]); EXPECT_EQ(0, out[7]); EXPECT_EQ(0, out[8]);
    EXPECT_EQ(0, out[9]); EXPECT_EQ(0, out[10]); EXPECT_EQ(0, out[11]);
}

TEST_F(ImageJpgGetRowCbTest, GivenRowBeyondSurfaceHeight_ReadsBlack) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 1));

    graphics_surfaces[0].pixels[0] = RGB(255, 255, 255, 0xFF);
    graphics_surfaces[0].pixels[1] = RGB(255, 255, 255, 0xFF);

    // y=0, row index 5 => surface row 5, which is out of bounds for a
    // 1-pixel-tall surface.
    ImageJpgRowSource source = { &graphics_surfaces[0], /* x */ 0, /* y */ 0, /* width */ 2 };
    uint8_t out[2 * 3];

    image_jpg_get_row_cb(5, out, &source);

    EXPECT_EQ(0, out[0]); EXPECT_EQ(0, out[1]); EXPECT_EQ(0, out[2]);
    EXPECT_EQ(0, out[3]); EXPECT_EQ(0, out[4]); EXPECT_EQ(0, out[5]);
}

// ---------------------------------------------------------------------------
// image_draw_buffer()
// ---------------------------------------------------------------------------

class ImageDrawBufferTest : public ::testing::Test {

protected:

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }
};

TEST_F(ImageDrawBufferTest, GivenSkip2_ReadsRgbTripletsWithOpaqueAlpha) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 1));

    // Two RGB triplets, no alpha byte.
    const unsigned char buffer[6] = {
        10, 20, 30,   // pixel (0,0)
        40, 50, 60,   // pixel (1,0)
    };

    image_draw_buffer(&graphics_surfaces[0], 0, 0, 1, 0, buffer, /* skip */ 2);

    int r, g, b;
    get_surface_rgb(&graphics_surfaces[0], 0, 0, &r, &g, &b);
    EXPECT_EQ(10, r); EXPECT_EQ(20, g); EXPECT_EQ(30, b);
    get_surface_rgb(&graphics_surfaces[0], 1, 0, &r, &g, &b);
    EXPECT_EQ(40, r); EXPECT_EQ(50, g); EXPECT_EQ(60, b);

    // Alpha defaults to opaque (0xFF) when skip & 1 is clear.
    MmGraphicsColour colour = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(&graphics_surfaces[0], 0, 0, &colour));
    EXPECT_EQ(0xFF, (colour >> 24) & 0xFF);
}

TEST_F(ImageDrawBufferTest, GivenSkip3_ReadsAlphaFromFourthSourceByte) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 1, 1));

    const unsigned char buffer[4] = { 10, 20, 30, 0x80 };  // R,G,B,A

    image_draw_buffer(&graphics_surfaces[0], 0, 0, 0, 0, buffer, /* skip */ 3);

    int r, g, b;
    get_surface_rgb(&graphics_surfaces[0], 0, 0, &r, &g, &b);
    EXPECT_EQ(10, r); EXPECT_EQ(20, g); EXPECT_EQ(30, b);

    MmGraphicsColour colour = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(&graphics_surfaces[0], 0, 0, &colour));
    EXPECT_EQ(0x80, (colour >> 24) & 0xFF);
}

TEST_F(ImageDrawBufferTest, GivenSkip0_ReversesChannelOrderWithZeroAlpha) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 1, 1));

    const unsigned char buffer[3] = { 10, 20, 30 };

    image_draw_buffer(&graphics_surfaces[0], 0, 0, 0, 0, buffer, /* skip */ 0);

    // With skip & 2 clear, channels come out reversed: red=src[2], blue=src[0].
    int r, g, b;
    get_surface_rgb(&graphics_surfaces[0], 0, 0, &r, &g, &b);
    EXPECT_EQ(30, r); EXPECT_EQ(20, g); EXPECT_EQ(10, b);

    MmGraphicsColour colour = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(&graphics_surfaces[0], 0, 0, &colour));
    EXPECT_EQ(0, (colour >> 24) & 0xFF);
}

TEST_F(ImageDrawBufferTest, GivenSkip1_SkipsFourthByteButAlphaStaysZero) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 1));

    // Pixel 0: R,G,B,<skipped>; pixel 1: R,G,B - confirms the 4th byte of
    // pixel 0 is consumed (not stored) and doesn't throw off pixel 1's read.
    const unsigned char buffer[7] = {
        10, 20, 30, 0xFF,  // pixel (0,0) + 1 skipped byte
        40, 50, 60,        // pixel (1,0)
    };

    image_draw_buffer(&graphics_surfaces[0], 0, 0, 1, 0, buffer, /* skip */ 1);

    int r, g, b;
    get_surface_rgb(&graphics_surfaces[0], 0, 0, &r, &g, &b);
    EXPECT_EQ(30, r); EXPECT_EQ(20, g); EXPECT_EQ(10, b);  // reversed, as skip&2 clear

    get_surface_rgb(&graphics_surfaces[0], 1, 0, &r, &g, &b);
    EXPECT_EQ(60, r); EXPECT_EQ(50, g); EXPECT_EQ(40, b);

    MmGraphicsColour colour = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(&graphics_surfaces[0], 0, 0, &colour));
    EXPECT_EQ(0, (colour >> 24) & 0xFF);
}

TEST_F(ImageDrawBufferTest, GivenReversedCoordinates_SwapsAndDrawsSameRegion) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 2));

    // 2x2 buffer, raster order regardless of how corners are specified.
    const unsigned char buffer[12] = {
        1, 1, 1,     2, 2, 2,    // row 0: (0,0) (1,0)
        3, 3, 3,     4, 4, 4,    // row 1: (0,1) (1,1)
    };

    // Pass corners in "reversed" order: (x2,y2) before (x1,y1) in effect.
    image_draw_buffer(&graphics_surfaces[0], /* x1 */ 1, /* y1 */ 1,
                      /* x2 */ 0, /* y2 */ 0, buffer, /* skip */ 2);

    int r, g, b;
    get_surface_rgb(&graphics_surfaces[0], 0, 0, &r, &g, &b);
    EXPECT_EQ(1, r);
    get_surface_rgb(&graphics_surfaces[0], 1, 0, &r, &g, &b);
    EXPECT_EQ(2, r);
    get_surface_rgb(&graphics_surfaces[0], 0, 1, &r, &g, &b);
    EXPECT_EQ(3, r);
    get_surface_rgb(&graphics_surfaces[0], 1, 1, &r, &g, &b);
    EXPECT_EQ(4, r);
}

TEST_F(ImageDrawBufferTest, GivenPartiallyOutOfBoundsRegion_ClipsButAdvancesSourceBuffer) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 1, 1));

    // Destination rectangle spans x=-1..0, i.e. column -1 is off-surface and
    // column 0 is on it. The off-surface pixel must still be skipped over in
    // the source buffer so column 0 picks up the *second* triplet, not the
    // first.
    const unsigned char buffer[6] = {
        99, 99, 99,   // would-be pixel (-1, 0): off surface, must not be drawn
        10, 20, 30,   // pixel (0, 0): on surface
    };

    image_draw_buffer(&graphics_surfaces[0], -1, 0, 0, 0, buffer, /* skip */ 2);

    int r, g, b;
    get_surface_rgb(&graphics_surfaces[0], 0, 0, &r, &g, &b);
    EXPECT_EQ(10, r); EXPECT_EQ(20, g); EXPECT_EQ(30, b);
}

TEST_F(ImageDrawBufferTest, GivenRowFullyOutOfBounds_DoesNotCrash) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 2, 1));

    const unsigned char buffer[6] = {
        1, 1, 1,
        2, 2, 2,
    };

    // y=5 is entirely below the 1-pixel-tall surface.
    image_draw_buffer(&graphics_surfaces[0], 0, 5, 1, 5, buffer, /* skip */ 2);

    // Surface content should be untouched (still whatever graphics_buffer_create
    // initialised it to); the main assertion is that this didn't crash/corrupt
    // memory, which ASan/valgrind in CI would otherwise catch.
    SUCCEED();
}

// ---------------------------------------------------------------------------
// image_load_png() - argument validation
// ---------------------------------------------------------------------------

class ImageLoadPngValidationTest : public ::testing::Test {

protected:

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }
};

TEST_F(ImageLoadPngValidationTest, GivenNullSurface_Fails) {
    EXPECT_EQ(kGraphicsInvalidWriteSurface,
             image_load_png(NULL, (char *) "foo.png", 0, 0, 0, 0));
}

TEST_F(ImageLoadPngValidationTest, GivenSurfaceTypeNone_Fails) {
    MmSurface surface = {};
    surface.type = kGraphicsNone;

    EXPECT_EQ(kGraphicsInvalidWriteSurface,
             image_load_png(&surface, (char *) "foo.png", 0, 0, 0, 0));
}

TEST_F(ImageLoadPngValidationTest, GivenFileNotFound_ReturnsErrorNotCrash) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 8, 8));

    const MmResult result =
        image_load_png(&graphics_surfaces[0], (char *) "does_not_exist.png", 0, 0, 0, 0);

    EXPECT_NE(kOk, result);
    EXPECT_NE(kGraphicsInvalidWriteSurface, result);
}

// ---------------------------------------------------------------------------
// image_load_png() - decoding
// ---------------------------------------------------------------------------
//
// Real (not fabricated) 4x4 PNGs generated with Python's zlib/struct:
// truecolour (UPNG_RGB8), truecolour-with-alpha (UPNG_RGBA8), and 8-bit
// grayscale (UPNG_LUMINANCE8, used to exercise the unsupported-format
// rejection path) - confirmed against upng.h's upng_format enum:
//   UPNG_BADFORMAT=0, UPNG_RGB8=1, UPNG_RGB16=2, UPNG_RGBA8=3,
//   UPNG_RGBA16=4, UPNG_LUMINANCE1=5, ..., UPNG_LUMINANCE8=8, ...
// image_load_png() only accepts format==1 (UPNG_RGB8) or format==3
// (UPNG_RGBA8), so these fixtures exactly straddle that boundary.
//
// RGB fixture:   R(x,y) = min(255, x*80), G(x,y) = min(255, y*80), B = 50
// RGBA fixture:  same R/G/B, A(x,y) = min(255, (x+y)*30)
// Gray fixture:  L(x,y) = min(255, (x+y)*30)  (unsupported format)

static const uint8_t kTinyPngRgb4x4[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x08, 0x02, 0x00, 0x00, 0x00, 0x26, 0x93, 0x09,
    0x29, 0x00, 0x00, 0x00, 0x29, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0x0D, 0xC7, 0x31, 0x01, 0x00,
    0x00, 0x0C, 0xC2, 0x30, 0x34, 0x55, 0x09, 0x12, 0x2B, 0x11, 0x09, 0x5B, 0xBE, 0x24, 0xA1, 0xC1,
    0xB0, 0x90, 0x94, 0x16, 0xCB, 0xFA, 0x91, 0x8A, 0x32, 0x3F, 0xA3, 0xC3, 0xB1, 0x71, 0x77, 0xE3,
    0x12, 0x21, 0xC6, 0xEE, 0x6D, 0x73, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42,
    0x60, 0x82,
};
static const size_t kTinyPngRgb4x4Len = sizeof(kTinyPngRgb4x4);

static const uint8_t kTinyPngRgba4x4[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x08, 0x06, 0x00, 0x00, 0x00, 0xA9, 0xF1, 0x9E,
    0x7E, 0x00, 0x00, 0x00, 0x3B, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0x05, 0xC1, 0x31, 0x01, 0x00,
    0x31, 0x0C, 0x03, 0x31, 0x23, 0x29, 0x88, 0x9B, 0x03, 0xC2, 0x73, 0x90, 0x14, 0x49, 0x67, 0x83,
    0x78, 0x60, 0x86, 0xF0, 0x92, 0x24, 0x64, 0x71, 0x22, 0xA6, 0x62, 0x25, 0x73, 0x6C, 0x26, 0x66,
    0x6B, 0xAE, 0x14, 0xC6, 0x61, 0x13, 0x6E, 0xC3, 0x93, 0xCA, 0xBA, 0xDC, 0x94, 0xD7, 0xF2, 0xFD,
    0x78, 0x6A, 0x17, 0xC1, 0x97, 0x54, 0xC7, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44,
    0xAE, 0x42, 0x60, 0x82,
};
static const size_t kTinyPngRgba4x4Len = sizeof(kTinyPngRgba4x4);

static const uint8_t kTinyPngGrayscale4x4[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x8C, 0x9A, 0xC1,
    0xA2, 0x00, 0x00, 0x00, 0x16, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0x63, 0x60, 0x90, 0xB3, 0x89,
    0x02, 0xE1, 0x0A, 0x06, 0x20, 0x9E, 0xC6, 0x00, 0xC4, 0x5B, 0x00, 0x2A, 0x44, 0x05, 0xA1, 0x59,
    0x26, 0x80, 0x72, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82,
};
static const size_t kTinyPngGrayscale4x4Len = sizeof(kTinyPngGrayscale4x4);

static void expected_png_rgb(int x, int y, int *r, int *g, int *b) {
    *r = std::min(255, x * 80);
    *g = std::min(255, y * 80);
    *b = 50;
}

class ImageLoadPngDecodeTest : public ::testing::Test {

protected:

    char rgb_filename[STRINGSIZE];
    char rgba_filename[STRINGSIZE];
    char gray_filename[STRINGSIZE];

    static void write_fixture(const char *filename, const uint8_t *data, size_t len) {
        FILE *f = fopen(filename, "wb");
        ASSERT_NE(nullptr, f);
        ASSERT_EQ(len, fwrite(data, 1, len, f));
        fclose(f);
    }

    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);

        snprintf(rgb_filename, sizeof(rgb_filename), "/tmp/mmb4l_test_tiny_rgb_%d.png",
                system_getpid());
        write_fixture(rgb_filename, kTinyPngRgb4x4, kTinyPngRgb4x4Len);

        snprintf(rgba_filename, sizeof(rgba_filename), "/tmp/mmb4l_test_tiny_rgba_%d.png",
                system_getpid());
        write_fixture(rgba_filename, kTinyPngRgba4x4, kTinyPngRgba4x4Len);

        snprintf(gray_filename, sizeof(gray_filename), "/tmp/mmb4l_test_tiny_gray_%d.png",
                system_getpid());
        write_fixture(gray_filename, kTinyPngGrayscale4x4, kTinyPngGrayscale4x4Len);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
        remove(rgb_filename);
        remove(rgba_filename);
        remove(gray_filename);
    }
};

TEST_F(ImageLoadPngDecodeTest, GivenRgbPng_DecodesExactPixels) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 4));

    EXPECT_EQ(kOk, image_load_png(&graphics_surfaces[0], rgb_filename, 0, 0, 0, 0));

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int er, eg, eb;
            expected_png_rgb(x, y, &er, &eg, &eb);
            int ar, ag, ab;
            get_surface_rgb(&graphics_surfaces[0], x, y, &ar, &ag, &ab);
            EXPECT_EQ(er, ar) << "at (" << x << "," << y << ") red";
            EXPECT_EQ(eg, ag) << "at (" << x << "," << y << ") green";
            EXPECT_EQ(eb, ab) << "at (" << x << "," << y << ") blue";
        }
    }
}

TEST_F(ImageLoadPngDecodeTest, GivenRgbaPng_Decodes) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 4));

    // RGBA truecolour PNGs take the format==3 branch in image_load_png(),
    // which is distinct code from the plain-RGB (format==1) branch above -
    // this just confirms it doesn't fail/crash and produces the right RGB
    // channels; alpha handling specifics are exercised indirectly via
    // image_draw_buffer()'s skip flags, not re-asserted bit-for-bit here.
    EXPECT_EQ(kOk, image_load_png(&graphics_surfaces[0], rgba_filename, 0, 0, 0, 0));

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int er, eg, eb;
            expected_png_rgb(x, y, &er, &eg, &eb);
            int ar, ag, ab;
            get_surface_rgb(&graphics_surfaces[0], x, y, &ar, &ag, &ab);
            EXPECT_EQ(er, ar) << "at (" << x << "," << y << ") red";
            EXPECT_EQ(eg, ag) << "at (" << x << "," << y << ") green";
            EXPECT_EQ(eb, ab) << "at (" << x << "," << y << ") blue";
        }
    }
}

TEST_F(ImageLoadPngDecodeTest, GivenXyOffset_DrawsAtCorrectPosition) {
    ASSERT_EQ(kOk, graphics_buffer_create(0, 10, 10));

    const MmGraphicsColour kSentinel = RGB(1, 2, 3, 0xFF);
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            graphics_surfaces[0].pixels[y * 10 + x] = kSentinel;
        }
    }

    EXPECT_EQ(kOk, image_load_png(&graphics_surfaces[0], rgb_filename, 3, 2, 0, 0));

    // (3,2) on the surface should be source (0,0).
    int ar, ag, ab;
    get_surface_rgb(&graphics_surfaces[0], 3, 2, &ar, &ag, &ab);
    EXPECT_EQ(0, ar);
    EXPECT_EQ(0, ag);
    EXPECT_EQ(50, ab);

    // (6,5) = (3+3, 2+3) should be source (3,3), the bottom-right corner.
    get_surface_rgb(&graphics_surfaces[0], 6, 5, &ar, &ag, &ab);
    EXPECT_EQ(240, ar);
    EXPECT_EQ(240, ag);
    EXPECT_EQ(50, ab);

    // A pixel outside the drawn 4x4 region should still be the sentinel.
    MmGraphicsColour outside = RGB_BLACK;
    ASSERT_EQ(kOk, graphics_get_pixel(&graphics_surfaces[0], 0, 0, &outside));
    EXPECT_EQ(kSentinel, outside);
}

TEST_F(ImageLoadPngDecodeTest, GivenImageTooLargeForSurface_Fails) {
    // Surface smaller than the 4x4 fixture, with a placement offset that
    // pushes it past the edge -> should fail with kImageTooLarge rather
    // than overflowing. This now correctly exercises surface->width/height
    // rather than graphics_current, and would have crashed before the fix
    // to image_load_png().
    ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 4));

    EXPECT_EQ(kImageTooLarge, image_load_png(&graphics_surfaces[0], rgb_filename, 2, 2, 0, 0));
}

TEST_F(ImageLoadPngDecodeTest, GivenUnsupportedFormat_ReturnsInvalidFormat) {
    // 8-bit grayscale -> upng reports UPNG_LUMINANCE8 (format value 8),
    // which falls outside image_load_png()'s accepted
    // {UPNG_RGB8 (1), UPNG_RGBA8 (3)} set.
    ASSERT_EQ(kOk, graphics_buffer_create(0, 4, 4));

    EXPECT_EQ(kImageInvalidFormat,
             image_load_png(&graphics_surfaces[0], gray_filename, 0, 0, 0, 0));
}
