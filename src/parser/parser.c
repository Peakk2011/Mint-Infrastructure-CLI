#include "parser.h"
#include "inline.h"
#include "../buf/buf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* Line */

typedef struct { const char *ptr; size_t len; } Line;

static Line *split_lines(const char *src, size_t src_len, size_t *n_out) {
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

/* Line helpers */

static const char *ltrim(const char *s, size_t len, size_t *trimmed_len) {
    size_t i = 0;
    while (i < len && (s[i] == ' ' || s[i] == '\t')) i++;
    if (trimmed_len) *trimmed_len = len - i;
    return s + i;
}

static int is_blank(const char *s, size_t len) {
    for (size_t i = 0; i < len; i++)
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\r') return 0;
    return 1;
}

static int is_rule(const char *s, size_t len) {
    char c = s[0];
    if (c != '-' && c != '*' && c != '_') return 0;
    size_t count = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == c) count++;
        else if (s[i] != ' ') return 0;
    }
    return count >= 3;
}

static int heading_level(const char *s, size_t len) {
    int lv = 0;
    while (lv < (int)len && s[lv] == '#') lv++;
    if (lv == 0 || lv > 6) return 0;
    if (lv < (int)len && s[lv] == ' ') return lv;
    return 0;
}

static int setext_level(const char *s, size_t len) {
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
    if (i != len || count == 0) return 0;

    return marker == '=' ? 1 : 2;
}

static int is_table_sep(const char *ptr, size_t len) {
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

/* Table ro */

static void emit_table_row(Buf *out, const char *ptr, size_t len, const char *tag) {
    const char *s = ptr, *e = ptr + len;
    if (s < e && *s == '|') s++;
    if (e > s && *(e - 1) == '|') e--;

    buf_puts(out, "<tr>\n");
    const char *p = s;
    while (p <= e) {
        const char *bar = (const char *)memchr(p, '|', e - p);
        const char *ec  = bar ? bar : e;
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

/* Block parser */

typedef enum {
    S_NONE, S_PARAGRAPH, S_UL, S_OL,
    S_BLOCKQUOTE, S_FENCE, S_TABLE
} State;

#define CLOSE_PARA() \
    do { if (in_para) { buf_puts(&out, "</p>\n"); in_para = 0; } } while(0)

#define CLOSE_LIST_ITEM() \
    do { if (list_item_open) { buf_puts(&out, "</li>\n"); list_item_open = 0; } } while(0)

#define CLOSE_BLOCK() \
    do { \
        if      (state == S_UL)         { CLOSE_LIST_ITEM(); buf_puts(&out, "</ul>\n"); } \
        else if (state == S_OL)         { CLOSE_LIST_ITEM(); buf_puts(&out, "</ol>\n"); } \
        else if (state == S_BLOCKQUOTE) buf_puts(&out, "</blockquote>\n"); \
        else if (state == S_TABLE)      buf_puts(&out, "</tbody>\n</table>\n"); \
        state = S_NONE; \
    } while(0)

char *parse_markdown(const char *src, size_t src_len) {
    size_t n_lines;
    Line  *lines = split_lines(src, src_len, &n_lines);
    if (!lines) return NULL;

    Buf   out;
    buf_init(&out);
    if (!out.ok) {
        free(lines);
        return NULL;
    }

    State state     = S_NONE;
    int   in_para   = 0;
    int   list_item_open = 0;
    char  fence_ch  = 0;
    int   fence_len = 0;

    for (size_t i = 0; i < n_lines; i++) {
        const char *ptr = lines[i].ptr;
        size_t      len = lines[i].len;

        /* Inside fenced code */
        if (state == S_FENCE) {
            int is_close = 1;
            for (int k = 0; k < fence_len; k++)
                if (k >= (int)len || ptr[k] != fence_ch) { is_close = 0; break; }
            if (is_close && (size_t)fence_len == len) {
                buf_puts(&out, "</code></pre>\n");
                state = S_NONE;
            } else {
                buf_escape(&out, ptr, len);
                buf_puts(&out, "\n");
            }
            continue;
        }

        /* Fence open */
        if ((len >= 3 && ptr[0] == '`' && ptr[1] == '`' && ptr[2] == '`') ||
            (len >= 3 && ptr[0] == '~' && ptr[1] == '~' && ptr[2] == '~')) {
            CLOSE_PARA(); CLOSE_BLOCK();
            fence_ch  = ptr[0];
            fence_len = 3;
            while (fence_len < (int)len && ptr[fence_len] == fence_ch) fence_len++;
            const char *lang    = ptr + fence_len;
            size_t      lang_len = len - fence_len;
            while (lang_len > 0 && (lang[lang_len-1]==' '||lang[lang_len-1]=='\t')) lang_len--;
            if (lang_len > 0) {
                buf_puts(&out, "<pre><code class=\"language-");
                buf_escape(&out, lang, lang_len);
                buf_puts(&out, "\">");
            } else {
                buf_puts(&out, "<pre><code>");
            }
            state = S_FENCE;
            continue;
        }

        /* Blank line */
        if (is_blank(ptr, len)) {
            CLOSE_PARA();
            if (state==S_UL || state==S_OL || state==S_BLOCKQUOTE) CLOSE_BLOCK();
            continue;
        }

        /* ATX heading */
        {
            int lv = heading_level(ptr, len);
            if (lv) {
                CLOSE_PARA(); CLOSE_BLOCK();
                char tag[4]  = { 'h', '0' + lv, '>', '\0' };
                char etag[5] = { '<', '/', 'h', '0' + lv, '>' };
                buf_puts(&out, "<");
                buf_append(&out, tag, 3);
                const char *text = ptr + lv + 1;
                size_t tlen = len - lv - 1;
                while (tlen > 0 && text[tlen-1] == '#') tlen--;
                while (tlen > 0 && text[tlen-1] == ' ') tlen--;
                parse_inline(&out, text, tlen);
                buf_append(&out, etag, 5);
                buf_puts(&out, "\n");
                continue;
            }
        }

        /* Setext heading */
        if (i + 1 < n_lines) {
            const char *nx  = lines[i+1].ptr;
            size_t       nxl = lines[i+1].len;
            int level = setext_level(nx, nxl);
            if (level == 1) {
                CLOSE_PARA(); CLOSE_BLOCK();
                buf_puts(&out, "<h1>"); parse_inline(&out, ptr, len);
                buf_puts(&out, "</h1>\n"); i++; continue;
            }
            if (level == 2) {
                CLOSE_PARA(); CLOSE_BLOCK();
                buf_puts(&out, "<h2>"); parse_inline(&out, ptr, len);
                buf_puts(&out, "</h2>\n"); i++; continue;
            }
        }

        /* Horizontal rule */
        if (is_rule(ptr, len)) {
            CLOSE_PARA(); CLOSE_BLOCK();
            buf_puts(&out, "<hr>\n");
            continue;
        }

        /* Blockquote */
        if (ptr[0] == '>') {
            CLOSE_PARA();
            if (state != S_BLOCKQUOTE) { CLOSE_BLOCK(); buf_puts(&out, "<blockquote>\n"); state = S_BLOCKQUOTE; }
            const char *inner = (len > 1 && ptr[1] == ' ') ? ptr + 2 : ptr + 1;
            size_t ilen       = (len > 1 && ptr[1] == ' ') ? len - 2 : len - 1;
            buf_puts(&out, "<p>"); parse_inline(&out, inner, ilen); buf_puts(&out, "</p>\n");
            continue;
        } else if (state == S_BLOCKQUOTE) {
            CLOSE_BLOCK();
        }

        /* Unordered list */
        {
            size_t tlen = 0;
            const char *t = ltrim(ptr, len, &tlen);
            if (tlen >= 2 && (t[0]=='-' || t[0]=='*' || t[0]=='+') && t[1]==' ') {
                CLOSE_PARA();
                if (state != S_UL) { CLOSE_BLOCK(); buf_puts(&out, "<ul>\n"); state = S_UL; }
                CLOSE_LIST_ITEM();
                const char *item = t + 2;
                size_t ilen = tlen - 2;
                buf_puts(&out, "<li>");
                parse_inline(&out, item, ilen);
                list_item_open = 1;
                continue;
            }
        }

        /* Ordered list */
        {
            size_t tlen = 0;
            const char *t = ltrim(ptr, len, &tlen);
            size_t digits = 0;
            while (digits < tlen && isdigit((unsigned char)t[digits])) digits++;
            if (digits > 0 && digits + 1 < tlen && t[digits] == '.' && t[digits+1] == ' ') {
                CLOSE_PARA();
                if (state != S_OL) { CLOSE_BLOCK(); buf_puts(&out, "<ol>\n"); state = S_OL; }
                CLOSE_LIST_ITEM();
                const char *item = t + digits + 2;
                size_t ilen = tlen - digits - 2;
                buf_puts(&out, "<li>");
                parse_inline(&out, item, ilen);
                list_item_open = 1;
                continue;
            }
        }

        /* Table */
        if (ptr[0] == '|') {
            int is_header = i+1 < n_lines && is_table_sep(lines[i+1].ptr, lines[i+1].len);
            if (is_header && state != S_TABLE) {
                CLOSE_PARA(); CLOSE_BLOCK();
                buf_puts(&out, "<table>\n<thead>\n");
                emit_table_row(&out, ptr, len, "th");
                buf_puts(&out, "</thead>\n<tbody>\n");
                state = S_TABLE;
                i++; /* skip separator */
                continue;
            }
            if (state == S_TABLE) {
                emit_table_row(&out, ptr, len, "td");
                continue;
            }
        } else if (state == S_TABLE) {
            CLOSE_BLOCK();
        }

        /* Paragraph */
        if (!in_para) {
            if (state == S_UL || state == S_OL) {
                if (list_item_open) {
                    buf_puts(&out, " ");
                } else {
                    buf_puts(&out, "<li>");
                    list_item_open = 1;
                }
                parse_inline(&out, ptr, len);
            } else {
                buf_puts(&out, "<p>");
                parse_inline(&out, ptr, len);
                in_para = 1;
                state = S_PARAGRAPH;
            }
        } else {
            buf_puts(&out, " ");
            parse_inline(&out, ptr, len);
        }
    }

    CLOSE_PARA();
    CLOSE_BLOCK();

    #undef CLOSE_PARA
    #undef CLOSE_LIST_ITEM
    #undef CLOSE_BLOCK

    if (!out.ok) {
        buf_free(&out);
        free(lines);
        return NULL;
    }

    free(lines);
    return out.data;
}

/* Extract title */

void extract_title(const char *md, char *out, size_t out_sz, const char *fallback) {
    const char *p = md;
    while (*p) {
        if (*p == '#' && *(p + 1) == ' ') {
            p += 2;
            const char *end = p;
            while (*end && *end != '\n') end++;
            size_t len = (size_t)(end - p);
            if (len >= out_sz) len = out_sz - 1;
            memcpy(out, p, len);
            out[len] = '\0';
            return;
        }
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
    strncpy(out, fallback, out_sz - 1);
    out[out_sz - 1] = '\0';
}