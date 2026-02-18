#include "buf.h"
#include <stdlib.h>
#include <string.h>

void buf_init(Buf *b) {
    b->cap  = 4096;
    b->data = (char *)malloc(b->cap);
    b->data[0] = '\0';
    b->len  = 0;
}

static void buf_grow(Buf *b, size_t need) {
    while (b->len + need + 1 > b->cap) b->cap *= 2;
    b->data = (char *)realloc(b->data, b->cap);
}

void buf_append(Buf *b, const char *s, size_t n) {
    buf_grow(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

void buf_puts(Buf *b, const char *s) {
    buf_append(b, s, strlen(s));
}

void buf_escape(Buf *b, const char *s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        switch (s[i]) {
            case '&': buf_puts(b, "&amp;");  break;
            case '<': buf_puts(b, "&lt;");   break;
            case '>': buf_puts(b, "&gt;");   break;
            case '"': buf_puts(b, "&quot;"); break;
            default:  buf_append(b, &s[i], 1);
        }
    }
}

void buf_free(Buf *b) {
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
}