#include "inline.h"
#include <ctype.h>

static int is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static int is_allowed_scheme(const char *s, size_t len) {
    if (len == 4 && tolower((unsigned char)s[0]) == 'h' &&
        tolower((unsigned char)s[1]) == 't' &&
        tolower((unsigned char)s[2]) == 't' &&
        tolower((unsigned char)s[3]) == 'p') return 1;

    if (len == 5 && tolower((unsigned char)s[0]) == 'h' &&
        tolower((unsigned char)s[1]) == 't' &&
        tolower((unsigned char)s[2]) == 't' &&
        tolower((unsigned char)s[3]) == 'p' &&
        tolower((unsigned char)s[4]) == 's') return 1;

    if (len == 6 && tolower((unsigned char)s[0]) == 'm' &&
        tolower((unsigned char)s[1]) == 'a' &&
        tolower((unsigned char)s[2]) == 'i' &&
        tolower((unsigned char)s[3]) == 'l' &&
        tolower((unsigned char)s[4]) == 't' &&
        tolower((unsigned char)s[5]) == 'o') return 1;

    if (len == 3 && tolower((unsigned char)s[0]) == 't' &&
        tolower((unsigned char)s[1]) == 'e' &&
        tolower((unsigned char)s[2]) == 'l') return 1;

    if (len == 3 && tolower((unsigned char)s[0]) == 'f' &&
        tolower((unsigned char)s[1]) == 't' &&
        tolower((unsigned char)s[2]) == 'p') return 1;

    return 0;
}

static int sanitize_url(const char *url, size_t url_len, const char **out_ptr, size_t *out_len) {
    size_t start = 0;
    size_t end = url_len;

    while (start < end && is_ws(url[start])) start++;
    while (end > start && is_ws(url[end - 1])) end--;
    if (end <= start) return 0;

    for (size_t i = start; i < end; i++) {
        unsigned char c = (unsigned char)url[i];
        if (c < 0x20 || c == 0x7f) return 0;
    }

    size_t i = start;
    while (i < end && url[i] != ':' && url[i] != '/' && url[i] != '?' && url[i] != '#') i++;
    if (i < end && url[i] == ':') {
        if (!is_allowed_scheme(url + start, i - start)) return 0;
    }

    *out_ptr = url + start;
    *out_len = end - start;
    return 1;
}

void parse_inline(Buf *out, const char *s, size_t len) {
    size_t i = 0;
    while (i < len) {

        /* HTML escape */
        if (s[i] == '&') { buf_puts(out, "&amp;");  i++; continue; }
        if (s[i] == '<') { buf_puts(out, "&lt;");   i++; continue; }
        if (s[i] == '>') { buf_puts(out, "&gt;");   i++; continue; }

        /* ![alt](url) */
        if (s[i] == '!' && i + 1 < len && s[i + 1] == '[') {
            size_t a = i + 2, a_end = i + 2;
            while (a_end < len && s[a_end] != ']') a_end++;
            if (a_end < len && a_end + 1 < len && s[a_end + 1] == '(') {
                size_t u = a_end + 2, u_end = a_end + 2;
                while (u_end < len && s[u_end] != ')') u_end++;
                if (u_end < len) {
                    const char *safe_url = NULL;
                    size_t safe_len = 0;
                    if (!sanitize_url(s + u, u_end - u, &safe_url, &safe_len)) {
                        buf_append(out, &s[i], 1);
                        i++;
                        continue;
                    }
                    buf_puts(out, "<img alt=\"");
                    buf_escape(out, s + a, a_end - a);
                    buf_puts(out, "\" src=\"");
                    buf_escape(out, safe_url, safe_len);
                    buf_puts(out, "\">");
                    i = u_end + 1;
                    continue;
                }
            }
        }

        /* [text](url) */
        if (s[i] == '[') {
            size_t t = i + 1, t_end = i + 1;
            while (t_end < len && s[t_end] != ']') t_end++;
            if (t_end < len && t_end + 1 < len && s[t_end + 1] == '(') {
                size_t u = t_end + 2, u_end = t_end + 2;
                while (u_end < len && s[u_end] != ')') u_end++;
                if (u_end < len) {
                    const char *safe_url = NULL;
                    size_t safe_len = 0;
                    if (!sanitize_url(s + u, u_end - u, &safe_url, &safe_len)) {
                        buf_append(out, &s[i], 1);
                        i++;
                        continue;
                    }
                    buf_puts(out, "<a href=\"");
                    buf_escape(out, safe_url, safe_len);
                    buf_puts(out, "\">");
                    parse_inline(out, s + t, t_end - t);
                    buf_puts(out, "</a>");
                    i = u_end + 1;
                    continue;
                }
            }
        }

        /* ***bold+italic*** */
        if (i + 2 < len && s[i] == '*' && s[i+1] == '*' && s[i+2] == '*') {
            size_t e = i + 3;
            while (e + 2 < len && !(s[e] == '*' && s[e+1] == '*' && s[e+2] == '*')) e++;
            if (e + 2 < len) {
                buf_puts(out, "<strong><em>");
                parse_inline(out, s + i + 3, e - (i + 3));
                buf_puts(out, "</em></strong>");
                i = e + 3; continue;
            }
        }

        /* **bold** */
        if (i + 1 < len && s[i] == '*' && s[i+1] == '*') {
            size_t e = i + 2;
            while (e + 1 < len && !(s[e] == '*' && s[e+1] == '*')) e++;
            if (e + 1 < len) {
                buf_puts(out, "<strong>");
                parse_inline(out, s + i + 2, e - (i + 2));
                buf_puts(out, "</strong>");
                i = e + 2; continue;
            }
        }

        /* *italic* */
        if (s[i] == '*' && (i == 0 || s[i-1] != '*')) {
            size_t e = i + 1;
            while (e < len && s[e] != '*') e++;
            if (e < len) {
                buf_puts(out, "<em>");
                parse_inline(out, s + i + 1, e - (i + 1));
                buf_puts(out, "</em>");
                i = e + 1; continue;
            }
        }

        /* _italic_ */
        if (s[i] == '_') {
            size_t e = i + 1;
            while (e < len && s[e] != '_') e++;
            if (e < len) {
                buf_puts(out, "<em>");
                parse_inline(out, s + i + 1, e - (i + 1));
                buf_puts(out, "</em>");
                i = e + 1; continue;
            }
        }

        /* ~~strikethrough~~ */
        if (i + 1 < len && s[i] == '~' && s[i+1] == '~') {
            size_t e = i + 2;
            while (e + 1 < len && !(s[e] == '~' && s[e+1] == '~')) e++;
            if (e + 1 < len) {
                buf_puts(out, "<del>");
                parse_inline(out, s + i + 2, e - (i + 2));
                buf_puts(out, "</del>");
                i = e + 2; continue;
            }
        }

        /* `code` */
        if (s[i] == '`') {
            size_t e = i + 1;
            while (e < len && s[e] != '`') e++;
            if (e < len) {
                buf_puts(out, "<code>");
                buf_escape(out, s + i + 1, e - (i + 1));
                buf_puts(out, "</code>");
                i = e + 1; continue;
            }
        }

        buf_append(out, &s[i], 1);
        i++;
    }
}