// spBMP - a Microsoft Windows .bmp decoder.
// Copyright (c) 2024-2025 Thomas Hugo Williams
// License MIT <https://opensource.org/licenses/MIT>
//
// 09-Sep-2024: Version 1.0.2 - Corrected error value returned for unsupported bits per pixel.
// 08-Sep-2024: Version 1.0.1 - Simplified BmpHeader and made some cosmetic changes.
// 08-Sep-2024: Version 1.0.0 - Initial offering.

#if !defined(SPBMP_H)
#define SPBMP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    kSpBmpOk = 0,
    kSpBmpError = 1,  // Other errors
    kSpBmpMissingData = 2,  // File truncated
    kSpBmpAborted = 3,  // abort_check_cb() returned value other than 0.
    kSpBmpUnknownFormat = 4
} SpBmpResult;

typedef enum {
    kSpBmpRgb121,
    kSpBmpRgb121Rle4,
    kSpBmpRgb222,
    kSpBmpRgb222Rle8,
    kSpBmpRgb332,
    kSpBmpRgb332Rle8,
    kSpBmp24bpp,
} SpBmpFormat;

// TODO: This is actually ARGB rather than RGBA.
typedef uint32_t SpColourRgba;

/**
 * Callback that reads a file.
 *
 * @param  file      Opaque pointer to file that should be read.
 * @param  buffer    Pointer to the buffer where the read data should be stored.
 * @param  size      Size of each element to read.
 * @param  count     Number of elements to read.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @return           The number of elements actually read.
 */
typedef size_t (*SpBmpFileReadCb)(void *file, void *buffer, size_t size, size_t count,
                                  void *userdata);

/**
 * Callback that writes a file.
 *
 * @param  file      Opaque pointer to file that should be written.
 * @param  buffer    Pointer to the buffer where the data to be written is stored.
 * @param  size      Size of each element to written.
 * @param  count     Number of elements to written.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @return           The number of elements actually written.
 */
typedef size_t (*SpBmpFileWriteCb)(void *file, const void *buffer, size_t size, size_t count,
                                   void *userdata);

/** Callback that gets a pixel. */
typedef SpColourRgba (*SpBmpGetPixelCb)(int x, int y, void *userdata);

/** Callback that sets a pixel. */
typedef void (*SpBmpSetPixelCb)(int x, int y, SpColourRgba colour, void *userdata);

/** Callback that checks for abort conditions. */
typedef int (*SpBmpAbortCheckCb)(void *userdata);

/**
 * Initialises callbacks.
 *
 * @param  file_read_cb    Callback to read bytes from .bmp file.
 * @param  file_write_cb   Callback to write bytes to a .bmp file.
 * @param  get_pixel_cb    Callback to get a pixel.
 * @param  set_pixel_cb    Callback to set a pixel.
 * @param  abort_check_cb  Callback to check if the BMP load should be aborted
 *                         (if it returns any value other than 0).
 */
void spbmp_init(SpBmpFileReadCb file_read_cb, SpBmpFileWriteCb file_write_cb, 
                SpBmpGetPixelCb get_pixel_cb, SpBmpSetPixelCb set_pixel_cb,
                SpBmpAbortCheckCb abort_check_cb);

/**
 * Loads a bitmap image (.bmp file).
 *
 * @param  file      Opaque pointer to file that should be loaded.
 * @param  x         The x-coordinate of the top left corner on the surface to render the image.
 * @param  y         The y-coordinate of the top left corner on the surface to render the image.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @return           0 on success, all other values indicate an error.
 */
SpBmpResult spbmp_load(void *file, int x, int y, void *userdata);

/**
 * Saves a bitmap image (.bmp file).
 *
 * @param  file      Opaque pointer to file that should be loaded.
 * @param  format    Format to use for .bmp file.
 * @param  userdata  Opaque pointer to "user data" that will be sent to callback functions.
 * @param  x         The x-coordinate of the top left corner on the surface of the image to save.
 * @param  y         The y-coordinate of the top left corner on the surface of the image to save.
 * @param  width     The width of the image to save.
 * @param  height    The height of the image to save.
 * @return           0 on success, all other values indicate an error.
 */
SpBmpResult spbmp_save(void *file, SpBmpFormat format, void *userdata, int x, int y, int width,
                       int height);

#endif  // #if !defined(SPBMP_H)
