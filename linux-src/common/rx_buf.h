#if !defined(MMB4L_RX_BUFFER_H)
#define MMB4L_RX_BUFFER_H

typedef struct {
    int head;     // next write goes here.
    int tail;     // next read comes from here.
    char *data;
    int data_sz;  // capacity will be 1 less than this.
} RxBuf;

/**
 * Initialises an RxBuf structure.
 *
 * @param  buf      pointer to the buffer.
 * @param  data     character buffer to encapsulate.
 * @param  data_sz  size of 'data' buffer,
 *                  the capacity of the RxBuf will be one less.
 */
void rx_buf_init(RxBuf *buf, char *data, int data_sz);

/**
 * Clears the buffer.
 *
 * @param  buf  pointer to the buffer
 */
void rx_buf_clear(RxBuf *buf);

/**
 * Gets a character from the buffer.
 *
 * @param  buf  pointer to the buffer.
 * @return      the next character, or -1 if the buffer is empty.
 */
int rx_buf_get(RxBuf *buf);

/**
 * Puts a character on the end of the buffer.
 *
 * @param  buf  pointer to the buffer.
 * @param  ch   the character.
 * @return      0 on success,
 *              -1 if the buffer was full, the character is not 'put''.
 */
int rx_buf_put(RxBuf *buf, char ch);

/**
 * Gets the number of items in the buffer.
 *
 * @param  buf  pointer to the buffer
 * @return      the number of items in the buffer.
 */
int rx_buf_size(RxBuf *buf);

/**
 * Puts a character back onto the front of the buffer.
 *
 * @param  buf  pointer to the buffer.
 * @param  ch   the character.
 * @return      0 on success,
 *              -1 if the buffer was full, the last character 'put' in the
 *              buffer is discarded to make room and the character is still
 *              'ungot'.
 */
int rx_buf_unget(RxBuf *buf, char ch);

/**
 * Writes the contents of the buffer to STDOUT.
 *
 * @param  buf  pointer to the buffer
 */
void rx_buf_dump(RxBuf *buf);

#endif
