#ifndef INLINE_H
#define INLINE_H

#include "../buf/buf.h"
#include <stddef.h>

/* Parse inline markdown (bold, italic, code, links, images) into buf */
void parse_inline(
    Buf *out,
    const char *s,
    size_t len
);

#endif