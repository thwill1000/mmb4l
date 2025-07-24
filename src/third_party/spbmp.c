// spBMP - a Microsoft Windows .bmp decoder.
// Copyright (c) 2024-2025 Thomas Hugo Williams
// License MIT <https://opensource.org/licenses/MIT>
//
// 09-Sep-2024: Version 1.0.2 - Corrected error value returned for unsupported bits per pixel.
// 08-Sep-2024: Version 1.0.1 - Simplified BmpHeader and made some cosmetic changes.
// 08-Sep-2024: Version 1.0.0 - Initial offering.

#include <string.h>

#include "spbmp_private.h"

#define RGB_TABLE_ENTRY(_idx) RGBA( \
    header->colour_table[_idx][0], \
    header->colour_table[_idx][1], \
    header->colour_table[_idx][2], \
    header->colour_table[_idx][3])

typedef enum {
    BI_RGB = 0,
    BI_RLE8 = 1,
    BI_RLE4 = 2,
    BI_BITFIELDS = 3,
} BmpCompression;

static SpBmpFileReadCb spbmp_file_read_cb;
static SpBmpFileWriteCb spbmp_file_write_cb;
static SpBmpGetPixelCb spbmp_get_pixel_cb;
static SpBmpSetPixelCb spbmp_set_pixel_cb;
static SpBmpAbortCheckCb spbmp_abort_check_cb;

void spbmp_init(SpBmpFileReadCb file_read_cb, SpBmpFileWriteCb file_write_cb,
                SpBmpGetPixelCb get_pixel_cb, SpBmpSetPixelCb set_pixel_cb,
                SpBmpAbortCheckCb abort_check_cb) {
    spbmp_file_read_cb = file_read_cb;
    spbmp_file_write_cb = file_write_cb;
    spbmp_get_pixel_cb = get_pixel_cb;
    spbmp_set_pixel_cb = set_pixel_cb;
    spbmp_abort_check_cb = abort_check_cb;
}

static inline SpBmpResult spbmp_read(void *file, void *buffer, size_t size, size_t count,
                                     void *userdata) {
    if (spbmp_file_read_cb(file, buffer, size, count, userdata) == count) {
        return kSpBmpOk;
    } else {
        return kSpBmpMissingData;
    }
}

static inline SpBmpResult spbmp_write(void *file, const void *buffer, size_t size, size_t count,
                                      void *userdata) {
    if (spbmp_file_write_cb(file, buffer, size, count, userdata) == count) {
        return kSpBmpOk;
    } else {
        return kSpBmpMissingData;
    }
}

#define ON_FAILURE_RETURN(x)  { \
  const SpBmpResult rezult = x; \
  if (rezult) { return rezult; } \
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

    size_t counter = 0;
    uint32_t dword;
    uint8_t buf[4];

    ////////////////////////////////////////////////////////////
    // 14-byte Bitmap File Header
    ////////////////////////////////////////////////////////////

    ON_FAILURE_RETURN(spbmp_read(file, buf, 2, 1, userdata));  // Signature
    if (buf[0] != 'B' || buf[1] != 'M') {
        return kSpBmpError;
    }

    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // File size
    header->file_size = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 2, 1, userdata));  // IGNORED - Reserved value 1
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 2, 1, userdata));  // IGNORED - Reserved value 2
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Pixel array offset
    header->pixel_array_offset = dword;

    counter += 14;

    ////////////////////////////////////////////////////////////
    // 40-byte DIB Header
    ////////////////////////////////////////////////////////////

    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Header size
    header->header_size = (uint8_t)dword;
    if (header->header_size < 40) return kSpBmpError;

    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Image width
    header->width = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Image height
    header->height = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 2, 1, userdata));  // Number of planes
    header->num_planes = (uint16_t) dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 2, 1, userdata));  // Bits per pixel
    header->bits_per_pixel = (uint16_t)dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // File compression type
    header->compression_type = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Image size
    header->image_size = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // X pixels per metre
    header->x_pixels_per_metre = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Y pixels per metre
    header->y_pixels_per_metre = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Size of the colour table
    header->colour_table_size = dword;
    ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));  // Important colour count
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
    if (header->compression_type != BI_RGB
            && header->compression_type != BI_RLE8
            && header->compression_type != BI_RLE4
            && header->compression_type != BI_BITFIELDS) return kSpBmpError;
    if (header->colour_table_size == 0 && header->bits_per_pixel == 8) return kSpBmpError;
    if (header->important_colour_count > header->colour_table_size) return kSpBmpError;

    counter += 40;

    ////////////////////////////////////////////////////////////
    // Extra bytes for 52-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size >= 52) {
        // Red channel bitmask
        ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));
        if (dword == 0xF800) header->rgb565_flag = 1;
        // IGNORED - Green channel bitmask
        ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));
        // IGNORED - Blue channel bitmask
        ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));
        counter += 12;
    }

    ////////////////////////////////////////////////////////////
    // Extra byte for 56-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size >= 56) {
        // IGNORED - Alpha channel bitmask
        ON_FAILURE_RETURN(spbmp_read(file, &dword, 4, 1, userdata));
        counter += 4;
    }

    ////////////////////////////////////////////////////////////
    // Extra bytes for 108 & 124 byte DIB Headers
    ////////////////////////////////////////////////////////////

    while (counter < header->header_size + 14) {
        // IGNORED
        ON_FAILURE_RETURN(spbmp_read(file, &dword, 1, 1, userdata));
        counter++;
    }

    ////////////////////////////////////////////////////////////
    // Colour table
    ////////////////////////////////////////////////////////////

    if (header->colour_table_size <= 256) {
        for (uint32_t i = 0; i < header->colour_table_size; i++) {
            ON_FAILURE_RETURN(spbmp_read(file, &buf, 4, 1, userdata));
            header->colour_table[i][0] = buf[2];  // Red
            header->colour_table[i][1] = buf[1];  // Green
            header->colour_table[i][2] = buf[0];  // Blue
            header->colour_table[i][3] = 0xFF;    // Alpha
            // buf[3] is ignored - is this the Alpha value ?
            counter += 4;
        }
    } else {
        // printf("Unexpected colour table size: %d\n", header->colour_table_size);
        return kSpBmpError;
    }

    ////////////////////////////////////////////////////////////
    // Skip everything else until we reach the pixel data
    ////////////////////////////////////////////////////////////

    while (counter < (size_t) header->pixel_array_offset) {
        // IGNORED
        ON_FAILURE_RETURN(spbmp_read(file, &dword, 1, 1, userdata));
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
            ON_FAILURE_RETURN(spbmp_read(file, buf, 1, 2, userdata));
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
                        ON_FAILURE_RETURN(spbmp_read(file, buf, 1, 2, userdata));
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
                                ON_FAILURE_RETURN(spbmp_read(file, buf, 1, 1, userdata));
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

                        // Skip until we reach a 16-bit boundary.
                        const int num_bytes = (num_pixels + 1) / 2;
                        for (int i = num_bytes; i % 2; ++i) {
                            ON_FAILURE_RETURN(spbmp_read(file, buf, 1, 1, userdata));
                        }
                        break;
                    }
                }
            } else {
                colour[0] = RGB_TABLE_ENTRY(buf[1] >> 4);
                colour[1] = RGB_TABLE_ENTRY(buf[1] & 0xF);
                for (int i = 0; i < buf[0]; ++i) {
                    if (x == header->width) {
                        return kSpBmpError;
                    }
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
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGBA(buf, buf, buf, 0xFF), userdata);
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

#define RGB_565(x) RGBA( \
    (x >> 11) << 3, \
    ((x & 0x07E0) >> 5) << 2, \
    (x & 0x001F) << 3, \
    0xFF)

#define RGB_555(x) RGBA( \
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
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGBA(buf[2], buf[1], buf[0], 0xFF),
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
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, RGBA(buf[2], buf[1], buf[0], buf[3]),
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
            if (header.compression_type == BI_RLE4) {
                result = spbmp_load_4bpp_rle4(file, &header, x_origin, y_origin, userdata);
            } else {
                result = spbmp_load_4bpp(file, &header, x_origin, y_origin, userdata);
            }
            break;
        case 8:
            if (header.compression_type == BI_RLE8) {
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
            result = kSpBmpError;
            break;
    }

    return result;
}

/**
 * Writes the bitmap header.
 *
 * @param  file      Opaque pointer to destimation file.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  header    Header to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_header(void *file, void *userdata, BmpHeader *header) {
    uint32_t dword = 0x0;

    ////////////////////////////////////////////////////////////
    // 14-byte Bitmap File Header
    ////////////////////////////////////////////////////////////

    ON_FAILURE_RETURN(spbmp_write(file, "BM", 2, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->file_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &dword, 2, 1, userdata)); // Reserved value 1.
    ON_FAILURE_RETURN(spbmp_write(file, &dword, 2, 1, userdata)); // Reserved value 2.
    ON_FAILURE_RETURN(spbmp_write(file, &header->pixel_array_offset, 4, 1, userdata));

    ////////////////////////////////////////////////////////////
    // 40-byte DIB Header
    ////////////////////////////////////////////////////////////

    ON_FAILURE_RETURN(spbmp_write(file, &header->header_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->width, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->height, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->num_planes, 2, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->bits_per_pixel, 2, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->compression_type, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->image_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->x_pixels_per_metre, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->y_pixels_per_metre, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->colour_table_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_write(file, &header->important_colour_count, 4, 1, userdata));

    ////////////////////////////////////////////////////////////
    // Colour table
    ////////////////////////////////////////////////////////////

    for (uint32_t i = 0; i < header->colour_table_size; ++i) {
        dword = header->colour_table[i][2];        // Blue
        dword += header->colour_table[i][1] << 8;  // Green
        dword += header->colour_table[i][0] << 16; // Red
        dword += header->colour_table[i][3] << 24; // Alpha
        ON_FAILURE_RETURN(spbmp_write(file, &dword, 4, 1, userdata));
    }

    return kSpBmpOk;
}

/**
 * Writes pixel data in 24 bpp format.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_24bpp(void *file, void *userdata, int x_origin, int y_origin,
                                     int width, int height) {
    const char padding[4] = { 0 };

    for (int y = height - 1; y >= 0; --y) {
        int x = 0;
        SpColourRgba pixel = 0x0;
        for (x = 0; x != width; ++x) {
            pixel = spbmp_get_pixel_cb(x + x_origin, y + y_origin, userdata) & 0xFFFFFF;
            ON_FAILURE_RETURN(spbmp_write(file, &pixel, 3, 1, userdata));
        }
        pixel = 0x0;
        if ((width * 3) % 4 != 0) {
            // Pad each row to multiple of 32-bits.
            ON_FAILURE_RETURN(spbmp_write(file, &padding, 4 - ((width * 3) % 4), 1,
                                          userdata));
        }
    }

    return kSpBmpOk;
}

static inline uint8_t spbmp_get_pixel_rgb121(int x, int y, void *userdata) {
    const SpColourRgba pixel = spbmp_get_pixel_cb(x, y, userdata);
//    printf("x = %d, y = %d, pixel: 0x%08x\n", x, y, pixel);
    return ((pixel & 0x800000) >> 20) | ((pixel & 0xC000) >> 13) | ((pixel & 0x80) >> 7);
}

#define GET_RGB_121(x, y)  spbmp_get_pixel_rgb121((x) + x_origin, (y) + y_origin, userdata)

static SpBmpResult spbmp_write_rgb121(void *file, void *userdata, int x_origin, int y_origin,
                                      int width, int height) {

    uint8_t buf;
    const int y_start = height - 1;
    const int y_end = -1;
    const int y_delta = -1;
    const int bytes_per_row = (width + 1) / 2;  // 2 pixels per byte, round up
    const int padding = (4 - (bytes_per_row % 4)) % 4;  // Pad to 32-bit boundary

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < width; x += 2) {
            const uint8_t pixel1 = GET_RGB_121(x, y);
            const uint8_t pixel2 = (x + 1 < width) ? GET_RGB_121(x + 1, y) : 0x0;
            buf = (pixel1 << 4) | (pixel2 & 0x0F);
            ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
        }

        // Pad each row to 32-bit boundary.
        buf = 0x0;
        for (int x = 0; x < padding; ++x) {
           ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
        }
    }

    return kSpBmpOk;
}

static inline SpBmpResult spbmp_write_rle4_encoded(void *file, void *userdata, int run_length,
                                                   uint8_t pixel) {
    // printf("Encoded mode: len = %d, colour = %d:\n", run_length, pixel);
    // printf("  0x%02x ", run_length);
    ON_FAILURE_RETURN(spbmp_write(file, &run_length, 1, 1, userdata));
    pixel |= pixel << 4;
    // printf("0x%02x\n", pixel);
    ON_FAILURE_RETURN(spbmp_write(file, &pixel, 1, 1, userdata));
    return kSpBmpOk;
}

static inline SpBmpResult spbmp_write_rle4_absolute(void *file, void *userdata, int count,
                                                    const uint8_t *pixels) {
    // printf("Absolute mode: count = %d, colours = ", count);
    // for (int i = 0; i < count; i++) {
    //     printf("%02x ", pixels[i]);
    // }

    uint8_t buf = 0x0;
    ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
    buf = (uint8_t)count;
    ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
    // printf("\n  0x00 0x%02x ", count);

    // Pack two 4-bit pixels per byte
    for (int i = 0; i < count; i += 2) {
        uint8_t first = pixels[i];
        uint8_t second = (i + 1 < count) ? pixels[i + 1] : 0;
        buf = (first << 4) | second;
        ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
        // printf("0x%02x ", buf);
    }

    // Pad to a 16-bit word boundary.
    int pixel_data_bytes = (count + 1) / 2;
    if (pixel_data_bytes % 2 != 0) {
        // printf("0x00 ");
        buf = 0x0;
        ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
    }

    // printf("\n");
    return kSpBmpOk;
}

static SpBmpResult spbmp_write_rgb121_rle4(void *file, void *userdata, int x_origin, int y_origin,
                                           int width, int height) {
    uint8_t buf;
    const int y_start = height - 1;
    const int y_end = -1;
    const int y_delta = -1;

    for (int y = y_start; y != y_end; y += y_delta) {
        // printf("NEW LINE: y = %d\n", y);
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        int x = 0;
        while (x < width) {
            // Check for run of repeated pixels.
            uint8_t pixel = GET_RGB_121(x, y);
            int run_length = 1;
            x++;
            while (x < width && run_length < 255) {
                if (GET_RGB_121(x, y) == pixel) {
                    run_length++;
                    x++;
                } else {
                    break;
                }
            }

            if (run_length > 1 || x == width) {
                ON_FAILURE_RETURN(spbmp_write_rle4_encoded(file, userdata, run_length, pixel));
                continue;
            }

            int count = 0;
            uint8_t pixels[255];
            pixels[count++] = pixel;

            while (x < width && count < 255) {
                pixels[count++] = GET_RGB_121(x, y);
                x++;
                // Check for the start of a new run.
                if (x < width) {
                    if (GET_RGB_121(x, y) == pixels[count - 1]) {
                        count--;
                        x--;
                        break;
                    }
                }
            }

            // We can only use absolute mode where count > 2.
            switch (count) {
                case 1:
                    ON_FAILURE_RETURN(spbmp_write_rle4_encoded(file, userdata, 1, pixels[0]));
                    break;
                case 2:
                    // TODO: Two pixels can be represented by one encoded run.
                    ON_FAILURE_RETURN(spbmp_write_rle4_encoded(file, userdata, 1, pixels[0]));
                    ON_FAILURE_RETURN(spbmp_write_rle4_encoded(file, userdata, 1, pixels[1]));
                    break;
                default:
                    ON_FAILURE_RETURN(spbmp_write_rle4_absolute(file, userdata, count, pixels));
                    break;
            }
        }

        // End of line marker.
        buf = 0x0;
        ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
        ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
    }

    // End of bitmap marker.
    buf = 0x0;
    ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));
    buf = 0x1;
    ON_FAILURE_RETURN(spbmp_write(file, &buf, 1, 1, userdata));

    return kSpBmpOk;
}

static inline uint8_t spbmp_get_pixel_rgb222(int x, int y, void *userdata) {
    const SpColourRgba pixel = spbmp_get_pixel_cb(x, y, userdata);
//    printf("x = %d, y = %d, pixel: 0x%08x\n", x, y, pixel);
    return ((pixel & 0xC00000) >> 18) | ((pixel & 0xC000) >> 12) | ((pixel & 0xC0) >> 6);
}

#define GET_RGB_222(x, y)  spbmp_get_pixel_rgb222((x) + x_origin, (y) + y_origin, userdata)

static SpBmpResult spbmp_write_rgb222(void *file, void *userdata, int x_origin, int y_origin,
                                      int width, int height) {
    const int y_start = height - 1;
    const int y_end = -1;
    const int y_delta = -1;
    const int padding = (4 - (width % 4)) % 4;  // Pad to 32-bit boundary

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;
        for (int x = 0; x < width; x++) {
            const uint8_t pixel = GET_RGB_222(x, y);
            ON_FAILURE_RETURN(spbmp_write(file, &pixel, 1, 1, userdata));
        }

        // Pad each row to 32-bit boundary.
        for (int x = 0; x < padding; ++x) {
            const uint8_t pixel = 0x0;
            ON_FAILURE_RETURN(spbmp_write(file, &pixel, 1, 1, userdata));
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_write_rgb332(void *file, void *userdata, int x_origin, int y_origin,
                                      int width, int height) {
    return kSpBmpError;
}

static SpBmpResult spbmp_write_rgb222_rle8(void *file, void *userdata, int x_origin, int y_origin,
                                           int width, int height) {
    return kSpBmpError;
}

static SpBmpResult spbmp_write_rgb332_rle8(void *file, void *userdata, int x_origin, int y_origin,
                                           int width, int height) {
    return kSpBmpError;
}

SpBmpResult spbmp_save(void *file, SpBmpFormat format, void *userdata, int x_origin, int y_origin,
                       int width, int height) {
    BmpHeader header = {
        .width = width,
        .height = height,
        .header_size = 40,
        .num_planes = 1,
        .compression_type = BI_RGB,
        .x_pixels_per_metre = DEFAULT_PIXELS_PER_METRE,
        .y_pixels_per_metre = DEFAULT_PIXELS_PER_METRE
    };

    switch (format) {
        case kSpBmp24bpp:
            header.image_size = 3 * width * height;
            header.pixel_array_offset = 54;
            header.file_size = header.pixel_array_offset + header.image_size;
            header.bits_per_pixel = 24;
            break;

        case kSpBmpRgb121:
        case kSpBmpCompressedRgb121:
            header.image_size = width * height / 2;
            header.pixel_array_offset = 54 + 16 * 4;
            header.file_size = header.pixel_array_offset + header.image_size;
            header.bits_per_pixel = 4;
            header.compression_type = (format == kSpBmpCompressedRgb121) ? BI_RLE4 : BI_RGB;
            header.colour_table_size = 16;
            header.important_colour_count = 16;
            memcpy(header.colour_table, rgb121_colour_table, sizeof(rgb121_colour_table));
            break;

        case kSpBmpRgb222:
            header.image_size = width * height;
            header.pixel_array_offset = 54 + 64 * 4;
            header.file_size = header.pixel_array_offset + header.image_size;
            header.bits_per_pixel = 8;
            header.compression_type = BI_RGB;
            header.colour_table_size = 64;
            header.important_colour_count = 64;
            memcpy(header.colour_table, rgb222_colour_table, sizeof(rgb222_colour_table));
            break;

        default:
            return kSpBmpUnknownFormat;
    }

    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    switch (format) {
        case kSpBmp24bpp:
            return spbmp_write_24bpp(file, userdata, x_origin, y_origin, width, height);

        case kSpBmpRgb121:
            return spbmp_write_rgb121(file, userdata, x_origin, y_origin, width, height);

        case kSpBmpRgb222:
            return spbmp_write_rgb222(file, userdata, x_origin, y_origin, width, height);

        case kSpBmpRgb332:
            return spbmp_write_rgb332(file, userdata, x_origin, y_origin, width, height);

        case kSpBmpCompressedRgb121:
            return spbmp_write_rgb121_rle4(file, userdata, x_origin, y_origin, width, height);

        case kSpBmpCompressedRgb222:
            return spbmp_write_rgb222_rle8(file, userdata, x_origin, y_origin, width, height);

        case kSpBmpCompressedRgb332:
            return spbmp_write_rgb332_rle8(file, userdata, x_origin, y_origin, width, height);

        default:
            return kSpBmpUnknownFormat;
    }
}
