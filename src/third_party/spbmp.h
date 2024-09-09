// spBMP - a Microsoft Windows .bmp decoder.
// Copyright (c) 2024 Thomas Hugo Williams
// License MIT <https://opensource.org/licenses/MIT>
//
// 09-Sep-2024: Version 1.0.2 - Corrected error value returned for unsupported bits per pixel.
// 08-Sep-2024: Version 1.0.1 - Simplified BmpHeader and made some cosmetic changes.
// 08-Sep-2024: Version 1.0.0 - Initial offering.

#if !defined(SPBMP_H)
#define SPBMP_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    kSpBmpOk = 0,
    kSpBmpError = 1,  // Other errors
    kSpBmpMissingData = 2,  // File truncated
    kSpBmpAborted = 3  // abort_check_cb() returned value other than 0.
} SpBmpResult;

typedef uint32_t SpColourRgba;

typedef size_t (*SpBmpFileReadCb)(void *file, void *buffer, size_t size, size_t count,
                                  void *userdata);
typedef void (*SpBmpSetPixelCb)(int x, int y, SpColourRgba colour, void *userdata);
typedef int (*SpBmpAbortCheckCb)(void *userdata);

/**
 * Initialises callbacks.
 *
 * @param  file_read_cb    Callback to read bytes from .bmp file.
 * @param  set_pixel_cb    Callback to set pixels.
 * @param  abort_check_cb  Callback to check if the BMP load should be aborted
 *                         (if it returns any value other than 0).
 */
void spbmp_init(SpBmpFileReadCb file_read_cb, SpBmpSetPixelCb set_pixel_cb,
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

#endif  // #if !defined(SPBMP_H)
