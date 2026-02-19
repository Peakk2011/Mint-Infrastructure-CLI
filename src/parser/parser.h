#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/* Parse markdown source into heap-allocated HTML string.
   Caller must free() the result. Returns NULL on error. */
char *parse_markdown(const char *src, size_t src_len);

/* Extract first # heading into out buffer.
   Falls back to fallback string if no heading found. */
void extract_title(const char *md, char *out, size_t out_sz, const char *fallback);

#endif