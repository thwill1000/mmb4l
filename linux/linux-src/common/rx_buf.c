#include <stdio.h>

#include "rx_buf.h"

void rx_buf_init(RxBuf *buf, char *data, int data_sz) {
    buf->head = 0;
    buf->tail = 0;
    buf->data = data;
    buf->data_sz = data_sz;
}

void rx_buf_clear(RxBuf *buf) {
    buf->head = 0;
    buf->tail = 0;
}

int rx_buf_get(RxBuf *buf) {
    if (buf->head == buf->tail) {
        return -1;
    } else {
        int result = buf->data[buf->tail++];
        buf->tail %= buf->data_sz;
        return result;
    }
}

int rx_buf_put(RxBuf *buf, char ch) {
    if ((buf->head + 1) % buf->data_sz == buf->tail) {
        return -1;
    } else {
        buf->data[buf->head++] = ch;
        buf->head %= buf->data_sz;
        return 0;
    }
}

int rx_buf_size(RxBuf *buf) {
    if (buf->tail > buf->head) {
        return buf->data_sz + (buf->head - buf->tail);
    } else {
        return buf->head - buf->tail;
    }
}

int rx_buf_unget(RxBuf *buf, char ch) {
    buf->tail--;
    if (buf->tail < 0) buf->tail = buf->data_sz - 1;
    buf->data[buf->tail] = ch;
    if (buf->head == buf->tail) {
        buf->head--;
        if (buf->head < 0) buf->head = buf->data_sz - 1;
        return -1;
    } else {
        return 0;
    }
}

void rx_buf_dump(RxBuf *buf) {
    printf("\n");
    for (int i = 0; i < buf->data_sz; ++i) {
        if (i > 0) printf(", ");
        printf("%d", buf->data[i]);
        if (i == buf->head) printf("H");
        if (i == buf->tail) printf("T");
    }
    printf("\n");
}
