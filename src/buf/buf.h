#ifndef BUF_H
#define BUF_H

#include <stddef.h>

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} Buf;

void buf_init(Buf *b);
void buf_append(Buf *b, const char *s, size_t n);
void buf_puts(Buf *b, const char *s);
void buf_escape(Buf *b, const char *s, size_t n);
void buf_free(Buf *b);

#endif