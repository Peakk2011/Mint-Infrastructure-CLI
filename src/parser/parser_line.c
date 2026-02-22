#include "parser_line.h"
#include "inline.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* Split source buffer into line slices used by the block parser. */
Line *split_lines(const char *src, size_t src_len, size_t *n_out) {
    size_t cap = 64, n = 0;
    Line *lines = (Line *)malloc(cap * sizeof(Line));
    if (!lines) {
        *n_out = 0;
        return NULL;
    }

    const char *p = src, *end = src + src_len;
    while (p < end) {
        const char *nl = (const char *)memchr(p, '\n', end - p);
        size_t len = nl ? (size_t)(nl - p) : (size_t)(end - p);
        if (len > 0 && p[len - 1] == '\r') len--;

        if (n == cap) {
            if (cap > SIZE_MAX / 2 / sizeof(Line)) {
                free(lines);
                *n_out = 0;
                return NULL;
            }
            cap *= 2;
            Line *tmp = (Line *)realloc(lines, cap * sizeof(Line));
            if (!tmp) {
                free(lines);
                *n_out = 0;
                return NULL;
            }
            lines = tmp;
        }

        lines[n].ptr = p;
        lines[n].len = len;
        n++;
        p = nl ? nl + 1 : end;
    }

    *n_out = n;
    return lines;
}

/* Left-trim spaces/tabs to support markdown indentation checks. */
const char *ltrim(const char *s, size_t len, size_t *trimmed_len) {
    size_t i = 0;
    while (i < len && (s[i] == ' ' || s[i] == '\t')) i++;
    if (trimmed_len) *trimmed_len = len - i;
    return s + i;
}

/* True when line has only whitespace. */
int is_blank(const char *s, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\r') return 0;
    }
    return 1;
}

/* Horizontal rule detector: ---, ***, ___ with optional spaces. */
int is_rule(const char *s, size_t len) {
    if (len == 0) return 0;

    char c = s[0];
    if (c != '-' && c != '*' && c != '_') return 0;

    size_t count = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == c) count++;
        else if (s[i] != ' ' && s[i] != '\t') return 0;
    }
    return count >= 3;
}

/* ATX heading detector (#..######). Returns level 1..6 or 0. */
int heading_level(const char *s, size_t len) {
    int lv = 0;
    while (lv < (int)len && s[lv] == '#') lv++;
    if (lv == 0 || lv > 6) return 0;
    if (lv < (int)len && s[lv] == ' ') return lv;
    return 0;
}

/* Setext heading underline detector. Returns level 1/2 or 0.
   Requires at least 2 markers to avoid accidental headings on lone '-'. */
int setext_level(const char *s, size_t len) {
    size_t i = 0;
    while (i < len && (s[i] == ' ' || s[i] == '\t')) i++;
    if (i == len) return 0;

    char marker = s[i];
    if (marker != '=' && marker != '-') return 0;

    size_t count = 0;
    while (i < len && s[i] == marker) {
        count++;
        i++;
    }
    while (i < len && (s[i] == ' ' || s[i] == '\t')) i++;
    if (i != len || count < 2) return 0;

    return marker == '=' ? 1 : 2;
}

/* Table separator detector for markdown pipe tables. */
int is_table_sep(const char *ptr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char c = ptr[i];
        if (c != '|' && c != '-' && c != ':' && c != ' ' && c != '\t') return 0;
    }

    int has_pipe = 0, has_dash = 0;
    for (size_t i = 0; i < len; i++) {
        if (ptr[i] == '|') has_pipe = 1;
        if (ptr[i] == '-') has_dash = 1;
    }
    return has_pipe && has_dash;
}

/* Emit a single table row from a pipe-delimited line. */
void emit_table_row(Buf *out, const char *ptr, size_t len, const char *tag) {
    const char *s = ptr, *e = ptr + len;
    if (s < e && *s == '|') s++;
    if (e > s && *(e - 1) == '|') e--;

    buf_puts(out, "<tr>\n");
    const char *p = s;
    while (p <= e) {
        const char *bar = (const char *)memchr(p, '|', e - p);
        const char *ec = bar ? bar : e;

        while (p < ec && (*p == ' ' || *p == '\t')) p++;
        const char *ce = ec;
        while (ce > p && (*(ce - 1) == ' ' || *(ce - 1) == '\t')) ce--;

        buf_puts(out, "<"); buf_puts(out, tag); buf_puts(out, ">");
        parse_inline(out, p, ce - p);
        buf_puts(out, "</"); buf_puts(out, tag); buf_puts(out, ">\n");

        if (!bar) break;
        p = bar + 1;
    }
    buf_puts(out, "</tr>\n");
}
