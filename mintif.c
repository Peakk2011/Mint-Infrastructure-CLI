/**
 * Mint Infrastructure — Markdown Compiler
 * Written in C99.
 *
 * Compile:
 *   gcc mintif.c -o mintif.exe -std=c99 -O2
 *
 * Usage:
 *   ./mintif input.md -o output.html
 *   ./mintif input.md                   -> outputs input.html
 *   ./mintif input.md -t "My Title"
 *   ./mintif input.md -s custom.css
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*  String Buffer */

typedef struct
{
    char *data;
    size_t len;
    size_t cap;
} Buf;

static void buf_init(Buf *b)
{
    b->cap = 4096;
    b->data = (char *)malloc(b->cap);
    b->data[0] = '\0';
    b->len = 0;
}

static void buf_grow(Buf *b, size_t need)
{
    while (b->len + need + 1 > b->cap)
        b->cap *= 2;
    b->data = (char *)realloc(b->data, b->cap);
}

static void buf_append(Buf *b, const char *s, size_t n)
{
    buf_grow(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void buf_puts(Buf *b, const char *s)
{
    buf_append(b, s, strlen(s));
}

static void buf_free(Buf *b)
{
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
}

/*  File I/O */

static char *read_file(const char *path, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    size_t sz = (size_t)ftell(f);
    rewind(f);
    char *buf = (char *)malloc(sz + 1);
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    if (out_len)
        *out_len = sz;
    return buf;
}

static int write_file(const char *path, const char *data, size_t len)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return 0;
    fwrite(data, 1, len, f);
    fclose(f);
    return 1;
}

/*  HTML Escape */

static void buf_escape(Buf *b, const char *s, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        switch (s[i])
        {
        case '&':
            buf_puts(b, "&amp;");
            break;
        case '<':
            buf_puts(b, "&lt;");
            break;
        case '>':
            buf_puts(b, "&gt;");
            break;
        case '"':
            buf_puts(b, "&quot;");
            break;
        default:
            buf_append(b, &s[i], 1);
        }
    }
}

/*  Inline Markdown Parser */
/* Handles: **bold** *italic* `code` ~~strike~~ [link](url) ![img](url) */

static void parse_inline(Buf *out, const char *s, size_t len)
{
    size_t i = 0;
    while (i < len)
    {

        /* Escape HTML */
        if (s[i] == '&')
        {
            buf_puts(out, "&amp;");
            i++;
            continue;
        }
        if (s[i] == '<')
        {
            buf_puts(out, "&lt;");
            i++;
            continue;
        }
        if (s[i] == '>')
        {
            buf_puts(out, "&gt;");
            i++;
            continue;
        }

        /* ![alt](url) */
        if (s[i] == '!' && i + 1 < len && s[i + 1] == '[')
        {
            size_t a = i + 2;
            size_t a_end = a;
            while (a_end < len && s[a_end] != ']')
                a_end++;
            if (a_end < len && a_end + 1 < len && s[a_end + 1] == '(')
            {
                size_t u = a_end + 2, u_end = u;
                while (u_end < len && s[u_end] != ')')
                    u_end++;
                if (u_end < len)
                {
                    buf_puts(out, "<img alt=\"");
                    buf_escape(out, s + a, a_end - a);
                    buf_puts(out, "\" src=\"");
                    buf_escape(out, s + u, u_end - u);
                    buf_puts(out, "\">");
                    i = u_end + 1;
                    continue;
                }
            }
        }

        /* [text](url) */
        if (s[i] == '[')
        {
            size_t t = i + 1, t_end = t;
            while (t_end < len && s[t_end] != ']')
                t_end++;
            if (t_end < len && t_end + 1 < len && s[t_end + 1] == '(')
            {
                size_t u = t_end + 2, u_end = u;
                while (u_end < len && s[u_end] != ')')
                    u_end++;
                if (u_end < len)
                {
                    buf_puts(out, "<a href=\"");
                    buf_escape(out, s + u, u_end - u);
                    buf_puts(out, "\">");
                    parse_inline(out, s + t, t_end - t);
                    buf_puts(out, "</a>");
                    i = u_end + 1;
                    continue;
                }
            }
        }

        /* ***bold+italic*** */
        if (i + 2 < len && s[i] == '*' && s[i + 1] == '*' && s[i + 2] == '*')
        {
            size_t e = i + 3;
            while (e + 2 < len && !(s[e] == '*' && s[e + 1] == '*' && s[e + 2] == '*'))
                e++;
            if (e + 2 < len)
            {
                buf_puts(out, "<strong><em>");
                parse_inline(out, s + i + 3, e - (i + 3));
                buf_puts(out, "</em></strong>");
                i = e + 3;
                continue;
            }
        }

        /* **bold** */
        if (i + 1 < len && s[i] == '*' && s[i + 1] == '*')
        {
            size_t e = i + 2;
            while (e + 1 < len && !(s[e] == '*' && s[e + 1] == '*'))
                e++;
            if (e + 1 < len)
            {
                buf_puts(out, "<strong>");
                parse_inline(out, s + i + 2, e - (i + 2));
                buf_puts(out, "</strong>");
                i = e + 2;
                continue;
            }
        }

        /* *italic* */
        if (s[i] == '*' && (i == 0 || s[i - 1] != '*'))
        {
            size_t e = i + 1;
            while (e < len && s[e] != '*')
                e++;
            if (e < len)
            {
                buf_puts(out, "<em>");
                parse_inline(out, s + i + 1, e - (i + 1));
                buf_puts(out, "</em>");
                i = e + 1;
                continue;
            }
        }

        /* _italic_ */
        if (s[i] == '_')
        {
            size_t e = i + 1;
            while (e < len && s[e] != '_')
                e++;
            if (e < len)
            {
                buf_puts(out, "<em>");
                parse_inline(out, s + i + 1, e - (i + 1));
                buf_puts(out, "</em>");
                i = e + 1;
                continue;
            }
        }

        /* ~~strikethrough~~ */
        if (i + 1 < len && s[i] == '~' && s[i + 1] == '~')
        {
            size_t e = i + 2;
            while (e + 1 < len && !(s[e] == '~' && s[e + 1] == '~'))
                e++;
            if (e + 1 < len)
            {
                buf_puts(out, "<del>");
                parse_inline(out, s + i + 2, e - (i + 2));
                buf_puts(out, "</del>");
                i = e + 2;
                continue;
            }
        }

        /* `code` */
        if (s[i] == '`')
        {
            size_t e = i + 1;
            while (e < len && s[e] != '`')
                e++;
            if (e < len)
            {
                buf_puts(out, "<code>");
                buf_escape(out, s + i + 1, e - (i + 1));
                buf_puts(out, "</code>");
                i = e + 1;
                continue;
            }
        }

        buf_append(out, &s[i], 1);
        i++;
    }
}

/*  Line Helpers */

/* Returns pointer to first non-space char */
static const char *ltrim(const char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

/* true if line is blank */
static int is_blank(const char *s, size_t len)
{
    for (size_t i = 0; i < len; i++)
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\r')
            return 0;
    return 1;
}

/* true if line is all same char (--- === ***) with len >= 3 */
static int is_rule(const char *s, size_t len)
{
    char c = s[0];
    if (c != '-' && c != '*' && c != '_')
        return 0;
    size_t count = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (s[i] == c)
            count++;
        else if (s[i] != ' ')
            return 0;
    }
    return count >= 3;
}

/* Count leading # chars */
static int heading_level(const char *s, size_t len)
{
    int lv = 0;
    while (lv < (int)len && s[lv] == '#')
        lv++;
    if (lv == 0 || lv > 6)
        return 0;
    if (lv < (int)len && s[lv] == ' ')
        return lv;
    return 0;
}

/*  Split into lines */

typedef struct
{
    const char *ptr;
    size_t len;
} Line;

static Line *split_lines(const char *src, size_t src_len, size_t *n_out)
{
    size_t cap = 64, n = 0;
    Line *lines = (Line *)malloc(cap * sizeof(Line));
    const char *p = src, *end = src + src_len;
    while (p < end)
    {
        const char *nl = (const char *)memchr(p, '\n', end - p);
        size_t len = nl ? (size_t)(nl - p) : (size_t)(end - p);
        /* strip \r */
        if (len > 0 && p[len - 1] == '\r')
            len--;
        if (n == cap)
        {
            cap *= 2;
            lines = (Line *)realloc(lines, cap * sizeof(Line));
        }
        lines[n].ptr = p;
        lines[n].len = len;
        n++;
        p = nl ? nl + 1 : end;
    }
    *n_out = n;
    return lines;
}

/*  Markdown Block Parser */

typedef enum
{
    S_NONE,
    S_PARAGRAPH,
    S_UL,
    S_OL,
    S_BLOCKQUOTE,
    S_FENCE,
    S_TABLE
} State;

static void parse_table_row(Buf *out, const char *ptr, size_t len,
                            const char *cell_tag)
{
    /* strip leading/trailing | */
    const char *s = ptr, *e = ptr + len;
    if (s < e && *s == '|')
        s++;
    if (e > s && *(e - 1) == '|')
        e--;

    out->len > 0 ? buf_puts(out, "<tr>\n") : buf_puts(out, "<tr>\n");
    buf_puts(out, "<tr>\n");

    const char *p = s;
    while (p <= e)
    {
        const char *bar = (const char *)memchr(p, '|', e - p);
        const char *end_cell = bar ? bar : e;
        /* trim whitespace */
        while (p < end_cell && (*p == ' ' || *p == '\t'))
            p++;
        const char *ce = end_cell;
        while (ce > p && (*(ce - 1) == ' ' || *(ce - 1) == '\t'))
            ce--;

        buf_puts(out, "<");
        buf_puts(out, cell_tag);
        buf_puts(out, ">");
        parse_inline(out, p, ce - p);
        buf_puts(out, "</");
        buf_puts(out, cell_tag);
        buf_puts(out, ">\n");

        if (!bar)
            break;
        p = bar + 1;
    }
    buf_puts(out, "</tr>\n");
}

static int is_table_sep(const char *ptr, size_t len)
{
    /* must start with | or - and contain only |, -, :, space */
    for (size_t i = 0; i < len; i++)
    {
        char c = ptr[i];
        if (c != '|' && c != '-' && c != ':' && c != ' ' && c != '\t')
            return 0;
    }
    /* must have at least one | and one - */
    int has_pipe = 0, has_dash = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (ptr[i] == '|')
            has_pipe = 1;
        if (ptr[i] == '-')
            has_dash = 1;
    }
    return has_pipe && has_dash;
}

static char *parse_markdown(const char *src, size_t src_len)
{
    size_t n_lines;
    Line *lines = split_lines(src, src_len, &n_lines);

    Buf out;
    buf_init(&out);

    State state = S_NONE;
    int in_para = 0;
    char fence_ch = 0;
    int fence_len = 0;

/* helpers as inline lambdas don't exist in C — use macros */
#define CLOSE_PARA()                  \
    do                                \
    {                                 \
        if (in_para)                  \
        {                             \
            buf_puts(&out, "</p>\n"); \
            in_para = 0;              \
        }                             \
    } while (0)
#define CLOSE_BLOCK()                               \
    do                                              \
    {                                               \
        if (state == S_UL)                          \
            buf_puts(&out, "</ul>\n");              \
        else if (state == S_OL)                     \
            buf_puts(&out, "</ol>\n");              \
        else if (state == S_BLOCKQUOTE)             \
            buf_puts(&out, "</blockquote>\n");      \
        else if (state == S_TABLE)                  \
            buf_puts(&out, "</tbody>\n</table>\n"); \
        state = S_NONE;                             \
    } while (0)

    for (size_t i = 0; i < n_lines; i++)
    {
        const char *ptr = lines[i].ptr;
        size_t len = lines[i].len;

        /*  Inside fenced code */
        if (state == S_FENCE)
        {
            int is_close = 1;
            for (int k = 0; k < fence_len; k++)
                if (k >= (int)len || ptr[k] != fence_ch)
                {
                    is_close = 0;
                    break;
                }
            if (is_close && (size_t)fence_len == len)
            {
                buf_puts(&out, "</code></pre>\n");
                state = S_NONE;
            }
            else
            {
                buf_escape(&out, ptr, len);
                buf_puts(&out, "\n");
            }
            continue;
        }

        /*  Fence open */
        if ((len >= 3 && ptr[0] == '`' && ptr[1] == '`' && ptr[2] == '`') ||
            (len >= 3 && ptr[0] == '~' && ptr[1] == '~' && ptr[2] == '~'))
        {
            CLOSE_PARA();
            CLOSE_BLOCK();
            fence_ch = ptr[0];
            fence_len = 3;
            while (fence_len < (int)len && ptr[fence_len] == fence_ch)
                fence_len++;
            /* language hint */
            const char *lang = ptr + fence_len;
            size_t lang_len = len - fence_len;
            while (lang_len > 0 && (lang[lang_len - 1] == ' ' || lang[lang_len - 1] == '\t'))
                lang_len--;
            if (lang_len > 0)
            {
                buf_puts(&out, "<pre><code class=\"language-");
                buf_escape(&out, lang, lang_len);
                buf_puts(&out, "\">");
            }
            else
            {
                buf_puts(&out, "<pre><code>");
            }
            state = S_FENCE;
            continue;
        }

        /*  Blank line */
        if (is_blank(ptr, len))
        {
            CLOSE_PARA();
            if (state == S_UL || state == S_OL || state == S_BLOCKQUOTE)
                CLOSE_BLOCK();
            continue;
        }

        /*  ATX Heading  # */
        {
            int lv = heading_level(ptr, len);
            if (lv)
            {
                CLOSE_PARA();
                CLOSE_BLOCK();
                char tag[4] = {'h', '0' + lv, '>', '\0'};
                char etag[5] = {'<', '/', 'h', '0' + lv, '>'};
                buf_puts(&out, "<");
                buf_append(&out, tag, 3);
                const char *text = ptr + lv + 1;
                size_t tlen = len - lv - 1;
                /* strip trailing # */
                while (tlen > 0 && text[tlen - 1] == '#')
                    tlen--;
                while (tlen > 0 && text[tlen - 1] == ' ')
                    tlen--;
                parse_inline(&out, text, tlen);
                buf_append(&out, etag, 5);
                buf_puts(&out, "\n");
                continue;
            }
        }

        /*  Setext heading */
        if (i + 1 < n_lines)
        {
            const char *nx = lines[i + 1].ptr;
            size_t nxl = lines[i + 1].len;
            int all_eq = 1, all_dash = 1;
            for (size_t k = 0; k < nxl; k++)
            {
                if (nx[k] != '=')
                    all_eq = 0;
                if (nx[k] != '-')
                    all_dash = 0;
            }
            if (all_eq && nxl >= 1)
            {
                CLOSE_PARA();
                CLOSE_BLOCK();
                buf_puts(&out, "<h1>");
                parse_inline(&out, ptr, len);
                buf_puts(&out, "</h1>\n");
                i++;
                continue;
            }
            if (all_dash && nxl >= 2)
            {
                CLOSE_PARA();
                CLOSE_BLOCK();
                buf_puts(&out, "<h2>");
                parse_inline(&out, ptr, len);
                buf_puts(&out, "</h2>\n");
                i++;
                continue;
            }
        }

        /*  Horizontal rule */
        if (is_rule(ptr, len))
        {
            CLOSE_PARA();
            CLOSE_BLOCK();
            buf_puts(&out, "<hr>\n");
            continue;
        }

        /*  Blockquote  > */
        if (ptr[0] == '>')
        {
            CLOSE_PARA();
            if (state != S_BLOCKQUOTE)
            {
                CLOSE_BLOCK();
                buf_puts(&out, "<blockquote>\n");
                state = S_BLOCKQUOTE;
            }
            const char *inner = ptr[1] == ' ' ? ptr + 2 : ptr + 1;
            size_t ilen = ptr[1] == ' ' ? len - 2 : len - 1;
            buf_puts(&out, "<p>");
            parse_inline(&out, inner, ilen);
            buf_puts(&out, "</p>\n");
            continue;
        }
        else if (state == S_BLOCKQUOTE)
        {
            CLOSE_BLOCK();
        }

        /*  Unordered list  - * + */
        {
            const char *t = ltrim(ptr);
            if ((t[0] == '-' || t[0] == '*' || t[0] == '+') && t[1] == ' ')
            {
                CLOSE_PARA();
                if (state != S_UL)
                {
                    CLOSE_BLOCK();
                    buf_puts(&out, "<ul>\n");
                    state = S_UL;
                }
                const char *item = t + 2;
                size_t ilen = len - (size_t)(item - ptr);
                buf_puts(&out, "<li>");
                parse_inline(&out, item, ilen);
                buf_puts(&out, "</li>\n");
                continue;
            }
        }

        /*  Ordered list  1. */
        {
            const char *t = ltrim(ptr);
            int digits = 0;
            while (isdigit((unsigned char)t[digits]))
                digits++;
            if (digits > 0 && t[digits] == '.' && t[digits + 1] == ' ')
            {
                CLOSE_PARA();
                if (state != S_OL)
                {
                    CLOSE_BLOCK();
                    buf_puts(&out, "<ol>\n");
                    state = S_OL;
                }
                const char *item = t + digits + 2;
                size_t ilen = len - (size_t)(item - ptr);
                buf_puts(&out, "<li>");
                parse_inline(&out, item, ilen);
                buf_puts(&out, "</li>\n");
                continue;
            }
        }

        /*  Table */
        if (ptr[0] == '|')
        {
            int is_header = i + 1 < n_lines && is_table_sep(lines[i + 1].ptr, lines[i + 1].len);
            if (is_header && state != S_TABLE)
            {
                CLOSE_PARA();
                CLOSE_BLOCK();
                buf_puts(&out, "<table>\n<thead>\n");
                /* remove the duplicate <tr> that parse_table_row adds */
                Buf tmp;
                buf_init(&tmp);
                /* build header manually */
                const char *s = ptr, *e = ptr + len;
                if (*s == '|')
                    s++;
                if (e > s && *(e - 1) == '|')
                    e--;
                buf_puts(&out, "<tr>\n");
                const char *p = s;
                while (p <= e)
                {
                    const char *bar = (const char *)memchr(p, '|', e - p);
                    const char *ec = bar ? bar : e;
                    while (p < ec && (*p == ' ' || *p == '\t'))
                        p++;
                    const char *ce = ec;
                    while (ce > p && (*(ce - 1) == ' ' || *(ce - 1) == '\t'))
                        ce--;
                    buf_puts(&out, "<th>");
                    parse_inline(&out, p, ce - p);
                    buf_puts(&out, "</th>\n");
                    if (!bar)
                        break;
                    p = bar + 1;
                }
                buf_puts(&out, "</tr>\n</thead>\n<tbody>\n");
                buf_free(&tmp);
                state = S_TABLE;
                i++; /* skip separator */
                continue;
            }
            if (state == S_TABLE)
            {
                const char *s = ptr, *e = ptr + len;
                if (*s == '|')
                    s++;
                if (e > s && *(e - 1) == '|')
                    e--;
                buf_puts(&out, "<tr>\n");
                const char *p = s;
                while (p <= e)
                {
                    const char *bar = (const char *)memchr(p, '|', e - p);
                    const char *ec = bar ? bar : e;
                    while (p < ec && (*p == ' ' || *p == '\t'))
                        p++;
                    const char *ce = ec;
                    while (ce > p && (*(ce - 1) == ' ' || *(ce - 1) == '\t'))
                        ce--;
                    buf_puts(&out, "<td>");
                    parse_inline(&out, p, ce - p);
                    buf_puts(&out, "</td>\n");
                    if (!bar)
                        break;
                    p = bar + 1;
                }
                buf_puts(&out, "</tr>\n");
                continue;
            }
        }
        else if (state == S_TABLE)
        {
            CLOSE_BLOCK();
        }

        /*  Paragraph */
        if (!in_para)
        {
            if (state == S_UL || state == S_OL)
            {
                parse_inline(&out, ptr, len);
                buf_puts(&out, "\n");
            }
            else
            {
                buf_puts(&out, "<p>");
                parse_inline(&out, ptr, len);
                in_para = 1;
                state = S_PARAGRAPH;
            }
        }
        else
        {
            buf_puts(&out, " ");
            parse_inline(&out, ptr, len);
        }
    }

    /* Close any open blocks */
    CLOSE_PARA();
    CLOSE_BLOCK();

#undef CLOSE_PARA
#undef CLOSE_BLOCK

    free(lines);
    return out.data; /* caller owns this */
}

/*  Extract title from first # heading */

static void extract_title(const char *md, char *out, size_t out_sz, const char *fallback)
{
    const char *p = md;
    while (*p)
    {
        if (*p == '#' && *(p + 1) == ' ')
        {
            p += 2;
            const char *end = p;
            while (*end && *end != '\n')
                end++;
            size_t len = (size_t)(end - p);
            if (len >= out_sz)
                len = out_sz - 1;
            memcpy(out, p, len);
            out[len] = '\0';
            return;
        }
        while (*p && *p != '\n')
            p++;
        if (*p == '\n')
            p++;
    }
    strncpy(out, fallback, out_sz - 1);
    out[out_sz - 1] = '\0';
}

/* Path helpers */

/* Replace extension of path with .html, write into out */
static void replace_ext(const char *path, char *out, size_t out_sz)
{
    strncpy(out, path, out_sz - 1);
    out[out_sz - 1] = '\0';
    char *dot = strrchr(out, '.');
    char *sep = strrchr(out, '/');
#ifdef _WIN32
    char *sep2 = strrchr(out, '\\');
    if (!sep || (sep2 && sep2 > sep))
        sep = sep2;
#endif
    if (dot && (!sep || dot > sep))
        strcpy(dot, ".html");
    else
        strncat(out, ".html", out_sz - strlen(out) - 1);
}

/* Get filename without extension for fallback title */
static void stem(const char *path, char *out, size_t out_sz)
{
    const char *base = path;
    const char *sep = strrchr(path, '/');
#ifdef _WIN32
    const char *sep2 = strrchr(path, '\\');
    if (!sep || (sep2 && sep2 > sep))
        sep = sep2;
#endif
    if (sep)
        base = sep + 1;
    strncpy(out, base, out_sz - 1);
    out[out_sz - 1] = '\0';
    char *dot = strrchr(out, '.');
    if (dot)
        *dot = '\0';
}

/* Exe directory */
static void exe_dir(const char *argv0, char *out, size_t out_sz)
{
    strncpy(out, argv0, out_sz - 1);
    out[out_sz - 1] = '\0';
    char *sep = strrchr(out, '/');
#ifdef _WIN32
    char *sep2 = strrchr(out, '\\');
    if (!sep || (sep2 && sep2 > sep))
        sep = sep2;
#endif
    if (sep)
        *sep = '\0';
    else
        strcpy(out, ".");
}

/*  HTML Shell */

static char *build_html(const char *title, const char *css, const char *body)
{
    Buf h;
    buf_init(&h);
    buf_puts(&h, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    buf_puts(&h, "  <meta charset=\"UTF-8\" />\n");
    buf_puts(&h, "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n");
    buf_puts(&h, "  <title>");
    buf_escape(&h, title, strlen(title));
    buf_puts(&h, "</title>\n");
    if (css && css[0])
    {
        buf_puts(&h, "  <style>\n");
        buf_puts(&h, css);
        buf_puts(&h, "\n  </style>\n");
    }
    buf_puts(&h, "</head>\n<body>\n");
    buf_puts(&h, body);
    buf_puts(&h, "</body>\n</html>\n");
    return h.data;
}

/*  Help */

static void print_help(void)
{
    puts(
        "\n"
        "  Mint Infrastructure - Markdown Compiler\n"
        "\n"
        "  Usage:\n"
        "    mintif <input.md> -o <output.html>\n"
        "    mintif <input.md>                     auto-names output\n"
        "    mintif <input.md> -t \"Custom Title\"\n"
        "    mintif <input.md> -s custom.css\n"
        "\n"
        "  Options:\n"
        "    -o, --output <file>   Output HTML file\n"
        "    -t, --title  <text>   Custom <title> tag\n"
        "    -s, --styles <file>   CSS file (default: styles.css next to exe)\n"
        "    -h, --help            Show this help\n");
}

/*  Main */

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 0;
    }

    const char *input_file = NULL;
    const char *output_file = NULL;
    const char *custom_title = NULL;
    const char *styles_file = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            print_help();
            return 0;
        }
        else if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) && i + 1 < argc)
        {
            output_file = argv[++i];
        }
        else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--title")) && i + 1 < argc)
        {
            custom_title = argv[++i];
        }
        else if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--styles")) && i + 1 < argc)
        {
            styles_file = argv[++i];
        }
        else if (argv[i][0] != '-')
        {
            input_file = argv[i];
        }
    }

    if (!input_file)
    {
        fprintf(stderr, "  x  No input file.\n");
        return 1;
    }

    /* Output path */
    char out_path[1024];
    if (output_file)
    {
        strncpy(out_path, output_file, sizeof(out_path) - 1);
    }
    else
    {
        replace_ext(input_file, out_path, sizeof(out_path));
    }

    /* CSS path — default: styles.css next to exe */
    char css_path[1024];
    if (styles_file)
    {
        strncpy(css_path, styles_file, sizeof(css_path) - 1);
    }
    else
    {
        char dir[1024];
        exe_dir(argv[0], dir, sizeof(dir));
        snprintf(css_path, sizeof(css_path), "%s/styles.css", dir);
    }

    /* Read markdown */
    size_t md_len;
    char *markdown = read_file(input_file, &md_len);
    if (!markdown)
    {
        fprintf(stderr, "  x  Cannot open: %s\n", input_file);
        return 1;
    }

    /* Read CSS */
    char *css = read_file(css_path, NULL);
    if (!css)
    {
        fprintf(stderr, "  !  No styles.css at %s — no styles applied.\n", css_path);
        css = (char *)calloc(1, 1);
    }

    /* Title */
    char title[512];
    char fallback[256];
    stem(input_file, fallback, sizeof(fallback));
    if (custom_title)
    {
        strncpy(title, custom_title, sizeof(title) - 1);
    }
    else
    {
        extract_title(markdown, title, sizeof(title), fallback);
    }

    /* Compile */
    char *body = parse_markdown(markdown, md_len);
    char *html = build_html(title, css, body);

    /* Write */
    if (!write_file(out_path, html, strlen(html)))
    {
        fprintf(stderr, "  x  Cannot write: %s\n", out_path);
        free(markdown);
        free(css);
        free(body);
        free(html);
        return 1;
    }

    /* Get just filenames for output message */
    const char *in_name = strrchr(input_file, '/');
    const char *out_name = strrchr(out_path, '/');
#ifdef _WIN32
    const char *in_name2 = strrchr(input_file, '\\');
    const char *out_name2 = strrchr(out_path, '\\');
    if (!in_name || (in_name2 && in_name2 > in_name))
        in_name = in_name2;
    if (!out_name || (out_name2 && out_name2 > out_name))
        out_name = out_name2;
#endif
    printf("  ok  %s -> %s\n",
           in_name ? in_name + 1 : input_file,
           out_name ? out_name + 1 : out_path);

    free(markdown);
    free(css);
    free(body);
    free(html);
    return 0;
}