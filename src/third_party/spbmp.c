// spBMP - a Microsoft Windows .bmp decoder.
// Copyright (c) 2024 Thomas Hugo Williams
// License MIT <https://opensource.org/licenses/MIT>
//
// 08-Sep-2024: Version 1.0.1 - Simplified BmpHeader and made some cosmetic changes.
// 08-Sep-2024: Version 1.0.0 - Initial offering.

#include "spbmp.h"

#include <stdbool.h>
#include <string.h>

#define RGB(r, g, b, a) \
    (SpColourRgba)(((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF))

#define RGB_TABLE_ENTRY(_idx) RGB( \
    header->colour_table[_idx][0], \
    header->colour_table[_idx][1], \
    header->colour_table[_idx][2], \
    0xFF)

typedef struct {
    uint32_t file_size;
    uint32_t pixel_array_offset;
    int32_t width;
    int32_t height;
    uint8_t header_size;
    uint8_t bits_per_pixel;
    uint8_t bm_marker_flag;
    uint8_t compression_type;
    uint32_t colour_table_size;
    uint32_t important_colour_count;
    uint8_t colour_table[256][3];  // 3 elements = RGB.
    bool rgb565_flag;
} BmpHeader;

static SpBmpFileReadCb spbmp_file_read_cb;
static SpBmpSetPixelCb spbmp_set_pixel_cb;
static SpBmpAbortCheckCb spbmp_abort_check_cb;

void spbmp_init(SpBmpFileReadCb file_read_cb, SpBmpSetPixelCb set_pixel_cb,
                SpBmpAbortCheckCb abort_check_cb) {
    spbmp_file_read_cb = file_read_cb;
    spbmp_set_pixel_cb = set_pixel_cb;
    spbmp_abort_check_cb = abort_check_cb;
}

/**
 * Reads the bitmap header.
 *
 * @param  file      Opaque pointer to file that should be read.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  header    The header is read into this structure.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_read_header(void *file, void *userdata, BmpHeader *header) {

    // An alternative to the following implementation is to re-define BmpHeader to
    // be binary equivalent to the 14-byte "Bitmap File Header" + 40-byte "DIB Header"
    // and read it in as a single blob, e.g.
    //
    //     spbmp_file_read_cb(file, header, 54, 1);
    //
    // However this is "fragile" requiring the struct to be "packed" and many
    // of the 32-bit fields to be not 32-bit aligned.

    memset(header, 0, sizeof(BmpHeader));

    int counter = 0;
    uint32_t dword;
    uint8_t buf[4];

    ////////////////////////////////////////////////////////////
    // 14-byte Bitmap File Header
    ////////////////////////////////////////////////////////////

    spbmp_file_read_cb(file, buf, 2, 1, userdata);  // Signature
    if (buf[0] == 'B' && buf[1] == 'M') {
        header->bm_marker_flag = 1;
    } else {
        return kSpBmpError;
    }

    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // File size
    header->file_size = dword;
    spbmp_file_read_cb(file, &dword, 2, 1, userdata);  // IGNORED - Reserved value 1
    spbmp_file_read_cb(file, &dword, 2, 1, userdata);  // IGNORED - Reserved value 2
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // Pixel array offset
    header->pixel_array_offset = dword;

    counter += 14;

    ////////////////////////////////////////////////////////////
    // 40-byte DIB Header
    ////////////////////////////////////////////////////////////

    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // Header size
    header->header_size = (uint8_t)dword;
    if (header->header_size < 40) return kSpBmpError;

    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // Image width
    header->width = dword;
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // Image height
    header->height = dword;
    spbmp_file_read_cb(file, &dword, 2, 1, userdata);  // IGNORED - Number of planes
    spbmp_file_read_cb(file, &dword, 2, 1, userdata);  // Bits per pixel
    header->bits_per_pixel = (uint8_t)dword;
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // File compression
    header->compression_type = (uint8_t)dword;
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // IGNORED - Image size
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // IGNORED - X pixels per metre
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // IGNORED - Y pixels per metre
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // Size of the colour table
    header->colour_table_size = dword;
    spbmp_file_read_cb(file, &dword, 4, 1, userdata);  // Important colour count
    header->important_colour_count = dword;

    // Handle MSPAINT bug.
    if (header->colour_table_size == 0) {
        uint32_t tmp = (uint32_t)(header->pixel_array_offset - 14 - 40) / 4;
        header->colour_table_size = tmp > 0 ? tmp : 0;
    }

    // Catch various insanities.
    if (header->pixel_array_offset > header->file_size) return kSpBmpError;
    if (header->pixel_array_offset == 0xFFFFFFFF) return kSpBmpError;
    if (header->pixel_array_offset == 0) return kSpBmpError;
    if (header->height == 0) return kSpBmpError;
    if (header->width <= 0) return kSpBmpError;
    if ((int64_t)header->width * (int64_t)header->height >= INT32_MAX) return kSpBmpError;
    if (header->compression_type > 3)  return kSpBmpError;
    if (header->colour_table_size == 0 && header->bits_per_pixel == 8) return kSpBmpError;
    if (header->important_colour_count > header->colour_table_size) return kSpBmpError;

    counter += 40;

    ////////////////////////////////////////////////////////////
    // Extra bytes for 52-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size >= 52) {
        // Red channel bitmask
        if (spbmp_file_read_cb(file, &dword, 4, 1, userdata) != 1) return kSpBmpMissingData;
        if (dword == 0xF800) header->rgb565_flag = 1;
        // IGNORED - Green channel bitmask
        if (spbmp_file_read_cb(file, &dword, 4, 1, userdata) != 1) return kSpBmpMissingData;
        // IGNORED - Blue channel bitmask
        if (spbmp_file_read_cb(file, &dword, 4, 1, userdata) != 1) return kSpBmpMissingData;
        counter += 12;
    }

    ////////////////////////////////////////////////////////////
    // Extra byte for 56-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size >= 56) {
        // IGNORED - Alpha channel bitmask
        if (spbmp_file_read_cb(file, &dword, 4, 1, userdata) != 1) return kSpBmpMissingData;
        counter += 4;
    }

    ////////////////////////////////////////////////////////////
    // Extra bytes for 108 & 124 byte DIB Headers
    ////////////////////////////////////////////////////////////

    while (counter < header->header_size + 14) {
        // IGNORED
        if (spbmp_file_read_cb(file, &dword, 1, 1, userdata) != 1) return kSpBmpMissingData;
        counter++;
    }

    ////////////////////////////////////////////////////////////
    // Colour table
    ////////////////////////////////////////////////////////////

    if (header->colour_table_size <= 256) {
        for (uint32_t i = 0; i < header->colour_table_size; i++) {
            if (spbmp_file_read_cb(file, &buf, 4, 1, userdata) != 1) return kSpBmpMissingData;
            header->colour_table[i][0] = buf[2];  // Red
            header->colour_table[i][1] = buf[1];  // Green
            header->colour_table[i][2] = buf[0];  // Blue
            // buf[3] is ignored.
            counter += 4;
        }
    } else {
        // printf("Unexpected colour table size: %d\n", header->colour_table_size);
        return kSpBmpError;
    }

    ////////////////////////////////////////////////////////////
    // Skip everything else until we reach the pixel data
    ////////////////////////////////////////////////////////////

    while (counter < (int)header->pixel_array_offset) {
        // IGNORED
        if (spbmp_file_read_cb(file, &dword, 1, 1, userdata) != 1) return kSpBmpMissingData;
        counter++;
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_1bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                   void *userdata) {
    if (header->colour_table_size == 0) return kSpBmpError;

    const int row_length = header->width / 8 + (header->width % 8 == 0 ? 0 : 1);
    uint8_t buf;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; ++x) {
            const int bit = x % 8;
            if (bit == 0) {
                if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
            }
            const int colourIndex = (buf & (0x80 >> bit)) ? 1 : 0;
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGB_TABLE_ENTRY(colourIndex), userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_4bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                   void *userdata) {
    if (header->colour_table_size == 0) return kSpBmpError;

    const int row_length = header->width / 2 + (header->width % 2 == 0 ? 0 : 1);
    uint8_t buf;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; ++x) {
            const int nibble = x % 2;
            if (nibble == 0) {
                if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
            }
            spbmp_set_pixel_cb(x + x_origin, y + y_origin,
                               RGB_TABLE_ENTRY(nibble == 0 ? buf >> 4 : buf & 0x0F), userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_4bpp_rle4(void *file, BmpHeader *header, int x_origin, int y_origin,
                                        void *userdata) {
    bool end_of_bmp = false;
    bool end_of_line = false;
    uint8_t buf[2];
    SpColourRgba colour[2];

    for (int y = header->height - 1; !end_of_bmp && y >= 0; --y) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        int x = 0;
        end_of_line = false;
        while (!end_of_line) {
            if (spbmp_file_read_cb(file, buf, 1, 2, userdata) != 2) return kSpBmpMissingData;
            if (buf[0] == 0) {
                switch (buf[1]) {
                    case 0:  // End of line
                        end_of_line = true;
                        break;
                    case 1:  // End of bitmap
                        end_of_bmp = true;
                        end_of_line = true;
                        break;
                    case 2: {  // Delta escape
                        if (spbmp_file_read_cb(file, buf, 1, 2, userdata) != 2) {
                            return kSpBmpMissingData;
                        }
                        const int x_end = x + buf[0];
                        const int y_end = y - buf[1];
                        while (x != x_end || y != y_end) {
                            spbmp_set_pixel_cb((x++) + x_origin, y + y_origin, 0, userdata);
                            if (x == header->width) {
                                x = 0;
                                y--;
                            }
                        }
                        break;
                    }
                    default: {  // Absolute mode
                        const int num_pixels = buf[1];
                        for (int i = 0; i < num_pixels; ++i) {
                            if (i % 2 == 0) {
                                if (spbmp_file_read_cb(file, buf, 1, 1, userdata) != 1) {
                                    return kSpBmpMissingData;
                                }
                                colour[0] = RGB_TABLE_ENTRY(buf[0] >> 4);
                                colour[1] = RGB_TABLE_ENTRY(buf[0] & 0xF);
                            }
                            spbmp_set_pixel_cb((x++) + x_origin, y + y_origin, colour[i % 2],
                                               userdata);
                            if (x == header->width) {
                                x = 0;
                                y--;
                            }
                        }

                        // Skip until we reach a 32-bit boundary.
                        const int num_bytes = (num_pixels / 2) + (num_pixels % 2 == 0 ? 0 : 1);
                        for (int i = num_bytes; i % 4; ++i) {
                            if (spbmp_file_read_cb(file, buf, 1, 1, userdata) != 1) {
                                return kSpBmpMissingData;
                            }
                        }
                        break;
                    }
                }
            } else {
                colour[0] = RGB_TABLE_ENTRY(buf[1] >> 4);
                colour[1] = RGB_TABLE_ENTRY(buf[1] & 0xF);
                for (int i = 0; i < buf[0]; ++i) {
                    if (x == header->width) return kSpBmpError;
                    spbmp_set_pixel_cb((x++) + x_origin, y + y_origin, colour[i % 2], userdata);
                }
            }
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_8bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                   void *userdata) {
    if (header->colour_table_size == 0) return kSpBmpError;

    const int row_length = header->width;
    uint8_t buf;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; ++x) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGB_TABLE_ENTRY(buf), userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }
    return kSpBmpOk;
}


static SpBmpResult spbmp_load_8bpp_greyscale(void *file, BmpHeader *header, int x_origin,
                                             int y_origin, void *userdata) {
    if (header->colour_table_size != 0) return kSpBmpError;

    const int row_length = header->width;
    uint8_t buf;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; ++x) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGB(buf, buf, buf, 0xFF), userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_8bpp_rle8(void *file, BmpHeader *header, int x_origin, int y_origin,
                                        void *userdata) {
    bool end_of_bmp = false;
    bool end_of_line = false;
    uint8_t buf[2];

    for (int y = header->height - 1; !end_of_bmp && y >= 0; --y) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        int x = 0;
        end_of_line = false;
        while (!end_of_line) {
            if (spbmp_file_read_cb(file, buf, 1, 2, userdata) != 2) return kSpBmpMissingData;
            if (buf[0] == 0) {
                switch (buf[1]) {
                    case 0:  // End of line
                        end_of_line = true;
                        break;
                    case 1:  // End of bitmap
                        end_of_bmp = true;
                        end_of_line = true;
                        break;
                    case 2: {  // Delta escape
                        if (spbmp_file_read_cb(file, buf, 1, 2, userdata) != 2) {
                            return kSpBmpMissingData;
                        }
                        const int x_end = x + buf[0];
                        const int y_end = y - buf[1];
                        while (x != x_end || y != y_end) {
                            spbmp_set_pixel_cb((x++) + x_origin, y + y_origin, 0, userdata);
                            if (x == header->width) {
                                x = 0;
                                y--;
                            }
                        }
                        break;
                    }
                    default: {  // Absolute mode
                        const int num_pixels = buf[1];
                        for (int i = 0; i < num_pixels; ++i) {
                            if (spbmp_file_read_cb(file, buf, 1, 1, userdata) != 1) {
                                return kSpBmpMissingData;
                            }
                            spbmp_set_pixel_cb((x++) + x_origin, y + y_origin,
                                               RGB_TABLE_ENTRY(buf[0]), userdata);
                            if (x == header->width) {
                                x = 0;
                                y--;
                            }
                        }

                        // Skip until we reach a 32-bit boundary.
                        for (int i = num_pixels; i % 4; ++i) {
                            if (spbmp_file_read_cb(file, buf, 1, 1, userdata) != 1) {
                                return kSpBmpMissingData;
                            }
                        }
                        break;
                    }
                }
            } else {
                for (int i = 0; i < buf[0]; ++i) {
                    if (x == header->width) return kSpBmpError;
                    spbmp_set_pixel_cb((x++) + x_origin, y + y_origin, RGB_TABLE_ENTRY(buf[1]),
                                       userdata);
                }
            }
        }
    }

    return kSpBmpOk;
}

#define RGB_565(x) RGB( \
    (x >> 11) << 3, \
    ((x & 0x07E0) >> 5) << 2, \
    (x & 0x001F) << 3, \
    0xFF)

#define RGB_555(x) RGB( \
    ((x & 0x7FFF) >> 10) << 3, \
    ((x & 0x03E0) >> 5) << 3, \
    (x & 0x001F) << 3, \
    0xFF)

static SpBmpResult spbmp_load_16bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                    void *userdata) {
    const int row_length = header->width * 2;
    uint32_t buf;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; x++) {
            if (spbmp_file_read_cb(file, &buf, 2, 1, userdata) != 1) return kSpBmpMissingData;
            spbmp_set_pixel_cb(x + x_origin, y + y_origin,
                               header->rgb565_flag == 1 ? RGB_565(buf) : RGB_555(buf), userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_24bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                    void *userdata) {
    uint8_t buf[3];
    const int row_length = header->width * 3;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; ++x) {
            if (spbmp_file_read_cb(file, buf, 1, 3, userdata) != 3) return kSpBmpMissingData;
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGB(buf[2], buf[1], buf[0], 0xFF),
                               userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_load_32bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                    void *userdata) {
    uint8_t buf[4];

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; ++x) {
            if (spbmp_file_read_cb(file, buf, 1, 4, userdata) != 4) return kSpBmpMissingData;
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGB(buf[2], buf[1], buf[0], buf[3]),
                               userdata);
        }
    }

    return kSpBmpOk;
}

SpBmpResult spbmp_load(void *file, int x_origin, int y_origin, void *userdata) {
    BmpHeader header;
    SpBmpResult result = spbmp_read_header(file, userdata, &header);
    if (result != kSpBmpOk) return result;

    switch (header.bits_per_pixel) {
        case 1:
            result = spbmp_load_1bpp(file, &header, x_origin, y_origin, userdata);
            break;
        case 4:
            if (header.compression_type == 2) {
                result = spbmp_load_4bpp_rle4(file, &header, x_origin, y_origin, userdata);
            } else {
                result = spbmp_load_4bpp(file, &header, x_origin, y_origin, userdata);
            }
            break;
        case 8:
            if (header.compression_type == 1) {
                result = spbmp_load_8bpp_rle8(file, &header, x_origin, y_origin, userdata);
            } else if (header.colour_table_size == 0) {
                result = spbmp_load_8bpp_greyscale(file, &header, x_origin, y_origin, userdata);
            } else {
                result = spbmp_load_8bpp(file, &header, x_origin, y_origin, userdata);
            }
            break;
        case 16:
            result = spbmp_load_16bpp(file, &header, x_origin, y_origin, userdata);
            break;
        case 24:
            result = spbmp_load_24bpp(file, &header, x_origin, y_origin, userdata);
            break;
        case 32:
            result = spbmp_load_32bpp(file, &header, x_origin, y_origin, userdata);
            break;
        default:
            result = -1;
            break;
    }

    return result;
}
