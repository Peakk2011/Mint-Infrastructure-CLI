#ifndef INLINE_HTML_H
#define INLINE_HTML_H

#include "../buf/buf.h"
#include <stddef.h>

int inline_sanitize_url(const char *url, size_t url_len, const char **out_ptr, size_t *out_len);
int inline_try_emit_html_tag(Buf *out, const char *s, size_t len, size_t *consumed);

#endif