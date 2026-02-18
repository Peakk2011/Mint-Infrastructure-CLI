#include "inline.h"

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
        if (s[i] == '[') {
            size_t t = i + 1, t_end = i + 1;
            while (t_end < len && s[t_end] != ']') t_end++;
            if (t_end < len && t_end + 1 < len && s[t_end + 1] == '(') {
                size_t u = t_end + 2, u_end = t_end + 2;
                while (u_end < len && s[u_end] != ')') u_end++;
                if (u_end < len) {
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