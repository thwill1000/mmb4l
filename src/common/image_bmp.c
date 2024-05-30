/*-*****************************************************************************

MMBasic for Linux (MMB4L)

image_bmp.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed  on the console at startup (additional copyright messages may
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

#include "file.h"
#include "graphics.h"
#include "image_bmp.h"
#include "memory.h"
#include "mmb4l.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FILE_READ(_buf, _size, _count, _fnbr) \
    fread(_buf, _size, _count, file_table[_fnbr].file_ptr)

#define MEMORY_ALLOC(_size)  GetMemory(_size)

#define MEMORY_FREE(_ptr)  FreeMemory((void*) _ptr)

#define SET_PIXEL(_xx, _yy, _rgb)  graphics_draw_pixel(surface, _xx + x, _yy + y, _rgb)

#define RGB_TABLE_ENTRY(_idx) \
    RGB(header->colour_table[_idx][0], header->colour_table[_idx][1], header->colour_table[_idx][2], 0xFF)

#define IMG_vCheckAndAbort() CheckAbort()

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pixel_array_offset;
    uint32_t colour_table_size;
    uint8_t bits_per_pixel;
    uint8_t header_type;
    uint8_t bm_marker_flag : 1;
    uint8_t compression_type : 3;
    uint8_t number_of_planes : 3;
    uint8_t b16bit565flag : 1;
    uint8_t colour_table[256][3]; // 3 elements = RGB.
} BitmapHeader;

/**
 * Reads the bitmap header.
 *
 * @param  header  The header is read into this structure.
 * @param  fnbr    MMBasic file number of an open file to read from.
 * @result         0 on success, all other values indicate an error.
 */
static uint8_t image_bmp_read_header(BitmapHeader *header, int fnbr) {
    memset(header, 0, sizeof(BitmapHeader));

    uint8_t byte1, byte2;
    uint32_t data16;
    uint32_t data32;
    FILE_READ(&byte1, 1, 1, fnbr); // Marker
    FILE_READ(&byte2, 1, 1, fnbr); // Marker

    if (byte1 == 'B' && byte2 == 'M') {
        header->bm_marker_flag = 1;
    } else {
        return -1;
    }

    // Ignore file length.
    FILE_READ(&data32, 4, 1, fnbr);

    // Ignore reserved value 1.
    FILE_READ(&data16, 2, 1, fnbr); /* Reserved */

    // Ignore reserved value 2.
    FILE_READ(&data16, 2, 1, fnbr); /* Reserved */

    // Read pixel array offset.
    FILE_READ(&data32, 4, 1, fnbr);
    header->pixel_array_offset = data32;

    FILE_READ(&data32, 4, 1, fnbr); /* Header length */
    header->header_type = (uint8_t)data32;

    if (header->header_type < 40) return 0; // But actually we don't handle this.

    // Read image width.
    FILE_READ(&data32, 4, 1, fnbr);
    header->width = data32;

    // Read image height.
    FILE_READ(&data32, 4, 1, fnbr);
    header->height = data32;

    // Read number of planes.
    FILE_READ(&data16, 2, 1, fnbr);
    header->number_of_planes = (uint8_t)data16;

    // Read bits per pixel.
    FILE_READ(&data16, 2, 1, fnbr);
    header->bits_per_pixel = (uint8_t)data16;

    // Read compression info.
    FILE_READ(&data32, 4, 1, fnbr);
    header->compression_type = (uint8_t)data32;

    // Ignore image size.
    FILE_READ(&data32, 4, 1, fnbr);

    // Ignore x pixels per metre.
    FILE_READ(&data32, 4, 1, fnbr);

    // Ignore y pixels per metre.
    FILE_READ(&data32, 4, 1, fnbr);

    // Read the size of the colour table.
    FILE_READ(&data32, 4, 1, fnbr);
    header->colour_table_size = data32;

    // Handle MSPAINT bug.
    if (header->colour_table_size == 0) {
        uint32_t tmp = (uint32_t)(header->pixel_array_offset - 14 - 40) / 4;
        header->colour_table_size = tmp > 0 ? tmp : 0;
    }

    // Ignore important colour count.
    FILE_READ(&data32, 4, 1, fnbr);

    if (header->bits_per_pixel == 16 && header->header_type > 40) {
        // Read red channel bitmask.
        FILE_READ(&data32, 4, 1, fnbr);
        if (data32 == 0xF800) header->b16bit565flag = 1;
    }

    // Read the colour table.
    if (header->colour_table_size <= 256) {
        for (uint32_t i = 0; i < header->colour_table_size; i++) {
            FILE_READ(&header->colour_table[i][2], 1, 1, fnbr); // Red.
            FILE_READ(&header->colour_table[i][1], 1, 1, fnbr); // Green.
            FILE_READ(&header->colour_table[i][0], 1, 1, fnbr); // Blue.
            FILE_READ(&data16, 1, 1, fnbr); // Dummy byte.
        }
    }

    return 0;
}

static uint8_t image_bmp_load_greyscale(BitmapHeader *header, MmSurface *surface, int x, int y,
                                        int fnbr) {
    if (header->colour_table_size != 0) return -1;

    uint8_t bPadding = (4 - (header->width % 4)) % 4;
    for (uint32_t wY = 0; wY < header->height; wY++) {
        IMG_vCheckAndAbort();
        for (uint32_t wX = 0; wX < header->width; wX++) {
            uint8_t bY;
            FILE_READ(&bY, 1, 1, fnbr); /* Y */
            SET_PIXEL(wX, header->height - wY - 1, RGB(bY, bY, bY, 0xFF));
        }
        for (uint32_t wX = 0; wX < bPadding; wX++) {
            uint8_t bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
        }
    }
    return 0;
}

static uint8_t image_bmp_load_black_and_white(BitmapHeader *header, MmSurface *surface, int x, int y,
                                              int fnbr) {
    if (header->colour_table_size == 0) return -1;

    uint32_t wBytesPerRow = header->width / 8;
    uint8_t bAdditionalBitsPerRow = header->width % 8;
    uint8_t bPadding = (4 - ((wBytesPerRow + (bAdditionalBitsPerRow ? 1 : 0)) % 4)) % 4;
    for (uint32_t wY = 0; wY < header->height; wY++) {
        uint8_t bBits, bValue;
        IMG_vCheckAndAbort();
        uint32_t wX;
        for (wX = 0; wX < wBytesPerRow; wX++) {
            FILE_READ(&bValue, 1, 1, fnbr);

            for (bBits = 0; bBits < 8; bBits++) {
                uint8_t bIndex = (bValue & (0x80 >> bBits)) ? 1 : 0;
                SET_PIXEL(wX * 8 + bBits, header->height - wY - 1, RGB_TABLE_ENTRY(bIndex));
            }
        }
        if (bAdditionalBitsPerRow > 0) {
            FILE_READ(&bValue, 1, 1, fnbr);

            for (bBits = 0; bBits < bAdditionalBitsPerRow; bBits++) {
                uint8_t bIndex = (bValue & (0x80 >> bBits)) ? 1 : 0;
                SET_PIXEL(wX * 8 + bBits, header->height - wY - 1, RGB_TABLE_ENTRY(bIndex));
            }
        }
        for (uint32_t wX = 0; wX < bPadding; wX++) {
            uint8_t bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
        }
    }
    return 0;
}

static uint8_t image_bmp_load_4_bit_colour(BitmapHeader *header, MmSurface *surface, int x, int y,
                                           int fnbr) {
    if (header->colour_table_size == 0) return -1;

    uint32_t wBytesPerRow = header->width / 2;
    uint8_t bAdditionalNibblePerRow = header->width % 2;
    uint8_t bPadding = (4 - ((wBytesPerRow + bAdditionalNibblePerRow) % 4)) % 4;
    for (uint32_t wY = 0; wY < header->height; wY++) {
        IMG_vCheckAndAbort();
        uint32_t wX;
        for (wX = 0; wX < wBytesPerRow; wX++) {
            uint8_t bIndex, bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
            bIndex = bValue >> 4;
            SET_PIXEL(wX * 2, header->height - wY - 1, RGB_TABLE_ENTRY(bIndex));
            bIndex = bValue & 0x0F;
            SET_PIXEL(wX * 2 + 1, header->height - wY - 1, RGB_TABLE_ENTRY(bIndex));
        }
        if (bAdditionalNibblePerRow) {
            uint8_t bIndex, bValue;
            FILE_READ(&bValue, 1, 1, fnbr); /* Bits8 */
            bIndex = bValue >> 4;
            SET_PIXEL(wX * 2, header->height - wY - 1, RGB_TABLE_ENTRY(bIndex));
        }
        for (uint32_t wX = 0; wX < bPadding; wX++) {
            uint8_t bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
        }
    }
    return 0;
}

static uint8_t image_bmp_load_4_bit_colour_compression_2(BitmapHeader *header, MmSurface *surface,
                                                         int x, int y, int fnbr) {
    uint8_t b[2];
    uint8_t count;
    for (uint32_t wY = 0; wY < header->height; wY++) {
        IMG_vCheckAndAbort();
        uint32_t wX = 0;
        do {
            FILE_READ(b, 1, 2, fnbr);
            MmGraphicsColour colour1 = RGB_TABLE_ENTRY(b[1] >> 4);
            MmGraphicsColour colour2 = RGB_TABLE_ENTRY(b[1] & 0xF);
            count = b[0];
            while (count) {
                SET_PIXEL(wX++, header->height - wY - 1, colour1);
                SET_PIXEL(wX++, header->height - wY - 1, colour2);
                count -= 2;
            }
        } while (b[0] != 0 || b[1] != 0);
    }
    return 0;
}

static uint8_t image_bmp_load_8_bit_colour(BitmapHeader *header, MmSurface *surface, int x, int y,
                                           int fnbr) {
    if (header->colour_table_size == 0) return -1;

    uint8_t bPadding = (4 - (header->width % 4)) % 4;
    for (uint32_t wY = 0; wY < header->height; wY++) {
        IMG_vCheckAndAbort();
        for (uint32_t wX = 0; wX < header->width; wX++) {
            uint8_t bIndex;
            FILE_READ(&bIndex, 1, 1, fnbr);
            SET_PIXEL(wX, header->height - wY - 1, RGB_TABLE_ENTRY(bIndex));
        }
        for (uint32_t wX = 0; wX < bPadding; wX++) {
            uint8_t bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
        }
    }
    return 0;
}

static uint8_t image_bmp_load_16_bit_colour(BitmapHeader *header, MmSurface *surface, int x, int y,
                                            int fnbr) {
    uint8_t bPadding = (4 - ((header->width * 2) % 4)) % 4;
    for (uint32_t wY = 0; wY < header->height; wY++) {
        IMG_vCheckAndAbort();
        for (uint32_t wX = 0; wX < header->width; wX++) {
            uint32_t wColor;
            uint8_t bR, bG, bB;
            FILE_READ(&wColor, 2, 1, fnbr); /* RGB */
            if (header->b16bit565flag == 1) {
                bR = (wColor >> 11) << 3;
                bG = ((wColor & 0x07E0) >> 5) << 2;
                bB = (wColor & 0x001F) << 3;
            } else {
                bR = ((wColor & 0x7FFF) >> 10) << 3;
                bG = ((wColor & 0x03E0) >> 5) << 3;
                bB = (wColor & 0x001F) << 3;
            }
            SET_PIXEL(wX, header->height - wY - 1, RGB(bR, bG, bB, 0xFF));
        }
        for (uint32_t wX = 0; wX < bPadding; wX++) {
            uint8_t bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
        }
    }
    return 0;
}

static uint8_t image_bmp_load_24_bit_colour(BitmapHeader *header, MmSurface *surface, int x, int y,
                                          int fnbr) {
    int pp;
    uint8_t bPadding = (4 - ((header->width * 3) % 4)) % 4;
    char* buf = (char*) MEMORY_ALLOC(header->width * sizeof(uint32_t));
    for (uint32_t wY = 0; wY < header->height; wY++) {
        IMG_vCheckAndAbort();
        FILE_READ(buf, 1, header->width * 3, fnbr); /* B */
        pp = 0;
        for (uint32_t wX = 0; wX < header->width; wX++) {
            SET_PIXEL(wX, header->height - wY - 1, RGB(buf[pp], buf[pp + 1], buf[pp + 2], 0xFF));
            pp += 3;
        }
        for (uint32_t wX = 0; wX < bPadding; wX++) {
            uint8_t bValue;
            FILE_READ(&bValue, 1, 1, fnbr);
        }
    }
    MEMORY_FREE(buf);
    return 0;
}

uint8_t image_bmp_load(MmSurface* surface, int x, int y, int fnbr) {
    BitmapHeader header;
    uint8_t result = image_bmp_read_header(&header, fnbr);
    if (result != 0) return result;

    if (header.bm_marker_flag == 0 || header.header_type < 40 ||
        (header.compression_type != 0 && header.compression_type != 2 && header.compression_type != 3)) {
        return -1;
    }

    switch (header.bits_per_pixel) {
        case 1:
            result = image_bmp_load_black_and_white(&header, surface, x, y, fnbr);
            break;
        case 4:
            result = header.compression_type == 2
                    ? image_bmp_load_4_bit_colour_compression_2(&header, surface, x, y, fnbr)
                    : image_bmp_load_4_bit_colour(&header, surface, x, y, fnbr);
            break;
        case 8:
            result = header.colour_table_size == 0
                    ? image_bmp_load_greyscale(&header, surface, x, y, fnbr)
                    : image_bmp_load_8_bit_colour(&header, surface, x, y, fnbr);
            break;
        case 16:
            result = image_bmp_load_16_bit_colour(&header, surface, x, y, fnbr);
            break;
        case 24:
            result = image_bmp_load_24_bit_colour(&header, surface, x, y, fnbr);
            break;
        default:
            result = -1;
            break;
    }

    return result;
}
