#include "buf.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void buf_init(Buf *b) {
    b->cap  = 4096;
    b->data = (char *)malloc(b->cap);
    b->len  = 0;
    b->ok   = b->data != NULL;

    if (b->ok) b->data[0] = '\0';
    else b->cap = 0;
}

static int buf_grow(Buf *b, size_t need) {
    if (!b->ok) return 0;
    if (need > SIZE_MAX - b->len - 1) {
        b->ok = 0;
        return 0;
    }

    if (b->len + need + 1 <= b->cap) return 1;

    size_t new_cap = b->cap ? b->cap : 4096;
    while (b->len + need + 1 > new_cap) {
        if (new_cap > SIZE_MAX / 2) {
            b->ok = 0;
            return 0;
        }
        new_cap *= 2;
    }

    char *new_data = (char *)realloc(b->data, new_cap);
    if (!new_data) {
        b->ok = 0;
        return 0;
    }

    b->data = new_data;
    b->cap = new_cap;
    return 1;
}

void buf_append(Buf *b, const char *s, size_t n) {
    if (!b || !b->ok || !s || n == 0) return;
    if (!buf_grow(b, n)) return;
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

void buf_puts(Buf *b, const char *s) {
    if (!s) return;
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
    if (!b) return;
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
    b->ok = 0;
}