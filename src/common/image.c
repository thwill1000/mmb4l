/*-*****************************************************************************

MMBasic for Linux (MMB4L)

image.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#include <assert.h>
#include <string.h>

#include "cstring.h"
#include "file_private.h"
#include "image_private.h"
#include "memory.h"
#include "path.h"
#include "streamio.h"
#include "../core/MMBasic.h"  // for perform_background_tasks()
#include "../third_party/picojpeg.h"
#include "../third_party/spbmp.h"
#include "../third_party/toojpeg_streaming.h"
#include "../third_party/upng.h"

// TODO: duplicated from graphics.c
static inline void graphics_set_pixel_safe(MmSurface *surface, int x, int y, MmGraphicsColour colour) {
    if (x >= 0 && y >= 0 && x < surface->width && y < surface->height) {
        surface->pixels[y*surface->width + x] = colour;
    }
}

static size_t spbmp_file_read_cb(void *file, void *buffer, size_t size, size_t count,
                                 void *userdata) {
    return fread(buffer, size, count, (FILE *) file);
}

static size_t spbmp_file_write_cb(void *file, const void *buffer, size_t size, size_t count,
                                  void *userdata) {
    return fwrite(buffer, size, count, (FILE *) file);
}

static SpColourArgb spbmp_get_pixel_cb(int x, int y, void *userdata) {
    MmGraphicsColour colour = RGB_BLACK;
    assert(graphics_get_pixel((MmSurface *) userdata, x, y, &colour) == kOk);
    return colour >= 0 ? (SpColourArgb) colour : (SpColourArgb) RGB_BLACK;
}

static void spbmp_set_pixel_cb(int x, int y, SpColourArgb colour, void *userdata) {
    graphics_set_pixel_safe((MmSurface *) userdata, x, y, (MmGraphicsColour) colour);
}

static int spbmp_abort_check_cb(void *userdata) {
    perform_background_tasks();
    return 0;
}

MmResult image_load_bmp(MmSurface *surface, char *filename, int x, int y) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;
    char _filename[STRINGSIZE];
    ON_FAILURE_RETURN(path_try_extension(filename, ".bmp", _filename, STRINGSIZE));

    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(_filename, "rb", fnbr));
    spbmp_init(spbmp_file_read_cb, NULL, NULL, spbmp_set_pixel_cb, spbmp_abort_check_cb);
    SpBmpResult bmp_result = spbmp_read(file_table[fnbr].file_ptr, x, y, surface);
    surface->dirty = true;
    ON_FAILURE_LOG(streamio_close(fnbr));
    return SUCCEEDED(bmp_result) ? kOk : kGraphicsLoadBitmapFailed;
}

/** Callback function called by picojpeg to read bytes from file. */
unsigned char pjpeg_file_read_cb(unsigned char *pBuf, unsigned char buf_size,
                                 unsigned char *pBytes_actually_read, void *pCallback_data) {
    const size_t elements_to_read = (size_t)buf_size;
    FILE *file = (FILE *)pCallback_data;
    const size_t elements_read = fread(pBuf, 1, elements_to_read, file);
    if (elements_read < elements_to_read && ferror(file)) {
        *pBytes_actually_read = 0;
        return PJPG_STREAM_READ_ERROR;
    }
    *pBytes_actually_read = (unsigned char)elements_read;
    return 0;
}

void *image_alloc_mem(size_t size) { return GetTempMemory(size); }

void image_free_mem(void *ptr) { ClearSpecificTempMemory(ptr); }

/**
 * Quantizes an (error-adjusted) RGB888 colour to 4-bit RGB121: 1 bit red,
 * 2 bits green, 1 bit blue, packed as [r1][g1 g0][b1] in bits 3..0.
 *
 * Inputs are nominally 0..255 but may fall outside that range because they
 * carry accumulated diffusion error, so they are clamped first. Red and
 * blue are quantized by simple midpoint thresholding (rounded to their one
 * representable "off"/"on" level); green is rounded to the nearest of its
 * 4 representable levels.
 *
 * @param  r  Red component, nominally 0..255 but may be out of range.
 * @param  g  Green component, nominally 0..255 but may be out of range.
 * @param  b  Blue component, nominally 0..255 but may be out of range.
 * @return    Packed 4-bit RGB121 value in the low nibble.
 */
uint8_t image_dither_rgb888_to_rgb121(int16_t r, int16_t g, int16_t b) {
    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
    b = (b < 0) ? 0 : (b > 255) ? 255 : b;
    uint8_t r1 = (r >= 128) ? 1 : 0;
    uint8_t g2 = (g * 3 + 127) / 255;
    uint8_t b1 = (b >= 128) ? 1 : 0;
    return (r1 << 3) | (g2 << 1) | b1;
}

/**
 * Quantizes an (error-adjusted) RGB888 colour to 6-bit RGB222: 2 bits each
 * for red, green and blue, packed as [r1 r0][g1 g0][b1 b0] in bits 5..0.
 *
 * Inputs are nominally 0..255 but may fall outside that range because they
 * carry accumulated diffusion error, so they are clamped first. Each
 * channel is rounded to the nearest of its 4 representable levels.
 *
 * @param  r  Red component, nominally 0..255 but may be out of range.
 * @param  g  Green component, nominally 0..255 but may be out of range.
 * @param  b  Blue component, nominally 0..255 but may be out of range.
 * @return    Packed 6-bit RGB222 value in the low 6 bits.
 */
uint8_t image_dither_rgb888_to_rgb222(int16_t r, int16_t g, int16_t b) {
    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
    b = (b < 0) ? 0 : (b > 255) ? 255 : b;
    uint8_t r2 = (r * 3 + 127) / 255;
    uint8_t g2 = (g * 3 + 127) / 255;
    uint8_t b2 = (b * 3 + 127) / 255;
    return (r2 << 4) | (g2 << 2) | b2;
}

/**
 * Quantizes an (error-adjusted) RGB888 colour to 8-bit RGB332: 3 bits red,
 * 3 bits green, 2 bits blue, packed as [r2 r1 r0][g2 g1 g0][b1 b0].
 *
 * Inputs are nominally 0..255 but may fall outside that range because they
 * carry accumulated diffusion error, so they are clamped first. Red and
 * green are each rounded to the nearest of their 8 representable levels;
 * blue is rounded to the nearest of its 4 representable levels.
 *
 * @param  r  Red component, nominally 0..255 but may be out of range.
 * @param  g  Green component, nominally 0..255 but may be out of range.
 * @param  b  Blue component, nominally 0..255 but may be out of range.
 * @return    Packed 8-bit RGB332 value.
 */
uint8_t image_dither_rgb888_to_rgb332(int16_t r, int16_t g, int16_t b) {
    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
    b = (b < 0) ? 0 : (b > 255) ? 255 : b;
    uint8_t r3 = (r * 7 + 127) / 255;
    uint8_t g3 = (g * 7 + 127) / 255;
    uint8_t b2 = (b * 3 + 127) / 255;
    return (r3 << 5) | (g3 << 2) | b2;
}

/**
 * Expands a packed 4-bit RGB121 value (as produced by
 * image_dither_rgb888_to_rgb121()) to an RGB888 colour.
 *
 * The single red/blue bit is expanded to 0 or 255; the 2-bit green value
 * is scaled evenly across its 4 levels (0, 85, 170, 255).
 *
 * @param  packed  Packed 4-bit RGB121 value (low nibble).
 * @param  r       Out: red component, 0..255.
 * @param  g       Out: green component, 0..255.
 * @param  b       Out: blue component, 0..255.
 */
static inline void image_unpack_rgb121(uint8_t packed, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t r1 = (packed >> 3) & 1;
    uint8_t g2 = (packed >> 1) & 3;
    uint8_t b1 = packed & 1;
    *r = r1 ? 255 : 0;
    *g = (g2 * 255) / 3;
    *b = b1 ? 255 : 0;
}

/**
 * Expands a packed 6-bit RGB222 value (as produced by
 * image_dither_rgb888_to_rgb222()) to an RGB888 colour.
 *
 * Each 2-bit channel is scaled evenly across its 4 levels (0, 85, 170, 255).
 *
 * @param  packed  Packed 6-bit RGB222 value (low 6 bits).
 * @param  r       Out: red component, 0..255.
 * @param  g       Out: green component, 0..255.
 * @param  b       Out: blue component, 0..255.
 */
static inline void image_unpack_rgb222(uint8_t packed, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t r2 = (packed >> 4) & 3;
    uint8_t g2 = (packed >> 2) & 3;
    uint8_t b2 = packed & 3;
    *r = (r2 * 255) / 3;
    *g = (g2 * 255) / 3;
    *b = (b2 * 255) / 3;
}

/**
 * Expands a packed 8-bit RGB332 value (as produced by
 * image_dither_rgb888_to_rgb332()) to an RGB888 colour.
 *
 * The 3-bit red and green channels are scaled evenly across their 8 levels
 * (multiples of 255/7); the 2-bit blue channel is scaled evenly across its
 * 4 levels (0, 85, 170, 255).
 *
 * @param  packed  Packed 8-bit RGB332 value.
 * @param  r       Out: red component, 0..255.
 * @param  g       Out: green component, 0..255.
 * @param  b       Out: blue component, 0..255.
 */
static inline void image_unpack_rgb332(uint8_t packed, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t r3 = (packed >> 5) & 7;
    uint8_t g3 = (packed >> 2) & 7;
    uint8_t b2 = packed & 3;
    *r = (r3 * 255) / 7;
    *g = (g3 * 255) / 7;
    *b = (b2 * 255) / 3;
}

void image_dither_row(uint8_t *row_buffer, int row_width, int16_t *curr_error,
                      int16_t *next_error, ImageDitherMode mode) {
    const ImageDitherMethod method = IMAGE_DITHER_METHOD(mode);
    const ImageDitherFormat format = IMAGE_DITHER_FORMAT(mode);

    for (int x = 0; x < row_width; x++) {
        uint8_t *pDst = row_buffer + (x * 3);

        // Apply error
        int16_t old_r = pDst[0] + curr_error[x * 3 + 0];
        int16_t old_g = pDst[1] + curr_error[x * 3 + 1];
        int16_t old_b = pDst[2] + curr_error[x * 3 + 2];

        // Quantize based on format
        uint8_t packed, new_r, new_g, new_b;

        switch (format) {
            case kImageDitherFormatRgb121:
                packed = image_dither_rgb888_to_rgb121(old_r, old_g, old_b);
                image_unpack_rgb121(packed, &new_r, &new_g, &new_b);
                break;
            case kImageDitherFormatRgb222:
                packed = image_dither_rgb888_to_rgb222(old_r, old_g, old_b);
                image_unpack_rgb222(packed, &new_r, &new_g, &new_b);
                break;
            case kImageDitherFormatRgb332:
                packed = image_dither_rgb888_to_rgb332(old_r, old_g, old_b);
                image_unpack_rgb332(packed, &new_r, &new_g, &new_b);
                break;
            default:  // kImageDitherFormatRgb565 - not yet implemented
                new_r = old_r;
                new_g = old_g;
                new_b = old_b;
                break;
        }

        // Clamp for error calculation
        if (old_r < 0) old_r = 0;
        if (old_r > 255) old_r = 255;
        if (old_g < 0) old_g = 0;
        if (old_g > 255) old_g = 255;
        if (old_b < 0) old_b = 0;
        if (old_b > 255) old_b = 255;

        // Calculate errors
        int16_t err_r = old_r - new_r;
        int16_t err_g = old_g - new_g;
        int16_t err_b = old_b - new_b;

        // Write result
        pDst[0] = new_r;
        pDst[1] = new_g;
        pDst[2] = new_b;

        // Distribute error
        if (method == kImageDitherMethodFloydSteinberg) {
            // Floyd-Steinberg distribution
            if (x + 1 < row_width) {
                curr_error[(x + 1) * 3 + 0] += (err_r * 7) / 16;
                curr_error[(x + 1) * 3 + 1] += (err_g * 7) / 16;
                curr_error[(x + 1) * 3 + 2] += (err_b * 7) / 16;
            }

            if (x > 0) {
                next_error[(x - 1) * 3 + 0] += (err_r * 3) / 16;
                next_error[(x - 1) * 3 + 1] += (err_g * 3) / 16;
                next_error[(x - 1) * 3 + 2] += (err_b * 3) / 16;
            }

            next_error[x * 3 + 0] += (err_r * 5) / 16;
            next_error[x * 3 + 1] += (err_g * 5) / 16;
            next_error[x * 3 + 2] += (err_b * 5) / 16;

            if (x + 1 < row_width) {
                next_error[(x + 1) * 3 + 0] += (err_r * 1) / 16;
                next_error[(x + 1) * 3 + 1] += (err_g * 1) / 16;
                next_error[(x + 1) * 3 + 2] += (err_b * 1) / 16;
            }
        } else {
            // Atkinson distribution
            if (x + 1 < row_width) {
                curr_error[(x + 1) * 3 + 0] += err_r / 8;
                curr_error[(x + 1) * 3 + 1] += err_g / 8;
                curr_error[(x + 1) * 3 + 2] += err_b / 8;
            }

            if (x + 2 < row_width) {
                curr_error[(x + 2) * 3 + 0] += err_r / 8;
                curr_error[(x + 2) * 3 + 1] += err_g / 8;
                curr_error[(x + 2) * 3 + 2] += err_b / 8;
            }

            if (x > 0) {
                next_error[(x - 1) * 3 + 0] += err_r / 8;
                next_error[(x - 1) * 3 + 1] += err_g / 8;
                next_error[(x - 1) * 3 + 2] += err_b / 8;
            }

            next_error[x * 3 + 0] += err_r / 8;
            next_error[x * 3 + 1] += err_g / 8;
            next_error[x * 3 + 2] += err_b / 8;

            if (x + 1 < row_width) {
                next_error[(x + 1) * 3 + 0] += err_r / 8;
                next_error[(x + 1) * 3 + 1] += err_g / 8;
                next_error[(x + 1) * 3 + 2] += err_b / 8;
            }
        }
    }
}

void image_bin_row(const uint8_t *mcu_row_buffer, int mcu_row_width, int rows_available,
                   int src_x_start, int src_width, int scale, int out_width,
                   uint8_t *out_row_buffer) {
    for (int ox = 0; ox < out_width; ox++) {
        int sx0 = src_x_start + ox * scale;
        uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
        int count = 0;

        for (int dy = 0; dy < scale && dy < rows_available; dy++) {
            const uint8_t *sp = mcu_row_buffer + (size_t) dy * mcu_row_width + sx0 * 3;
            for (int dx = 0; dx < scale; dx++) {
                if (sx0 + dx >= src_width) break;
                sum_r += sp[0];
                sum_g += sp[1];
                sum_b += sp[2];
                sp += 3;
                count++;
            }
        }

        if (count == 0) continue;

        // Average the sub-area, rounding to nearest.
        uint8_t *op = out_row_buffer + ox * 3;
        op[0] = (uint8_t) ((sum_r + count / 2) / count);
        op[1] = (uint8_t) ((sum_g + count / 2) / count);
        op[2] = (uint8_t) ((sum_b + count / 2) / count);
    }
}

MmResult image_load_jpg(MmSurface *surface, char *filename, int x, int y, ImageDitherMode mode,
                        int ximage, int yimage, int scale) {
    if (!surface || surface->type == kGraphicsNone) {
        return kGraphicsInvalidWriteSurface;
    }
    if (mode == kImageDitherFsRgb565 || mode == kImageDitherAtkinsonRgb565) {
        return mmresult_ex(kInvalidArgument,
                           "Invalid mode: %d; RGB565 modes are currently unsupported", mode);
    }
    if (scale != 1 && scale != 2 && scale != 4 && scale != 8) {
        return mmresult_ex(kInvalidArgument,
                           "Invalid scale: %d; valid values are 1, 2, 4 or 8", scale);
    }

    char _filename[STRINGSIZE];
    ON_FAILURE_RETURN(path_try_extension(filename, ".jpg", _filename, STRINGSIZE));

    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(_filename, "rb", fnbr));
    FILE *file = file_table[fnbr].file_ptr;

    MmResult result = kGraphicsLoadBitmapFailed;
    unsigned char *mcu_row_buffer = NULL;
    unsigned char *scaled_line_buffer = NULL;
    int16_t *error_buffer_0 = NULL;
    int16_t *error_buffer_1 = NULL;

    // Allocate working buffers
    if (FAILED(picojpeg_alloc(image_alloc_mem, image_free_mem))) {
        result = kOutOfMemory;
        goto cleanup;
    }

    pjpeg_image_info_t image_info;
    uint8_t status = pjpeg_decode_init(&image_info, pjpeg_file_read_cb, file, 0);
    switch (status) {
        case 0:
            // OK, do nothing
            break;
        case PJPG_UNSUPPORTED_MODE:
            result = mmresult_ex(kGraphicsLoadBitmapFailed, "Unsupported progressive JPEG");
            goto cleanup;
            break;
        default:
            result = kGraphicsLoadBitmapFailed;
            goto cleanup;
            break;
    }

    int mcu_x = 0;
    int mcu_y = 0;

    // Buffer for one MCU-row's worth of decoded pixels, stored as packed
    // 24-bit RGB triplets. Buffering a whole row (rather than writing
    // directly to the surface, block by block, as before) is required so
    // that error-diffusion dithering can look at, and update, a complete
    // row of pixels before it is drawn.
    const int mcu_row_width = image_info.m_width * 3;
    const int mcu_row_height = image_info.m_MCUHeight;
    mcu_row_buffer = (unsigned char *)image_alloc_mem((size_t)mcu_row_height * mcu_row_width);
    if (!mcu_row_buffer) {
        result = kOutOfMemory;
        goto cleanup;
    }

    // Holds one binned output row when scale > 1: each output pixel is the
    // average of a scale x scale block of decoded source pixels.
    if (scale > 1) {
        scaled_line_buffer =
            (unsigned char *)image_alloc_mem((size_t)(image_info.m_width / scale + 1) * 3);
        if (!scaled_line_buffer) {
            result = kOutOfMemory;
            goto cleanup;
        }
    }

    // Error-diffusion accumulator buffers, one for the row currently being
    // dithered and one for the row below it; they are swapped after each
    // row and must persist across MCU-row boundaries.
    if (mode >= 0) {
        const size_t err_size = (size_t)image_info.m_width * 3 * sizeof(int16_t);
        error_buffer_0 = (int16_t *)image_alloc_mem(err_size);
        error_buffer_1 = (int16_t *)image_alloc_mem(err_size);
        if (!error_buffer_0 || !error_buffer_1) {
            result = kOutOfMemory;
            goto cleanup;
        }
        memset(error_buffer_0, 0, err_size);
        memset(error_buffer_1, 0, err_size);
    }

    int16_t *curr_error = error_buffer_0;
    int16_t *next_error = error_buffer_1;

    // Decode all MCUs, one MCU-row at a time.
    for (mcu_y = 0; mcu_y < image_info.m_MCUSPerCol; mcu_y++) {
        int image_y = mcu_y * image_info.m_MCUHeight;

        // Skip MCU rows before yimage offset
        if (image_y + image_info.m_MCUHeight <= yimage) {
            // Still need to decode to maintain state, just don't display
            for (mcu_x = 0; mcu_x < image_info.m_MCUSPerRow; mcu_x++) {
                status = pjpeg_decode_mcu();
                switch (status) {
                    case 0:
                        // OK, do nothing
                        break;
                    case PJPG_NO_MORE_BLOCKS:
                        goto decode_complete;
                        break;
                    case PJPG_UNSUPPORTED_MODE:
                        result = mmresult_ex(kGraphicsLoadBitmapFailed,
                                             "Unsupported progressive JPEG");
                        goto cleanup;
                        break;
                    default:
                        result = kGraphicsLoadBitmapFailed;
                        goto cleanup;
                        break;
                }
            }
            continue;
        }

        // Stop if we've reached the bottom of the display. With binning, each
        // screen row consumes "scale" source rows, so the visible source height
        // grows by the same factor.
        const int src_visible_limit = (scale > 1)
                ? yimage + scale * (surface->height - y)
                : surface->height + yimage;
        if (image_y >= src_visible_limit) {
            break;
        }

        // Decode MCUs in this row into mcu_row_buffer.
        //
        // The MCU buffers are organised as one or more 8x8 blocks (see the
        // layout diagram in picojpeg.h), so - as in the original reference
        // implementation - we walk block by block and, within each block,
        // read the 64 bytes sequentially via pointer increment. This avoids
        // recomputing a per-pixel offset formula that is only valid at
        // block boundaries.
        for (mcu_x = 0; mcu_x < image_info.m_MCUSPerRow; mcu_x++) {
            status = pjpeg_decode_mcu();
            if (status) {
                if (status != PJPG_NO_MORE_BLOCKS) {
                    goto cleanup;
                }
                goto decode_complete;
            }

            int mcu_x_offset = mcu_x * image_info.m_MCUWidth;

            for (int by0 = 0; by0 < image_info.m_MCUHeight; by0 += 8) {
                const int by_limit =
                    min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + by0));

                for (int bx0 = 0; bx0 < image_info.m_MCUWidth; bx0 += 8) {
                    unsigned src_ofs = (bx0 * 8U) + (by0 * 16U);
                    const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
                    const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
                    const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

                    const int bx_limit = min(8, image_info.m_width - (mcu_x_offset + bx0));

                    for (int by = 0; by < by_limit; by++) {
                        uint8_t *pDst =
                            mcu_row_buffer + (by0 + by) * mcu_row_width + (mcu_x_offset + bx0) * 3;

                        for (int bx = 0; bx < bx_limit; bx++) {
                            pDst[0] = *pSrcR++;
                            pDst[1] = *pSrcG++;
                            pDst[2] = *pSrcB++;
                            pDst += 3;
                        }

                        // Advance source pointers past any unread columns in this
                        // block row (i.e. the part of the 8x8 block beyond bx_limit,
                        // which lies outside the image).
                        pSrcR += (8 - bx_limit);
                        pSrcG += (8 - bx_limit);
                        pSrcB += (8 - bx_limit);
                    }
                }
            }
        }

        // Dither this MCU row, one image row at a time. We must process
        // every row (even ones that fall outside yimage/surface bounds) to
        // maintain correct error-diffusion state between rows.
        if (mode >= 0) {
            for (int row = 0; row < mcu_row_height; row++) {
                int img_y = mcu_y * mcu_row_height + row;
                if (img_y >= image_info.m_height) break;

                uint8_t *row_ptr = mcu_row_buffer + row * mcu_row_width;
                image_dither_row(row_ptr, image_info.m_width, curr_error, next_error, mode);

                // Swap buffers for next row
                int16_t *temp = curr_error;
                curr_error = next_error;
                next_error = temp;

                // Clear next buffer
                memset(next_error, 0, (size_t)image_info.m_width * 3 * sizeof(int16_t));
            }
        }

        // Draw this (possibly dithered) MCU row to the surface, honouring
        // the x/y placement and ximage/yimage crop offsets.
        if (scale == 1) {
            for (int row = 0; row < image_info.m_MCUHeight; row++) {
                int image_line_y = image_y + row;
                if (image_line_y >= image_info.m_height) break;
                if (image_line_y < yimage) continue;

                int screen_y = y + (image_line_y - yimage);
                if (screen_y < 0 || screen_y >= surface->height) continue;

                const uint8_t *row_ptr = mcu_row_buffer + row * mcu_row_width;

                for (int image_x = ximage; image_x < image_info.m_width; image_x++) {
                    int screen_x = x + (image_x - ximage);
                    if (screen_x < 0 || screen_x >= surface->width) continue;

                    const uint8_t *p = row_ptr + image_x * 3;
                    const MmGraphicsColour colour = RGB(p[0], p[1], p[2], 0xFF);
                    graphics_set_pixel_safe(surface, screen_x, screen_y, colour);
                }
            }
        } else {
            for (int row = 0; row < image_info.m_MCUHeight; row += scale) {
                int image_line_y = image_y + row;  // top source row of this bin
                if (image_line_y >= image_info.m_height) break;
                if (image_line_y < yimage) continue;

                int screen_y = y + (image_line_y - yimage) / scale;
                if (screen_y < 0 || screen_y >= surface->height) continue;

                int available_out = (image_info.m_width - ximage) / scale;
                if (available_out <= 0) continue;
                if (x >= surface->width) continue;

                // Rows available in this bin, clamped by both the MCU-row boundary and
                // the bottom of the image (mirrors the two separate break-conditions
                // the old inline loop checked per-dy; both are monotonic in dy so a
                // single min() bound is equivalent).
                int rows_available = image_info.m_MCUHeight - row;
                const int rows_left_in_image = image_info.m_height - image_line_y;
                if (rows_left_in_image < rows_available) rows_available = rows_left_in_image;

                image_bin_row(mcu_row_buffer + (size_t)row * mcu_row_width, mcu_row_width,
                              rows_available, ximage, image_info.m_width, scale, available_out,
                              scaled_line_buffer);

                for (int ox = 0; ox < available_out; ox++) {
                    int screen_x = x + ox;
                    if (screen_x < 0 || screen_x >= surface->width) continue;

                    const uint8_t *p = scaled_line_buffer + ox * 3;
                    const MmGraphicsColour colour = RGB(p[0], p[1], p[2], 0xFF);
                    graphics_set_pixel_safe(surface, screen_x, screen_y, colour);
                }
            }
        }
    }

decode_complete:
    surface->dirty = true;
    result = kOk;

cleanup:
    if (mcu_row_buffer) image_free_mem(mcu_row_buffer);
    if (scaled_line_buffer) image_free_mem(scaled_line_buffer);
    if (error_buffer_0) image_free_mem(error_buffer_0);
    if (error_buffer_1) image_free_mem(error_buffer_1);
    ON_FAILURE_LOG(streamio_close(fnbr));
    picojpeg_free(image_free_mem);

    return result;
}

void image_draw_buffer(MmSurface *surface, int x1, int y1, int x2, int y2,
                              const unsigned char *buffer, int skip) {
    const unsigned char *psrc = buffer;
    union colourmap {
        char rgbbytes[4];
        uint32_t rgb;
    } c;

    // make sure the coordinates are kept within the display area
    if (x2 <= x1) SWAP(int, x1, x2);
    if (y2 <= y1) SWAP(int, y1, y2);

    for (int y = y1; y <= y2; y++) {
        uint32_t *pdst = surface->pixels + (y * surface->width + x1);
        for (int x = x1; x <= x2; x++) {
            if (x >= 0 && x < surface->width && y >= 0 && y < surface->height) {
                if (skip & 2) {
                    c.rgbbytes[3] = 0xFF;     // assume solid colour
                    c.rgbbytes[2] = *psrc++;  // this order swaps the bytes to match the .BMP file
                    c.rgbbytes[1] = *psrc++;
                    c.rgbbytes[0] = *psrc++;
                    if (skip & 1) c.rgbbytes[3] = *psrc++;  // ARGB8888 so set transparency
                } else {
                    c.rgbbytes[3] = 0;
                    c.rgbbytes[0] = *psrc++;  // this order swaps the bytes to match the .BMP file
                    c.rgbbytes[1] = *psrc++;
                    c.rgbbytes[2] = *psrc++;
                    if (skip & 1) psrc++;
                }
                *pdst = c.rgb;
            } else {
                psrc += (skip & 1) ? 4 : 3;
            }
            pdst++;
        }
    }
}

MmResult image_load_png(MmSurface *surface, char *filename, int x, int y, int transparent,
                        int force) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;
    char _filename[STRINGSIZE];
    MmResult result = path_try_extension(filename, ".png", _filename, STRINGSIZE);
    if (FAILED(result)) return result;

    upng_t *upng = upng_new_from_file(_filename);
    // routinechecks(1);
    upng_header(upng);
    const int w = upng_get_width(upng);
    const int h = upng_get_height(upng);
    if (x + w > surface->width || y + h > surface->height) {
        upng_free(upng);
        return kImageTooLarge;
    }
    if (!(upng_get_format(upng)==1 || upng_get_format(upng)==3)){
        upng_free(upng);
        return kImageInvalidFormat;
    }
    // routinechecks(1);
    upng_decode(upng);
    // routinechecks(1);
    const unsigned char *buffer = upng_get_buffer(upng);
    // int savey = optiony;
    // optiony = 0;
    if (upng_get_format(upng) ==3) {
        image_draw_buffer(surface, x, y, x + w - 1, y + h - 1, buffer, 3 | transparent | force);
    } else {
        image_draw_buffer(surface, x, y, x + w - 1, y + h - 1, buffer, 2 | transparent | force);
    }
    // optiony = savey;
    upng_free(upng);
    // clearrepeat();
    surface->dirty = true;
    return kOk;
}

MmResult image_save_bmp(MmSurface *surface, char *filename, BmpFormat format, int x, int y,
                        int width, int height) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidReadSurface;
    char _filename[STRINGSIZE];
    if (FAILED(cstring_cpy(_filename, filename, sizeof(_filename)))) return kFilenameTooLong;

    // If the filename does not have a ".bmp" extension then add one.
    if (cstring_casecmp(path_get_extension(_filename), ".bmp") != 0) {
        if (FAILED(cstring_cat(_filename, ".bmp", STRINGSIZE))) return kFilenameTooLong;
    }

    SpBmpFormat spFormat;
    switch (format) {
        case kBmpFormat1bpp:
            spFormat = kSpBmp1bpp;
            break;
        case kBmpFormat4bppRgb121:
            spFormat = kSpBmp4bppRgb121;
            break;
        case kBmpFormat4bppRgb121Rle4:
            spFormat = kSpBmp4bppRgb121Rle4;
            break;
        case kBmpFormat8bppRgb222:
            spFormat = kSpBmp8bppRgb222;
            break;
        case kBmpFormat8bppRgb222Rle8:
            spFormat = kSpBmp8bppRgb222Rle8;
            break;
        case kBmpFormat8bppRgb332:
            spFormat = kSpBmp8bppRgb332;
            break;
        case kBmpFormat8bppRgb332Rle8:
            spFormat = kSpBmp8bppRgb332Rle8;
            break;
        case kBmpFormat16bppRgb555:
            spFormat = kSpBmp16bppRgb555;
            break;
        case kBmpFormat16bppRgb565:
            spFormat = kSpBmp16bppRgb565;
            break;
        case kBmpFormat24bpp:
            spFormat = kSpBmp24bpp;
            break;
        case kBmpFormat32bpp:
            spFormat = kSpBmp32bpp;
            break;
        default:
            return kImageInvalidFormat;
    }

    // Open the file for writing.
    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(_filename, "wb", fnbr));

    // Write the bitmap to the file.
    spbmp_init(NULL, spbmp_file_write_cb, spbmp_get_pixel_cb, NULL, spbmp_abort_check_cb);
    SpBmpResult bmp_result = spbmp_write(file_table[fnbr].file_ptr, spFormat, surface, x, y,
                                         width, height);

    ON_FAILURE_LOG(streamio_close(fnbr));

    return SUCCEEDED(bmp_result) ? kOk : kGraphicsSaveBitmapFailed;
}

/** Callback given to toojpeg_write_streaming(); writes a chunk of encoded bytes to a FILE*. */
static int image_jpg_write_cb(const void *buffer, size_t size, void *userdata) {
    FILE *file = (FILE *) userdata;
    return fwrite(buffer, 1, size, file) == size;
}

void image_jpg_get_row_cb(unsigned short row, unsigned char *row_buffer, void *userdata) {
    const ImageJpgRowSource *src = (const ImageJpgRowSource *) userdata;
    const int surface_y = src->y + row;
    unsigned char *dst = row_buffer;
    for (int col = 0; col < src->width; col++) {
        MmGraphicsColour colour = RGB_BLACK;
        (void) graphics_get_pixel(src->surface, src->x + col, surface_y, &colour);
        // graphics_get_pixel() always returns kOk and instead signals an
        // out-of-range (x, y) by writing -1 into *colour (see
        // spbmp_get_pixel_cb() for the same convention) - map that back to
        // RGB_BLACK rather than encoding 0xFFFFFFFF into the JPEG row.
        if (colour < 0) colour = RGB_BLACK;
        *dst++ = (colour >> 16) & 0xFF;  // Red
        *dst++ = (colour >> 8) & 0xFF;   // Green
        *dst++ = colour & 0xFF;          // Blue
    }
}

MmResult image_save_jpg(MmSurface *surface, char *filename, int x, int y, int width, int height,
                        int quality) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidReadSurface;
    if (width <= 0 || height <= 0 || width > 65535 || height > 65535) return kImageTooLarge;

    char _filename[STRINGSIZE];
    if (FAILED(cstring_cpy(_filename, filename, sizeof(_filename)))) return kFilenameTooLong;

    // If the filename does not already have a ".jpg"/".jpeg" extension then add one.
    if (cstring_casecmp(path_get_extension(_filename), ".jpg") != 0
            && cstring_casecmp(path_get_extension(_filename), ".jpeg") != 0) {
        if (FAILED(cstring_cat(_filename, ".jpg", STRINGSIZE))) return kFilenameTooLong;
    }

    // Unlike a flat-buffer encoder, TooJpeg's streaming variant only ever
    // needs a handful of image rows resident at once (8, or 16 if chroma
    // downsampling is enabled) - it pulls them on demand via
    // image_jpg_get_row_cb() below, reading straight from the surface.
    const int downsample = 0;
    const size_t window_size = toojpeg_row_window_size(width, /* is_rgb */ 1, downsample);
    unsigned char *row_window = (unsigned char *) image_alloc_mem(window_size);
    if (!row_window) return kOutOfMemory;

    const int fnbr = streamio_find_free();
    MmResult result = streamio_open(_filename, "wb", fnbr);
    if (FAILED(result)) {
        image_free_mem(row_window);
        return result;
    }

    ImageJpgRowSource row_source = { surface, x, y, width };
    const int ok = toojpeg_write_streaming(
        width, height, /* is_rgb */ 1, quality, downsample, /* comment */ NULL,
        image_jpg_get_row_cb, &row_source, row_window, image_jpg_write_cb,
        file_table[fnbr].file_ptr);

    image_free_mem(row_window);
    ON_FAILURE_LOG(streamio_close(fnbr));

    return ok ? kOk : kGraphicsSaveBitmapFailed;
}
