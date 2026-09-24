// //////////////////////////////////////////////////////////
// toojpeg_streaming.h
//
// Derivative of toojpeg.h (https://create.stephan-brumme.com/toojpeg/),
// written by Stephan Brumme, 2018-2019, zlib licensed (see third_party/LICENSE).
//
// A row-streaming baseline JPEG encoder, callable directly from C. Unlike
// toojpeg.h's writeJpeg(), which needs the entire image as one flat
// width*height*bytesPerPixel buffer, toojpeg_write_streaming() pulls pixel
// data on demand, one row at a time, through a callback - so peak memory
// use is bounded by a small caller-supplied row window (8 or 16 image rows)
// rather than by the whole image. Intended for RAM-constrained targets
// (e.g. microcontrollers) where holding a full frame buffer copy isn't
// practical.
//
// This header/implementation is plain C-callable (all extern "C") even
// though toojpeg_streaming.cpp is C++ internally - there's no separate C
// wrapper layer to it, since the only place a C++ vs. C linkage boundary
// actually matters is right here at the public entry points.

#if !defined(TOOJPEG_STREAMING_H)
#define TOOJPEG_STREAMING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fetches one row of pixel data into row_buffer (caller-owned, at least
 * width * (is_rgb ? 3 : 1) bytes), for the given absolute (0-based) image
 * row. Called at most once per row - each row is cached internally until
 * the encoder has permanently finished with it.
 */
typedef void (*ToojpegGetRowCb)(unsigned short row, unsigned char *row_buffer, void *userdata);

/**
 * Receives a chunk of encoded JPEG bytes.
 *
 * @return  Non-zero on success, 0 to abort encoding.
 */
typedef int (*ToojpegWriteCb)(const void *buffer, size_t size, void *userdata);

/**
 * Returns how many bytes the row_window buffer passed to
 * toojpeg_write_streaming() must be at least: 8 or 16 image rows worth
 * (16 if downsample is set), each width * (is_rgb ? 3 : 1) bytes.
 */
size_t toojpeg_row_window_size(int width, int is_rgb, int downsample);

/**
 * Encodes an image to baseline JPEG, pulling pixel rows on demand via
 * get_row_cb and streaming the compressed output to write_cb.
 *
 * @param  width           Image width in pixels (1 - 65535).
 * @param  height          Image height in pixels (1 - 65535).
 * @param  is_rgb          Non-zero for RGB888 rows (3 bytes/pixel), zero for
 *                          8-bit grayscale rows (1 byte/pixel).
 * @param  quality         JPEG quality, clamped to the range 1 (worst) - 100 (best).
 * @param  downsample      Non-zero to use YCbCr 4:2:0 chroma subsampling
 *                          (smaller file, minor quality loss); ignored for
 *                          grayscale. Also determines whether row_window
 *                          must hold 8 or 16 rows - see toojpeg_row_window_size().
 * @param  comment         Optional NUL-terminated JPEG comment, or NULL for
 *                          none. Must not contain the byte 0xFF.
 * @param  get_row_cb      Callback that fills one row of pixel data on demand.
 * @param  get_row_userdata  Opaque pointer passed through to get_row_cb.
 * @param  row_window      Caller-supplied scratch buffer of at least
 *                          toojpeg_row_window_size(width, is_rgb, downsample)
 *                          bytes. May be a stack, static, or heap buffer -
 *                          this function performs no allocations of its own.
 * @param  write_cb        Callback invoked with encoded bytes as they are produced.
 * @param  write_userdata  Opaque pointer passed through to write_cb.
 * @return                  1 on success, 0 on failure (bad arguments, or
 *                          write_cb returned 0).
 */
int toojpeg_write_streaming(int width, int height, int is_rgb, int quality, int downsample,
                            const char *comment, ToojpegGetRowCb get_row_cb,
                            void *get_row_userdata, unsigned char *row_window,
                            ToojpegWriteCb write_cb, void *write_userdata);

#ifdef __cplusplus
}
#endif

#endif  // #if !defined(TOOJPEG_STREAMING_H)
