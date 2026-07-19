/*-*****************************************************************************

MMBasic for Linux (MMB4L)

image_private.h

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

#if !defined(MMB4L_IMAGE_PRIVATE)
#define MMB4L_IMAGE_PRIVATE

#include "image.h"

/**
 * Source context for image_jpg_get_row_cb(): identifies which surface,
 * region, and width a row-fetch callback should read from.
 */
typedef struct {
    MmSurface *surface;
    int x;
    int y;
    int width;
} ImageJpgRowSource;

/**
 * Callback matching ToojpegGetRowCb, given to toojpeg_write_streaming().
 * Fills row_buffer with one row of packed RGB888 pixel data (3 bytes per
 * pixel, R,G,B order), read directly from userdata->surface at pixel row
 * (userdata->y + row), columns [userdata->x, userdata->x + userdata->width).
 *
 * Pure with respect to its inputs other than the surface read - no
 * allocation, no file I/O, no encoder state - so it can be unit-tested
 * directly against a surface fixture without going through TooJpeg at all.
 *
 * Pixels outside the surface bounds read as RGB_BLACK (0,0,0), matching
 * graphics_get_pixel()'s existing out-of-bounds behaviour.
 *
 * @param  row        Row index relative to userdata->y (0-based).
 * @param  row_buffer  Destination buffer, at least userdata->width * 3 bytes.
 * @param  userdata    Pointer to an ImageJpgRowSource.
 */
void image_jpg_get_row_cb(unsigned short row, unsigned char *row_buffer, void *userdata);

/**
 * Bins one output row of pixels by averaging scale x scale blocks of source
 * pixels read from mcu_row_buffer. Pure function - no I/O, no MmSurface.
 *
 * @param mcu_row_buffer  Source buffer of packed RGB triplets, already
 *                        positioned at the first source row of the bin
 *                        (i.e. caller passes base + row * mcu_row_width).
 * @param mcu_row_width   Stride of mcu_row_buffer in bytes.
 * @param rows_available  Number of valid source rows starting at
 *                        mcu_row_buffer (already clamped by the caller to
 *                        both the MCU-row height and the image height; may
 *                        be less than scale, or 0).
 * @param src_x_start     X pixel-column offset in the source row where
 *                        binning starts (crop offset, e.g. ximage).
 * @param src_width       Width in pixels of a source row; bounds column
 *                        clipping at the right edge.
 * @param scale           Bin factor (pixels averaged per side).
 * @param out_width       Number of output (binned) pixels to produce.
 * @param out_row_buffer  Destination buffer, out_width * 3 bytes. Pixels
 *                        whose source block is entirely out of range
 *                        (count == 0) are left unmodified, matching the
 *                        original inline implementation's "continue"
 *                        behaviour.
 */
void image_bin_row(const uint8_t *mcu_row_buffer, int mcu_row_width, int rows_available,
                   int src_x_start, int src_width, int scale, int out_width,
                   uint8_t *out_row_buffer);

/**
 * Applies error-diffusion dithering to one row of decoded JPEG pixels
 * in-place, and updates the error buffers so the same error diffusion can
 * continue on subsequent rows.
 *
 * row_buffer holds row_width pixels, each 3 bytes, in R,G,B order (matching
 * how image_load_jpg() fills mcu_row_buffer and how it is later read back
 * for the final blit); this must stay in sync with that layout.
 *
 * For each pixel: any already-diffused error for that pixel (from
 * curr_error) is added to the raw R/G/B values, the result is quantized to
 * the target format (RGB121/RGB222/RGB332, selected by
 * IMAGE_DITHER_FORMAT(mode)) and immediately expanded back to RGB888, and the
 * pixel in row_buffer is overwritten with that quantized-then-expanded colour.
 * The quantization error (clamped original minus quantized-and-expanded result)
 * is then diffused to not-yet-processed neighbours:
 * RGB565 (IMAGE_DITHER_FORMAT(mode)==3) is not implemented and passes each
 * pixel through unchanged, contributing no error.
 *
 * curr_error holds pending error for the row currently being processed
 * (indexed the same as row_buffer, 3 int16_t's per pixel: R, G, B) and
 * next_error accumulates error to be applied when the following row is
 * processed; next_error is written but not cleared by this function, so
 * the caller must swap and clear the buffers between rows.
 *
 * Distribution pattern depends on IMAGE_DITHER_METHOD(mode):
 * - Floyd-Steinberg (method 0): classic 7/16, 3/16, 5/16, 1/16 kernel,
 *   distributing to the next pixel in the current row and to the
 *   pixel below-left, directly below, and below-right in next_error.
 * - Atkinson (method 1): 1/8 to each of the two next pixels in the current
 *   row (via curr_error) and 1/8 to each of below-left, directly below,
 *   and below-right in next_error (6/8 of the error is diffused in total;
 *   the remainder is discarded, as in the original Atkinson algorithm).
 *
 * @param  row_buffer   Packed R,G,B pixel row to dither in-place, row_width
 *                      pixels (3 bytes each).
 * @param  row_width    Number of pixels in the row.
 * @param  curr_error   Pending per-channel error for this row, updated for
 *                      any error diffused within the row itself.
 * @param  next_error   Accumulator for per-channel error to diffuse into
 *                      the following row.
 * @param  mode         Dither mode (method + target format packed together);
 *                      see ImageDitherMode.
 */
void image_dither_row(uint8_t *row_buffer, int row_width, int16_t *curr_error,
                      int16_t *next_error, ImageDitherMode mode);


#endif // #if !defined(MMB4L_IMAGE_PRIVATE)
