// spBMP - a Microsoft Windows .bmp decoder.
// Copyright (c) 2024-2026 Thomas Hugo Williams
// License MIT <https://opensource.org/licenses/MIT>
//
// 07-Jan-2026: Version 1.1.0 - Added spbmp_write() to save .bmp files.
//                              Renamed sbmp_load() to spbmp_read().
//                              Assorted other renaming/refactoring.
// 09-Sep-2024: Version 1.0.2 - Corrected error value returned for unsupported bits per pixel.
// 08-Sep-2024: Version 1.0.1 - Simplified BmpHeader and made some cosmetic changes.
// 08-Sep-2024: Version 1.0.0 - Initial offering.

#include <assert.h>
#include <string.h>

#include "spbmp_private.h"

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

static inline SpBmpResult spbmp_fread(void *file, void *buffer, size_t size, size_t count,
                                      void *userdata) {
    if (spbmp_file_read_cb(file, buffer, size, count, userdata) == count) {
        return kSpBmpOk;
    } else {
        return kSpBmpMissingData;
    }
}

static inline SpBmpResult spbmp_fwrite(void *file, const void *buffer, size_t size, size_t count,
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

    ON_FAILURE_RETURN(spbmp_fread(file, buf, 2, 1, userdata));  // Signature
    if (buf[0] != 'B' || buf[1] != 'M') {
        return kSpBmpError;
    }

    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // File size
    header->file_size = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 2, 1, userdata));  // IGNORED - Reserved value 1
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 2, 1, userdata));  // IGNORED - Reserved value 2
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Pixel array offset
    header->pixel_array_offset = dword;

    counter += BMP_FILE_HEADER_SIZE;

    ////////////////////////////////////////////////////////////
    // 40-byte DIB Header
    ////////////////////////////////////////////////////////////

    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Header size
    header->header_size = (uint8_t)dword;
    if (header->header_size < BMP_DIB_HEADER_SIZE) return kSpBmpError;

    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Image width
    header->width = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Image height
    header->height = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 2, 1, userdata));  // Number of planes
    header->num_planes = (uint16_t) dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 2, 1, userdata));  // Bits per pixel
    header->bits_per_pixel = (uint16_t)dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // File compression type
    header->compression_type = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Image size
    header->image_size = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // X pixels per metre
    header->x_pixels_per_metre = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Y pixels per metre
    header->y_pixels_per_metre = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Size of the colour table
    header->colour_table_size = dword;
    ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));  // Important colour count
    header->important_colour_count = dword;

    // Heuristic to workaround "size zero" colour-table.
    // In the BMP format, the biClrUsed field (at offset 46) is often set to 0
    // to indicate that the table contains the maximum number of colors for that
    // bit depth (e.g., 256 for 8-bpp). However, some versions of MS Paint (and
    // other legacy tools) occasionally write malformed headers where they set
    // biClrUsed to 0 but don't actually provide the full 256 entries, or they
    // provide a custom number of entries without updating the header field.
    if (header->colour_table_size == 0 && header->bits_per_pixel <= 8) {
        uint32_t header_size = header->header_size;
        uint32_t metadata_size = BMP_FILE_HEADER_SIZE + header_size;
        if (header->pixel_array_offset > metadata_size) {
            header->colour_table_size = (header->pixel_array_offset - metadata_size) / 4;
        }
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
    if (header->colour_table_size == 0 && header->bits_per_pixel <= 8) return kSpBmpError;
    if (header->important_colour_count > header->colour_table_size) return kSpBmpError;

    counter += BMP_DIB_HEADER_SIZE;

    ////////////////////////////////////////////////////////////
    // Extra bytes for 52-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size >= 52) {
        // Red channel bitmask
        ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));
        if (dword == 0xF800) header->rgb565_flag = 1;
        // IGNORED - Green channel bitmask
        ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));
        // IGNORED - Blue channel bitmask
        ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));
        counter += 12;
    }

    ////////////////////////////////////////////////////////////
    // Extra byte for 56-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size >= 56) {
        // IGNORED - Alpha channel bitmask
        ON_FAILURE_RETURN(spbmp_fread(file, &dword, 4, 1, userdata));
        counter += 4;
    }

    ////////////////////////////////////////////////////////////
    // Extra bytes for 108 & 124 byte DIB Headers
    ////////////////////////////////////////////////////////////

    while (counter < header->header_size + BMP_FILE_HEADER_SIZE) {
        // IGNORED
        ON_FAILURE_RETURN(spbmp_fread(file, &dword, 1, 1, userdata));
        counter++;
    }

    ////////////////////////////////////////////////////////////
    // Colour table
    ////////////////////////////////////////////////////////////

    if (header->colour_table_size <= 256) {
        for (uint32_t i = 0; i < header->colour_table_size; i++) {
            ON_FAILURE_RETURN(spbmp_fread(file, &buf, 4, 1, userdata));
            header->colour_table[i][0] = buf[2];  // Red
            header->colour_table[i][1] = buf[1];  // Green
            header->colour_table[i][2] = buf[0];  // Blue
            header->colour_table[i][3] = 0xFF;    // Alpha
            // buf[3] is ignored - is this the Alpha value ?
            counter += 4;
        }
    } else {
        return kSpBmpError;
    }

    if (header->bits_per_pixel <= 8) {
        uint32_t max_colors = 1u << header->bits_per_pixel;
        if (header->colour_table_size > max_colors) {
            return kSpBmpError;  // Invalid: too many colors
        }
        if (header->colour_table_size == 0) {
            return kSpBmpError;  // Already checked earlier, but be explicit
        }
        // For well-formed files, all indices will be < colour_table_size
        // because they're encoded in the bit depth
    }

    ////////////////////////////////////////////////////////////
    // Skip everything else until we reach the pixel data
    ////////////////////////////////////////////////////////////

    while (counter < (size_t) header->pixel_array_offset) {
        // IGNORED
        ON_FAILURE_RETURN(spbmp_fread(file, &dword, 1, 1, userdata));
        counter++;
    }

    return kSpBmpOk;
}

/**
 * Safely retrieves a color from the color table with bounds protection.
 *
 * Clamps out-of-bounds color indices to the first color table entry to prevent
 * buffer overruns when reading malformed BMP files. Well-formed files never trigger
 * this check since pixel indices are constrained by the bit depth.
 *
 * @param  header  Pointer to BMP header containing the color table
 * @param  idx     Color table index from pixel data
 * @return         32-bit ARGB color value (index 0 used as fallback for invalid indices)
 */
static inline SpColourArgb spbmp_get_colour_table_entry(
    const BmpHeader *header, uint8_t idx) {
    uint8_t safe_idx = idx & 0xFF;  // No-op for uint8_t, but documents the intent
    if (safe_idx >= header->colour_table_size) {
        safe_idx = 0;  // Clamp to first color as fallback
    }
    return RGBA(
        header->colour_table[safe_idx][0],
        header->colour_table[safe_idx][1],
        header->colour_table[safe_idx][2],
        header->colour_table[safe_idx][3]
    );
}

static SpBmpResult spbmp_read_1bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                   void *userdata) {
    if (header->colour_table_size == 0) return kSpBmpError;

    const int row_length = header->width / 8 + (header->width % 8 == 0 ? 0 : 1);
    uint8_t buf = 0x0;

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

            const int colour_index = (buf & (0x80 >> bit)) ? 1 : 0;
            const SpColourArgb colour = spbmp_get_colour_table_entry(header, colour_index);
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour, userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_read_4bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                   void *userdata) {
    if (header->colour_table_size == 0) return kSpBmpError;

    const int row_length = header->width / 2 + (header->width % 2 == 0 ? 0 : 1);
    uint8_t buf = 0x0;

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

            const int colour_index = (nibble == 0) ? (buf >> 4) : (buf & 0x0F);
            const SpColourArgb colour = spbmp_get_colour_table_entry(header, colour_index);
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour, userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_read_4bpp_rle4(void *file, BmpHeader *header, int x_origin, int y_origin,
                                        void *userdata) {
    bool end_of_bmp = false;
    bool end_of_line = false;
    uint8_t buf[2];
    SpColourArgb colour[2];

    for (int y = header->height - 1; !end_of_bmp && y >= 0; --y) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        int x = 0;
        end_of_line = false;
        while (!end_of_line) {
            ON_FAILURE_RETURN(spbmp_fread(file, buf, 1, 2, userdata));
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
                        ON_FAILURE_RETURN(spbmp_fread(file, buf, 1, 2, userdata));
                        x += buf[0];
                        y -= buf[1];
                        break;
                    }
                    default: {  // Absolute mode
                        const int num_pixels = buf[1];
                        int bytes_read = 0;

                        for (int i = 0; i < num_pixels; ++i) {
                            if (i % 2 == 0) {
                                ON_FAILURE_RETURN(spbmp_fread(file, buf, 1, 1, userdata));
                                bytes_read++;
                                colour[0] = spbmp_get_colour_table_entry(header, buf[0] >> 4);
                                colour[1] = spbmp_get_colour_table_entry(header, buf[0] & 0xF);
                            }

                            if (x >= header->width || y < 0) return kSpBmpError;
                            spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour[i % 2], userdata);
                            x++;
                        }

                        // Align the file pointer to a 16-bit (2-byte) boundary
                        if (bytes_read % 2 != 0) {
                            ON_FAILURE_RETURN(spbmp_fread(file, buf, 1, 1, userdata));
                        }
                        break;
                    }
                }
            } else {
                // Encoded mode
                colour[0] = spbmp_get_colour_table_entry(header, buf[1] >> 4);
                colour[1] = spbmp_get_colour_table_entry(header, buf[1] & 0xF);
                for (int i = 0; i < buf[0]; ++i) {
                    if (x >= header->width || y < 0) return kSpBmpError;
                    spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour[i % 2], userdata);
                    x++;
                }
            }
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_read_8bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
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
            const SpColourArgb colour = spbmp_get_colour_table_entry(header, buf);
            spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour, userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) return kSpBmpMissingData;
        }
    }
    return kSpBmpOk;
}


static SpBmpResult spbmp_read_8bpp_greyscale(void *file, BmpHeader *header, int x_origin,
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

static SpBmpResult spbmp_read_8bpp_rle8(void *file, BmpHeader *header, int x_origin, int y_origin,
                                        void *userdata) {
    bool end_of_bmp = false;
    uint8_t buf[2];
    int x = 0;
    int y = header->height - 1; // Start at the bottom

    while (!end_of_bmp && y >= 0) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        if (spbmp_file_read_cb(file, buf, 1, 2, userdata) != 2) return kSpBmpMissingData;

        if (buf[0] == 0) {
            switch (buf[1]) {
                case 0:  // End of line
                    x = 0;
                    y--;
                    break;
                case 1:  // End of bitmap
                    end_of_bmp = true;
                    break;
                case 2:  // Delta escape
                    if (spbmp_file_read_cb(file, buf, 1, 2, userdata) != 2) return kSpBmpMissingData;
                    x += buf[0];
                    y -= buf[1];
                    break;
                default: {  // Absolute mode
                    const int num_pixels = buf[1];
                    for (int i = 0; i < num_pixels; ++i) {
                        uint8_t idx;
                        if (spbmp_file_read_cb(file, &idx, 1, 1, userdata) != 1) return kSpBmpMissingData;

                        // Bounds check - standard RLE shouldn't wrap, but we check to be safe
                        if (x >= header->width || y < 0) return kSpBmpError;
                        const SpColourArgb colour = spbmp_get_colour_table_entry(header, idx);
                        spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour, userdata);
                        x++;
                    }
                    if (num_pixels % 2 != 0) {
                        spbmp_file_read_cb(file, buf, 1, 1, userdata); // Padding
                    }
                    break;
                }
            }
        } else {
            // Encoded mode: buf[0] is run length, buf[1] is color index
            for (int i = 0; i < buf[0]; ++i) {
                if (x >= header->width || y < 0) return kSpBmpError;
                const SpColourArgb colour = spbmp_get_colour_table_entry(header, buf[1]);
                spbmp_set_pixel_cb(x + x_origin, y + y_origin, colour, userdata);
                x++;
            }
        }
    }
    return kSpBmpOk;
}

#define DECODE_RGB_565(x) RGBA( \
    (x >> 11) << 3, \
    ((x & 0x07E0) >> 5) << 2, \
    (x & 0x001F) << 3, \
    0xFF)

#define DECODE_RGB_555(x) RGBA( \
    ((x & 0x7FFF) >> 10) << 3, \
    ((x & 0x03E0) >> 5) << 3, \
    (x & 0x001F) << 3, \
    0xFF)

static SpBmpResult spbmp_read_16bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
                                    void *userdata) {
    const int row_length = header->width * 2;
    uint32_t buf;

    const int y_start = (header->height >= 0) ? header->height - 1 : 0;
    const int y_end = (header->height >= 0) ? -1 : -header->height;
    const int y_delta = (header->height >= 0) ? -1 : +1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        for (int x = 0; x < header->width; x++) {
            if (spbmp_file_read_cb(file, &buf, 2, 1, userdata) != 1) {
                return kSpBmpMissingData;
            }
            spbmp_set_pixel_cb(x + x_origin, y + y_origin,
                               header->rgb565_flag == 1 ? DECODE_RGB_565(buf) : DECODE_RGB_555(buf),
                               userdata);
        }

        // Discard padding bytes to next 32-bit boundary.
        for (int i = row_length; i % 4; ++i) {
            if (spbmp_file_read_cb(file, &buf, 1, 1, userdata) != 1) {
                return kSpBmpMissingData;
            }
        }
    }

    return kSpBmpOk;
}

static SpBmpResult spbmp_read_24bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
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

static SpBmpResult spbmp_read_32bpp(void *file, BmpHeader *header, int x_origin, int y_origin,
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

SpBmpResult spbmp_read(void *file, int x_origin, int y_origin, void *userdata) {
    BmpHeader header;
    SpBmpResult result = spbmp_read_header(file, userdata, &header);
    if (result != kSpBmpOk) return result;

    switch (header.bits_per_pixel) {
        case 1:
            result = spbmp_read_1bpp(file, &header, x_origin, y_origin, userdata);
            break;
        case 4:
            if (header.compression_type == BI_RLE4) {
                result = spbmp_read_4bpp_rle4(file, &header, x_origin, y_origin, userdata);
            } else {
                result = spbmp_read_4bpp(file, &header, x_origin, y_origin, userdata);
            }
            break;
        case 8:
            if (header.compression_type == BI_RLE8) {
                result = spbmp_read_8bpp_rle8(file, &header, x_origin, y_origin, userdata);
            } else if (header.colour_table_size == 0) {
                result = spbmp_read_8bpp_greyscale(file, &header, x_origin, y_origin, userdata);
            } else {
                result = spbmp_read_8bpp(file, &header, x_origin, y_origin, userdata);
            }
            break;
        case 16:
            result = spbmp_read_16bpp(file, &header, x_origin, y_origin, userdata);
            break;
        case 24:
            result = spbmp_read_24bpp(file, &header, x_origin, y_origin, userdata);
            break;
        case 32:
            result = spbmp_read_32bpp(file, &header, x_origin, y_origin, userdata);
            break;
        default:
            result = kSpBmpError;
            break;
    }

    return result;
}

static void generate_rgb332_table(uint8_t table[256][4]) {
    for (int i = 0; i < 256; i++) {
        // Red: bits 7,6,5. Scale 3-bit to 8-bit.
        uint8_t r = (i >> 5) & 0x07;
        table[i][0] = (r << 5) | (r << 2) | (r >> 1);

        // Green: bits 4,3,2. Scale 3-bit to 8-bit.
        uint8_t g = (i >> 2) & 0x07;
        table[i][1] = (g << 5) | (g << 2) | (g >> 1);

        // Blue: bits 1,0. Scale 2-bit to 8-bit.
        uint8_t b = i & 0x03;
        table[i][2] = (b << 6) | (b << 4) | (b << 2) | b;

        // Alpha: Fixed at 0xFF
        table[i][3] = 0xFF;
    }
}

static SpBmpResult spbmp_init_header(SpBmpFormat format, int width, int height, BmpHeader *header) {
    if (width <= 0 || height <= 0) return kSpBmpError;

    memset(header, 0, sizeof(BmpHeader));
    header->width = width;
    header->height = height;
    header->header_size = BMP_DIB_HEADER_SIZE;
    header->num_planes = 1;
    header->compression_type = BI_RGB;
    header->x_pixels_per_metre = DEFAULT_PIXELS_PER_METRE;
    header->y_pixels_per_metre = DEFAULT_PIXELS_PER_METRE;

    switch (format) {
        case kSpBmp1bpp: {
            // 1. Calculate the padded row size in 64-bit to prevent overflow
            // (width + 31) / 32 gives number of 4-byte chunks
            const int64_t row_size = ((int64_t) width + 31) / 32 * 4;
            const int64_t image_size = row_size * (int64_t) height;

            // 2. Verify it fits in the 32-bit header field
            if (image_size > UINT32_MAX) return kSpBmpError;

            header->image_size = (uint32_t) image_size;
            header->pixel_array_offset = BMP_BASIC_HEADER_SIZE + (2 * 4); // DIB header + 2 palette entries

            // 3. Check for file_size overflow
            if ((uint64_t)header->pixel_array_offset + header->image_size > UINT32_MAX) {
                return kSpBmpError;
            }

            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 1;
            header->colour_table_size = 2;
            header->important_colour_count = 2;
            memcpy(header->colour_table, monochrome_1bpp_colour_table,
                   sizeof(monochrome_1bpp_colour_table));
            break;
        }

        case kSpBmp4bppRgb121:
        case kSpBmp4bppRgb121Rle4: {
            const int64_t image_size = (int64_t) width * (int64_t) height / 2 + 1; // +1 to round up for odd pixels
            if (image_size > UINT32_MAX) return kSpBmpError;
            header->image_size = (uint32_t) image_size;
            header->pixel_array_offset = BMP_BASIC_HEADER_SIZE + 16 * 4;
            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 4;
            header->compression_type = (format == kSpBmp4bppRgb121) ? BI_RGB : BI_RLE4;
            header->colour_table_size = 16;
            header->important_colour_count = 16;
            memcpy(header->colour_table, rgb121_colour_table, sizeof(rgb121_colour_table));
            break;
        }

        case kSpBmp8bppRgb222:
        case kSpBmp8bppRgb222Rle8: {
            const int64_t image_size = (int64_t) width * (int64_t) height;
            if (image_size > UINT32_MAX) return kSpBmpError;
            header->image_size = (uint32_t) image_size;
            header->pixel_array_offset = BMP_BASIC_HEADER_SIZE + 64 * 4;
            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 8;
            header->compression_type = (format == kSpBmp8bppRgb222) ? BI_RGB : BI_RLE8;
            header->colour_table_size = 64;
            header->important_colour_count = 64;
            memcpy(header->colour_table, rgb222_colour_table, sizeof(rgb222_colour_table));
            break;
        }

        case kSpBmp8bppRgb332:
        case kSpBmp8bppRgb332Rle8: {
            const int64_t image_size = (int64_t) width * (int64_t) height;
            if (image_size > UINT32_MAX) return kSpBmpError;
            header->image_size = (uint32_t) image_size;
            header->pixel_array_offset = BMP_BASIC_HEADER_SIZE + 256 * 4;
            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 8;
            header->compression_type = (format == kSpBmp8bppRgb332) ? BI_RGB : BI_RLE8;
            header->colour_table_size = 256;
            header->important_colour_count = 256;
            generate_rgb332_table(header->colour_table);
            break;
        }

        case kSpBmp16bppRgb555:
        case kSpBmp16bppRgb565: {
            const int64_t image_size = 2 * (int64_t) width * (int64_t) height;
            if (image_size > UINT32_MAX) return kSpBmpError;
            header->image_size = (uint32_t) image_size;
            if (format == kSpBmp16bppRgb565) {
                header->header_size += 12; // 12 bytes for bitmasks
                header->rgb565_flag = 1;
                header->compression_type = BI_BITFIELDS;
            } else {
                header->rgb565_flag = 0;
                header->compression_type = BI_RGB;  // RGB555 uses BI_RGB
            }
            header->pixel_array_offset = header->header_size + BMP_FILE_HEADER_SIZE;
            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 16;
            break;
        }

        case kSpBmp24bpp: {
            const int64_t image_size = 3 * (int64_t) width * (int64_t) height;
            if (image_size > UINT32_MAX) return kSpBmpError;
            header->image_size = (uint32_t) image_size;
            header->pixel_array_offset = BMP_BASIC_HEADER_SIZE;
            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 24;
            break;
        }

        case kSpBmp32bpp: {
            const int64_t image_size = 4 * (int64_t) width * (int64_t) height;
            if (image_size > UINT32_MAX) return kSpBmpError;
            header->image_size = (uint32_t) image_size;
            header->pixel_array_offset = BMP_BASIC_HEADER_SIZE;
            header->file_size = header->pixel_array_offset + header->image_size;
            header->bits_per_pixel = 32;
            break;
        }

        default:
            return kSpBmpUnknownFormat;
    }

    return kSpBmpOk;
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

    ON_FAILURE_RETURN(spbmp_fwrite(file, "BM", 2, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->file_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &dword, 2, 1, userdata)); // Reserved value 1.
    ON_FAILURE_RETURN(spbmp_fwrite(file, &dword, 2, 1, userdata)); // Reserved value 2.
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->pixel_array_offset, 4, 1, userdata));

    ////////////////////////////////////////////////////////////
    // 40-byte DIB Header
    ////////////////////////////////////////////////////////////

    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->header_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->width, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->height, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->num_planes, 2, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->bits_per_pixel, 2, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->compression_type, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->image_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->x_pixels_per_metre, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->y_pixels_per_metre, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->colour_table_size, 4, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &header->important_colour_count, 4, 1, userdata));

    ////////////////////////////////////////////////////////////
    // Extra bytes for 52-byte DIB Header
    ////////////////////////////////////////////////////////////

    if (header->header_size == 52 || header->rgb565_flag == 1) {
        if (header->header_size != 52 || header->rgb565_flag != 1) return kSpBmpError;
        const uint32_t red_mask = 0x0000F800;
        const uint32_t green_mask = 0x000007E0;
        const uint32_t blue_mask = 0x0000001F;
        ON_FAILURE_RETURN(spbmp_fwrite(file, &red_mask, 4, 1, userdata));
        ON_FAILURE_RETURN(spbmp_fwrite(file, &green_mask, 4, 1, userdata));
        ON_FAILURE_RETURN(spbmp_fwrite(file, &blue_mask, 4, 1, userdata));
    }

    ////////////////////////////////////////////////////////////
    // Colour table
    ////////////////////////////////////////////////////////////

    for (uint32_t i = 0; i < header->colour_table_size; ++i) {
        dword = header->colour_table[i][2];        // Blue
        dword += header->colour_table[i][1] << 8;  // Green
        dword += header->colour_table[i][0] << 16; // Red
        dword += header->colour_table[i][3] << 24; // Alpha
        ON_FAILURE_RETURN(spbmp_fwrite(file, &dword, 4, 1, userdata));
    }

    return kSpBmpOk;
}

/**
 * Writes pixel data in 1 bpp (black & white) format.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp1bpp).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_1bpp(void *file, SpBmpFormat format, void *userdata, int x_origin,
                                    int y_origin, int width, int height) {
    assert(format == kSpBmp1bpp);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    const uint8_t zero_padding[4] = { 0 };

    // Calculate how many bytes of data we actually have per row
    int bytes_per_row = (width + 7) / 8;

    // Calculate bytes needed to reach the next 4-byte (32-bit) boundary
    int padding_bytes = (4 - (bytes_per_row % 4)) % 4;

    for (int y = height - 1; y >= 0; --y) {
        uint8_t current_byte = 0;
        int bit_count = 0;

        for (int x = 0; x < width; ++x) {
            SpColourArgb pixel = spbmp_get_pixel_cb(x + x_origin, y + y_origin, userdata);

            // Any non-black pixel (ignoring alpha) is considered white (index 1)
            if ((pixel & 0x00FFFFFF) != 0) {
                // In BMP, the first pixel is the Most Significant Bit (MSB)
                current_byte |= (1 << (7 - bit_count));
            }

            bit_count++;

            // Once we have 8 bits, write the byte and reset
            if (bit_count == 8) {
                ON_FAILURE_RETURN(spbmp_fwrite(file, &current_byte, 1, 1, userdata));
                current_byte = 0;
                bit_count = 0;
            }
        }

        // Write the partial byte if the width wasn't a multiple of 8
        if (bit_count > 0) {
            ON_FAILURE_RETURN(spbmp_fwrite(file, &current_byte, 1, 1, userdata));
        }

        // Write row padding to reach 32-bit alignment
        if (padding_bytes > 0) {
            ON_FAILURE_RETURN(spbmp_fwrite(file, zero_padding, padding_bytes, 1, userdata));
        }
    }

    return kSpBmpOk;
}

/**
 * Writes pixel data in 24 bpp format.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp24bpp).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_24bpp(void *file, SpBmpFormat format, void *userdata, int x_origin,
                                     int y_origin, int width, int height) {
    assert(format == kSpBmp24bpp);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    const char padding[4] = { 0 };

    for (int y = height - 1; y >= 0; --y) {
        int x = 0;
        SpColourArgb pixel = 0x0;
        for (x = 0; x != width; ++x) {
            pixel = spbmp_get_pixel_cb(x + x_origin, y + y_origin, userdata) & 0xFFFFFF;
            // Write BGR order on both big and little-endian systems.
            uint8_t bgr[3] = {
                (pixel >> 0) & 0xFF,   // Blue
                (pixel >> 8) & 0xFF,   // Green
                (pixel >> 16) & 0xFF   // Red
            };
            ON_FAILURE_RETURN(spbmp_fwrite(file, bgr, 3, 1, userdata));
        }
        pixel = 0x0;
        if ((width * 3) % 4 != 0) {
            // Pad each row to multiple of 32-bits.
            ON_FAILURE_RETURN(spbmp_fwrite(file, &padding, 4 - ((width * 3) % 4), 1,
                                          userdata));
        }
    }

    return kSpBmpOk;
}

/**
 * Writes pixel data in 32 bpp format.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp32bpp).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_32bpp(void *file, SpBmpFormat format, void *userdata, int x_origin,
                                     int y_origin, int width, int height) {
    assert(format == kSpBmp32bpp);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    for (int y = height - 1; y >= 0; --y) {
        int x = 0;
        SpColourArgb pixel = 0x0;
        for (x = 0; x != width; ++x) {
            pixel = spbmp_get_pixel_cb(x + x_origin, y + y_origin, userdata);
            // Write BGRA order on both big and little-endian systems.
            uint8_t bgr[4] = {
                (pixel >> 0) & 0xFF,   // Blue
                (pixel >> 8) & 0xFF,   // Green
                (pixel >> 16) & 0xFF,  // Red
                (pixel >> 24) & 0xFF   // Alpha
            };
            ON_FAILURE_RETURN(spbmp_fwrite(file, bgr, 4, 1, userdata));
        }
    }

    return kSpBmpOk;
}

static inline uint8_t spbmp_get_pixel_rgb121(int x, int y, void *userdata) {
    const SpColourArgb pixel = spbmp_get_pixel_cb(x, y, userdata);
    return ((pixel & 0x800000) >> 20) | ((pixel & 0xC000) >> 13) | ((pixel & 0x80) >> 7);
}

#define GET_RGB_121(x, y)  spbmp_get_pixel_rgb121((x) + x_origin, (y) + y_origin, userdata)

/**
 * Writes pixel data in 4 bpp RGB121 format.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp4bppRgb121).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_rgb121(void *file, SpBmpFormat format, void *userdata, int x_origin,
                                      int y_origin, int width, int height) {
    assert(format == kSpBmp4bppRgb121);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

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
            ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
        }

        // Pad each row to 32-bit boundary.
        buf = 0x0;
        for (int x = 0; x < padding; ++x) {
           ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
        }
    }

    return kSpBmpOk;
}

static inline SpBmpResult spbmp_write_rle4_encoded(void *file, void *userdata, int run_length,
                                                   uint8_t pixel) {
    ON_FAILURE_RETURN(spbmp_fwrite(file, &run_length, 1, 1, userdata));
    pixel |= pixel << 4;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &pixel, 1, 1, userdata));
    return kSpBmpOk;
}

static inline SpBmpResult spbmp_write_rle4_absolute(void *file, void *userdata, int count,
                                                    const uint8_t *pixels) {
    uint8_t buf = 0x0;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    buf = (uint8_t)count;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));

    // Pack two 4-bit pixels per byte
    for (int i = 0; i < count; i += 2) {
        uint8_t first = pixels[i];
        uint8_t second = (i + 1 < count) ? pixels[i + 1] : 0;
        buf = (first << 4) | second;
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    }

    // Pad to a 16-bit word boundary.
    int pixel_data_bytes = (count + 1) / 2;
    if (pixel_data_bytes % 2 != 0) {
        buf = 0x0;
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    }

    return kSpBmpOk;
}

/**
 * Writes pixel data in 4 bpp RGB121 format with RLE4 encoding.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp4bppRgb121Rle4).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_rgb121_rle4(void *file, SpBmpFormat format, void *userdata,
                                           int x_origin, int y_origin, int width, int height) {
    assert(format == kSpBmp4bppRgb121Rle4);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    uint8_t buf;
    const int y_start = height - 1;
    const int y_end = -1;
    const int y_delta = -1;

    for (int y = y_start; y != y_end; y += y_delta) {
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
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    }

    // End of bitmap marker.
    buf = 0x0;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    buf = 0x1;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));

    return kSpBmpOk;
}

// Typedef for a function pointer that returns uint8_t and takes
// int x, int y, and void *userdata as arguments.
typedef uint8_t (*SpBmpGetPixel8bpp)(int x, int y, void *userdata);

static inline uint8_t spbmp_get_pixel_rgb222(int x, int y, void *userdata) {
    const SpColourArgb pixel = spbmp_get_pixel_cb(x, y, userdata);
    return ((pixel & 0xC00000) >> 18) | ((pixel & 0xC000) >> 12) | ((pixel & 0xC0) >> 6);
}

static inline uint8_t spbmp_get_pixel_rgb332(int x, int y, void *userdata) {
    const SpColourArgb pixel = spbmp_get_pixel_cb(x, y, userdata);
    return ((pixel & 0xE00000) >> 16) | ((pixel & 0xE000) >> 11) | ((pixel & 0xC0) >> 6);
}

#define GET_PIXEL_8BPP(x, y)  get_pixel_fn((x) + x_origin, (y) + y_origin, userdata)

/**
 * Writes pixel data in 8 bpp RGB222 or RGB332 format.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp8bppRgb222 or kSpBmp8bppRgb332).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_8bpp(void *file, SpBmpFormat format, void *userdata, int x_origin,
                                    int y_origin, int width, int height) {
    assert(format == kSpBmp8bppRgb222 || format == kSpBmp8bppRgb332);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    const SpBmpGetPixel8bpp get_pixel_fn =
        format == kSpBmp8bppRgb222 ? spbmp_get_pixel_rgb222 : spbmp_get_pixel_rgb332;
    const int y_start = height - 1;
    const int y_end = -1;
    const int y_delta = -1;
    const int padding = (4 - (width % 4)) % 4;  // Pad to 32-bit boundary

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;
        for (int x = 0; x < width; x++) {
            const uint8_t pixel = GET_PIXEL_8BPP(x, y);
            ON_FAILURE_RETURN(spbmp_fwrite(file, &pixel, 1, 1, userdata));
        }

        // Pad each row to 32-bit boundary.
        for (int x = 0; x < padding; ++x) {
            const uint8_t pixel = 0x0;
            ON_FAILURE_RETURN(spbmp_fwrite(file, &pixel, 1, 1, userdata));
        }
    }

    return kSpBmpOk;
}

static inline SpBmpResult spbmp_write_rle8_encoded(void *file, void *userdata, int run_length,
                                                   uint8_t pixel) {
    ON_FAILURE_RETURN(spbmp_fwrite(file, &run_length, 1, 1, userdata));
    ON_FAILURE_RETURN(spbmp_fwrite(file, &pixel, 1, 1, userdata));
    return kSpBmpOk;
}

static inline SpBmpResult spbmp_write_rle8_absolute(void *file, void *userdata, int count,
                                                    const uint8_t *pixels) {
    uint8_t buf = 0x0;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    buf = (uint8_t)count;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));

    // Write each pixel as a full byte
    for (int i = 0; i < count; i++) {
        ON_FAILURE_RETURN(spbmp_fwrite(file, &pixels[i], 1, 1, userdata));
    }

    // Pad to 16-bit word boundary
    if (count % 2 != 0) {
        buf = 0x0;
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    }

    return kSpBmpOk;
}

/**
 * Writes pixel data in 8 bpp RGB222 or RGB332 format with RLE8 encoding.
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    The BMP format to write (must be kSpBmp8bppRgb222Rle8 or kSpBmp8bppRgb332Rle8).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_8bpp_rle8(void *file, SpBmpFormat format, void *userdata,
                                         int x_origin, int y_origin, int width, int height) {
    assert(format == kSpBmp8bppRgb222Rle8 || format == kSpBmp8bppRgb332Rle8);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    const SpBmpGetPixel8bpp get_pixel_fn =
        format == kSpBmp8bppRgb222Rle8 ? spbmp_get_pixel_rgb222 : spbmp_get_pixel_rgb332;
    uint8_t buf;
    const int y_start = height - 1;
    const int y_end = -1;
    const int y_delta = -1;

    for (int y = y_start; y != y_end; y += y_delta) {
        if (spbmp_abort_check_cb(userdata) != 0) return kSpBmpAborted;

        int x = 0;
        while (x < width) {
            // Check for run of repeated pixels.
            uint8_t pixel = GET_PIXEL_8BPP(x, y);
            int run_length = 1;
            x++;
            while (x < width && run_length < 255) {
                if (GET_PIXEL_8BPP(x, y) == pixel) {
                    run_length++;
                    x++;
                } else {
                    break;
                }
            }

            if (run_length > 1 || x == width) {
                ON_FAILURE_RETURN(spbmp_write_rle8_encoded(file, userdata, run_length, pixel));
                continue;
            }

            int count = 0;
            uint8_t pixels[255];
            pixels[count++] = pixel;

            while (x < width && count < 255) {
                pixels[count++] = GET_PIXEL_8BPP(x, y);
                x++;
                // Check for the start of a new run.
                if (x < width) {
                    if (GET_PIXEL_8BPP(x, y) == pixels[count - 1]) {
                        count--;
                        x--;
                        break;
                    }
                }
            }

            // We can only use absolute mode where count > 2.
            switch (count) {
                case 1:
                    ON_FAILURE_RETURN(spbmp_write_rle8_encoded(file, userdata, 1, pixels[0]));
                    break;
                case 2:
                    ON_FAILURE_RETURN(spbmp_write_rle8_encoded(file, userdata, 1, pixels[0]));
                    ON_FAILURE_RETURN(spbmp_write_rle8_encoded(file, userdata, 1, pixels[1]));
                    break;
                default:
                    ON_FAILURE_RETURN(spbmp_write_rle8_absolute(file, userdata, count, pixels));
                    break;
            }
        }

        // End of line marker.
        buf = 0x0;
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
        ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    }

    // End of bitmap marker.
    buf = 0x0;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));
    buf = 0x1;
    ON_FAILURE_RETURN(spbmp_fwrite(file, &buf, 1, 1, userdata));

    return kSpBmpOk;
}

#define ENCODE_RGB_555(pixel) ( \
    (((pixel >> 19) & 0x1F) << 10) | \
    (((pixel >> 11) & 0x1F) << 5) | \
    ((pixel >> 3) & 0x1F) \
)

#define ENCODE_RGB_565(pixel) ( \
    (((pixel >> 19) & 0x1F) << 11) | \
    (((pixel >> 10) & 0x3F) << 5) | \
    ((pixel >> 3) & 0x1F) \
)

/**
 * Writes pixel data in 16 bpp format (555 or 565).
 *
 * @param  file      Opaque pointer to destination file.
 * @param  format    Format of the 16 bpp image (kSpBmp16bppRgb555 or kSpBmp16bppRgb565).
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x_origin  The x-coordinate of the top left corner on the source image.
 * @param  y_origin  The y-coordinate of the top left corner on the source image.
 * @param  width     Width of the image to write.
 * @param  height    Height of the image to write.
 * @result           kSpBmpOk on success, all other values indicate an error.
 */
static SpBmpResult spbmp_write_16bpp(void *file, SpBmpFormat format, void *userdata, int x_origin,
                                     int y_origin, int width, int height) {
    assert(format == kSpBmp16bppRgb565 || format == kSpBmp16bppRgb555);
    BmpHeader header;
    ON_FAILURE_RETURN(spbmp_init_header(format, width, height, &header);)
    ON_FAILURE_RETURN(spbmp_write_header(file, userdata, &header));

    const int row_length = width * 2;
    uint8_t buf[2];

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            SpColourArgb pixel = spbmp_get_pixel_cb(x + x_origin, y + y_origin, userdata);

            uint16_t encoded =
                (format == kSpBmp16bppRgb565) ? ENCODE_RGB_565(pixel) : ENCODE_RGB_555(pixel);

            // Write in little-endian format
            buf[0] = encoded & 0xFF;
            buf[1] = (encoded >> 8) & 0xFF;
            ON_FAILURE_RETURN(spbmp_fwrite(file, buf, 2, 1, userdata));
        }

        // Add padding bytes to next 32-bit boundary
        for (int i = row_length; i % 4; ++i) {
            buf[0] = 0x00;
            ON_FAILURE_RETURN(spbmp_fwrite(file, buf, 1, 1, userdata));
        }
    }

    return kSpBmpOk;
}

SpBmpResult spbmp_write(void *file, SpBmpFormat format, void *userdata, int x_origin, int y_origin,
                        int width, int height) {
    switch (format) {
        case kSpBmp1bpp:
            return spbmp_write_1bpp(file, format, userdata, x_origin, y_origin, width, height);
        case kSpBmp4bppRgb121:
            return spbmp_write_rgb121(file, format, userdata, x_origin, y_origin, width, height);

        case kSpBmp4bppRgb121Rle4:
            return spbmp_write_rgb121_rle4(file, format, userdata, x_origin, y_origin, width,
                                           height);

        case kSpBmp8bppRgb222:
        case kSpBmp8bppRgb332:
            return spbmp_write_8bpp(file, format, userdata, x_origin, y_origin, width, height);

        case kSpBmp8bppRgb222Rle8:
        case kSpBmp8bppRgb332Rle8:
            return spbmp_write_8bpp_rle8(file, format, userdata, x_origin, y_origin, width, height);

        case kSpBmp16bppRgb555:
        case kSpBmp16bppRgb565:
            return spbmp_write_16bpp(file, format, userdata, x_origin, y_origin, width, height);

        case kSpBmp24bpp:
            return spbmp_write_24bpp(file, format, userdata, x_origin, y_origin, width, height);

        case kSpBmp32bpp:
            return spbmp_write_32bpp(file, format, userdata, x_origin, y_origin, width, height);

        default:
            return kSpBmpUnknownFormat;
    }
}
