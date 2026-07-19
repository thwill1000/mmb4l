/*-*****************************************************************************

MMBasic for Linux (MMB4L)

image.h

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

#if !defined(MMBASIC_IMAGE_H)
#define MMBASIC_IMAGE_H

#include "graphics.h"

// Diffusion method, packed into bit 2 of DitherMode.
typedef enum {
    kImageDitherMethodFloydSteinberg = 0,
    kImageDitherMethodAtkinson = 1,
} ImageDitherMethod;

// Target pixel format, packed into bits 1-0 of DitherMode.
typedef enum {
    kImageDitherFormatRgb121 = 0,
    kImageDitherFormatRgb222 = 1,
    kImageDitherFormatRgb332 = 2,
    kImageDitherFormatRgb565 = 3,  // Unimplemented.
} ImageDitherFormat;

// Dithering modes - bit layout: [method(bit2)][format(bits 1-0)]
// Bit 2: 0=Floyd-Steinberg, 1=Atkinson
// Bits 1-0: 00=RGB121, 01=RGB222, 10=RGB332, 11=RGB565 (Unimplemented)
typedef enum {
    kImageDitherNone = -1,
    kImageDitherFsRgb121 = 0,        // 0b000 - Floyd Steinberg RGB121
    kImageDitherFsRgb222 = 1,        // 0b001
    kImageDitherFsRgb332 = 2,        // 0b010
    kImageDitherFsRgb565 = 3,        // 0b011 - Unimplemented
    kImageDitherAtkinsonRgb121 = 4,  // 0b100
    kImageDitherAtkinsonRgb222 = 5,  // 0b101
    kImageDitherAtkinsonRgb332 = 6,  // 0b110
    kImageDitherAtkinsonRgb565 = 7,  // 0b111 - Unimplemented
} ImageDitherMode;

#define IMAGE_DITHER_METHOD(mode) ((ImageDitherMethod) ((mode) >> 2))
#define IMAGE_DITHER_FORMAT(mode) ((ImageDitherFormat) ((mode) & 0x3))

/**
 * Loads a .bmp image.
 *
 * @param  surface   Surface to draw the image on.
 * @param  filename  Name of file to load the image from.
 * @param  x         The x-coordinate on the target surface where the image will be drawn.
 * @param  y         The x-coordinate on the target surface where the image will be drawn.
 * @return           kOk on success, or an error code on failure.
 */
MmResult image_load_bmp(MmSurface *surface, char *filename, int x, int y);

/**
 * Loads a .jpg image
 *
 * @param  surface   Surface to draw the image on.
 * @param  filename  Name of file to load the image from.
 * @param  x         The x-coordinate on the target surface where the image will be drawn.
 * @param  y         The x-coordinate on the target surface where the image will be drawn.
 * @param  mode      Dither mode, kImageDitherNone for no dithering.
 * @param  ximage    The horizontal offset (crop start) within the source JPEG image.
 * @param  yimage    The vertical offset (crop start) within the source JPEG image.
 * @param  scale     Pixel-binning downsample factor: 1, 2, 4 or 8. Each scale x scale
 *                   block of decoded source pixels is averaged into a single output
 *                   pixel, so the image is effectively displayed at 1/scale resolution.
 *                   Dithering (if enabled via mode) is applied at full source resolution
 *                   before binning; averaging the dithered pixels reconstructs an
 *                   approximation of the original colour at the reduced size.
 * @return           kOk on success, or an error code on failure.
 */
MmResult image_load_jpg(MmSurface *surface, char *filename, int x, int y, ImageDitherMode mode,
                        int ximage, int yimage, int scale);

/**
 * Loads a .png image.
 *
 * @param  surface      Surface to draw the image on.
 * @param  filename     Name of file to load the image from.
 * @param  x            The x-coordinate on the target surface where the image will be drawn.
 * @param  y            The x-coordinate on the target surface where the image will be drawn.
 * @param  transparent  TODO
 * @param  force        TODO
 * @return              kOk on success, or an error code on failure.
 */
MmResult image_load_png(MmSurface *surface, char *filename, int x, int y, int transparent,
                        int force);

/**
 * Saves a .bmp image to a file.
 *
 * @param  surface   Surface to read the image from.
 * @param  filename  Name of file to save the image to.
 * @param  format    BMP format to use of the saved image.
 * @param  x         X-coordinate for top left corner of image to save.
 * @param  y         Y-coordinate for top left corner of image to save.
 * @param  width     Width or image to save.
 * @param  height    Height of image to save.
 * @return           kOk on success, or an error code on failure.
 */
MmResult image_save_bmp(MmSurface *surface, char *filename, BmpFormat format, int x, int y,
                        int width, int height);

/**
 * Saves a .jpg image to a file.
 *
 * @param  surface   Surface to read the image from.
 * @param  filename  Name of file to save the image to. A ".jpg" extension is
 *                    appended if it does not already have a ".jpg" or
 *                    ".jpeg" extension.
 * @param  x         X-coordinate for top left corner of image to save.
 * @param  y         Y-coordinate for top left corner of image to save.
 * @param  width     Width of image to save.
 * @param  height    Height of image to save.
 * @param  quality   JPEG quality, clamped to the range 1 (worst) - 100 (best).
 * @return           kOk on success, or an error code on failure.
 */
MmResult image_save_jpg(MmSurface *surface, char *filename, int x, int y, int width, int height,
                        int quality);

#endif // #if !defined(MMBASIC_IMAGE_H)
