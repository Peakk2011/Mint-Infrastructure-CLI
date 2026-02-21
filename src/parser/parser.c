#include "parser.h"
#include "inline.h"
#include "parser_line.h"
#include "parser_html.h"
#include "../buf/buf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Block parsing states for markdown document traversal. */
typedef enum {
    S_NONE, S_PARAGRAPH, S_UL, S_OL,
    S_BLOCKQUOTE, S_FENCE, S_TABLE, S_HTMLBLOCK
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

/* Main block-level markdown parser. */
char *parse_markdown(const char *src, size_t src_len) {
    size_t n_lines;
    Line *lines = split_lines(src, src_len, &n_lines);
    if (!lines) return NULL;

    Buf out;
    buf_init(&out);
    if (!out.ok) {
        free(lines);
        return NULL;
    }

    State state = S_NONE;
    int in_para = 0;
    int list_item_open = 0;
    char fence_ch = 0;
    int fence_len = 0;
    char html_block_tag[32] = {0};

    for (size_t i = 0; i < n_lines; i++) {
        const char *ptr = lines[i].ptr;
        size_t len = lines[i].len;

        /* Continue fenced code until matching closing fence. */
        if (state == S_FENCE) {
            int is_close = 1;
            for (int k = 0; k < fence_len; k++) {
                if (k >= (int)len || ptr[k] != fence_ch) { is_close = 0; break; }
            }
            if (is_close && (size_t)fence_len == len) {
                buf_puts(&out, "</code></pre>\n");
                state = S_NONE;
            } else {
                buf_escape(&out, ptr, len);
                buf_puts(&out, "\n");
            }
            continue;
        }

        /* Continue raw HTML block until a matching closing tag appears. */
        if (state == S_HTMLBLOCK) {
            parse_inline(&out, ptr, len);
            buf_puts(&out, "\n");
            if (contains_closing_html_tag(ptr, len, html_block_tag)) {
                state = S_NONE;
                html_block_tag[0] = '\0';
            }
            continue;
        }

        /* Fenced code opening: ``` or ~~~, optionally with language name. */
        if ((len >= 3 && ptr[0] == '`' && ptr[1] == '`' && ptr[2] == '`') ||
            (len >= 3 && ptr[0] == '~' && ptr[1] == '~' && ptr[2] == '~')) {
            CLOSE_PARA();
            CLOSE_BLOCK();

            fence_ch = ptr[0];
            fence_len = 3;
            while (fence_len < (int)len && ptr[fence_len] == fence_ch) fence_len++;

            const char *lang = ptr + fence_len;
            size_t lang_len = len - fence_len;
            while (lang_len > 0 && (lang[lang_len - 1] == ' ' || lang[lang_len - 1] == '\t')) lang_len--;

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

        /* Empty line ends paragraphs and list-related blocks. */
        if (is_blank(ptr, len)) {
            CLOSE_PARA();
            if (state == S_UL || state == S_OL || state == S_BLOCKQUOTE) CLOSE_BLOCK();
            continue;
        }

        /* Try raw HTML block line before markdown text parsing. */
        {
            char tag[32];
            int is_closing = 0, is_self_close = 0;
            if (parse_html_tag_line(ptr, len, tag, sizeof(tag), &is_closing, &is_self_close)) {
                if (!is_closing) {
                    CLOSE_PARA();
                    CLOSE_BLOCK();
                    parse_inline(&out, ptr, len);
                    buf_puts(&out, "\n");

                    if (!is_self_close &&
                        !is_void_html_tag(tag, strlen(tag)) &&
                        !contains_closing_html_tag(ptr, len, tag)) {
                        state = S_HTMLBLOCK;
                        strncpy(html_block_tag, tag, sizeof(html_block_tag) - 1);
                        html_block_tag[sizeof(html_block_tag) - 1] = '\0';
                    }
                    continue;
                }
            }
        }

        /* ATX heading */
        {
            int lv = heading_level(ptr, len);
            if (lv) {
                CLOSE_PARA();
                CLOSE_BLOCK();

                char tag[4] = { 'h', '0' + lv, '>', '\0' };
                char etag[5] = { '<', '/', 'h', '0' + lv, '>' };

                buf_puts(&out, "<");
                buf_append(&out, tag, 3);

                const char *text = ptr + lv + 1;
                size_t tlen = len - lv - 1;
                while (tlen > 0 && text[tlen - 1] == '#') tlen--;
                while (tlen > 0 && text[tlen - 1] == ' ') tlen--;
                parse_inline(&out, text, tlen);

                buf_append(&out, etag, 5);
                buf_puts(&out, "\n");
                continue;
            }
        }

        /* Setext heading */
        if (i + 1 < n_lines) {
            const char *nx = lines[i + 1].ptr;
            size_t nxl = lines[i + 1].len;
            int level = setext_level(nx, nxl);
            if (level == 1) {
                CLOSE_PARA(); CLOSE_BLOCK();
                buf_puts(&out, "<h1>"); parse_inline(&out, ptr, len); buf_puts(&out, "</h1>\n");
                i++;
                continue;
            }
            if (level == 2) {
                CLOSE_PARA(); CLOSE_BLOCK();
                buf_puts(&out, "<h2>"); parse_inline(&out, ptr, len); buf_puts(&out, "</h2>\n");
                i++;
                continue;
            }
        }

        /* Horizontal rule */
        if (is_rule(ptr, len)) {
            CLOSE_PARA();
            CLOSE_BLOCK();
            buf_puts(&out, "<hr>\n");
            continue;
        }

        /* Blockquote */
        if (ptr[0] == '>') {
            CLOSE_PARA();
            if (state != S_BLOCKQUOTE) { CLOSE_BLOCK(); buf_puts(&out, "<blockquote>\n"); state = S_BLOCKQUOTE; }
            const char *inner = (len > 1 && ptr[1] == ' ') ? ptr + 2 : ptr + 1;
            size_t ilen = (len > 1 && ptr[1] == ' ') ? len - 2 : len - 1;
            buf_puts(&out, "<p>"); parse_inline(&out, inner, ilen); buf_puts(&out, "</p>\n");
            continue;
        } else if (state == S_BLOCKQUOTE) {
            CLOSE_BLOCK();
        }

        /* Unordered list */
        {
            size_t tlen = 0;
            const char *t = ltrim(ptr, len, &tlen);
            if (tlen >= 2 && (t[0] == '-' || t[0] == '*' || t[0] == '+') && t[1] == ' ') {
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
            if (digits > 0 && digits + 1 < tlen && t[digits] == '.' && t[digits + 1] == ' ') {
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
            int is_header = i + 1 < n_lines && is_table_sep(lines[i + 1].ptr, lines[i + 1].len);
            if (is_header && state != S_TABLE) {
                CLOSE_PARA(); CLOSE_BLOCK();
                buf_puts(&out, "<table>\n<thead>\n");
                emit_table_row(&out, ptr, len, "th");
                buf_puts(&out, "</thead>\n<tbody>\n");
                state = S_TABLE;
                i++;
                continue;
            }
            if (state == S_TABLE) {
                emit_table_row(&out, ptr, len, "td");
                continue;
            }
        } else if (state == S_TABLE) {
            CLOSE_BLOCK();
        }

        /* Paragraph / list-item continuation */
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

/* Extract title from first '# ' heading; fallback to filename stem. */
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