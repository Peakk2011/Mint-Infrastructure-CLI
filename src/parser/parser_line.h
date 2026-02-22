#ifndef PARSER_LINE_H
#define PARSER_LINE_H

#include "../buf/buf.h"
#include <stddef.h>

/* One logical source line (LF/CRLF removed). */
typedef struct {
    const char *ptr;
    size_t len;
} Line;

Line *split_lines(
    const char *src,
    size_t src_len,
    size_t *n_out
);

const char *ltrim(
    const char *s,
    size_t len,
    size_t *trimmed_len
);

int is_blank(
    const char *s,
    size_t len
);

int is_rule(
    const char *s,
    size_t len
);

int heading_level(
    const char *s,
    size_t len
);

int setext_level(
    const char *s,
    size_t len
);

int is_table_sep(
    const char *ptr,
    size_t len
);

void emit_table_row(
    Buf *out,
    const char *ptr,
    size_t len,
    const char *tag
);

#endif