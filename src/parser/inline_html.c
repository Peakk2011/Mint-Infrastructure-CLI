#include "inline_html.h"
#include <ctype.h>
#include <string.h>

/* Generic ASCII whitespace helper used in URL and tag parsing. */
static int is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

/* Allow only conservative URL schemes for rendered links/resources. */
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

/* Case-insensitive compare for a span and string literal. */
static int ascii_ieq(const char *a, size_t a_len, const char *b) {
    size_t b_len = strlen(b);
    if (a_len != b_len) return 0;
    for (size_t i = 0; i < a_len; i++) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return 0;
    }
    return 1;
}

/* HTML tag/attribute name character policy for inline raw HTML support. */
static int is_name_char(unsigned char c) {
    return isalnum(c) || c == '-' || c == '_' || c == ':';
}

/* URL sanitizer used by markdown links/images and raw HTML href/src attributes. */
int inline_sanitize_url(const char *url, size_t url_len, const char **out_ptr, size_t *out_len) {
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

/* Allowed inline HTML tags. */
static int is_allowed_html_tag(const char *name, size_t len) {
    static const char *tags[] = {
        "a", "p", "img",
        "h1", "h2", "h3", "h4", "h5", "h6",
        "div", "span", "br", "hr", "blockquote",
        "b", "strong", "i", "em", "u", "s", "strike", "del",
        "small", "big", "sub", "sup", "tt",
        "code", "pre",
        "ul", "ol", "li",
        "table", "thead", "tbody", "tfoot", "tr", "th", "td", "caption",
        "center", "font"
    };

    for (size_t i = 0; i < sizeof(tags) / sizeof(tags[0]); i++) {
        if (ascii_ieq(name, len, tags[i])) return 1;
    }
    return 0;
}

/* Allowed attributes per tag. Unknown/unsafe attrs are dropped. */
static int is_allowed_html_attr(const char *tag, size_t tag_len, const char *attr, size_t attr_len) {
    if (attr_len >= 2 &&
        tolower((unsigned char)attr[0]) == 'o' &&
        tolower((unsigned char)attr[1]) == 'n') {
        return 0;
    }

    if (ascii_ieq(attr, attr_len, "id") ||
        ascii_ieq(attr, attr_len, "class") ||
        ascii_ieq(attr, attr_len, "title") ||
        ascii_ieq(attr, attr_len, "align") ||
        ascii_ieq(attr, attr_len, "valign") ||
        ascii_ieq(attr, attr_len, "width") ||
        ascii_ieq(attr, attr_len, "height") ||
        ascii_ieq(attr, attr_len, "bgcolor") ||
        ascii_ieq(attr, attr_len, "border")) {
        return 1;
    }

    if (ascii_ieq(tag, tag_len, "a")) {
        return ascii_ieq(attr, attr_len, "href") ||
               ascii_ieq(attr, attr_len, "target") ||
               ascii_ieq(attr, attr_len, "rel") ||
               ascii_ieq(attr, attr_len, "name");
    }
    if (ascii_ieq(tag, tag_len, "img")) {
        return ascii_ieq(attr, attr_len, "src") ||
               ascii_ieq(attr, attr_len, "alt");
    }
    if (ascii_ieq(tag, tag_len, "table")) {
        return ascii_ieq(attr, attr_len, "cellpadding") ||
               ascii_ieq(attr, attr_len, "cellspacing");
    }
    if (ascii_ieq(tag, tag_len, "td") || ascii_ieq(tag, tag_len, "th")) {
        return ascii_ieq(attr, attr_len, "colspan") ||
               ascii_ieq(attr, attr_len, "rowspan");
    }
    if (ascii_ieq(tag, tag_len, "ol")) {
        return ascii_ieq(attr, attr_len, "start") ||
               ascii_ieq(attr, attr_len, "type");
    }
    if (ascii_ieq(tag, tag_len, "li")) {
        return ascii_ieq(attr, attr_len, "value");
    }
    if (ascii_ieq(tag, tag_len, "font")) {
        return ascii_ieq(attr, attr_len, "color") ||
               ascii_ieq(attr, attr_len, "size") ||
               ascii_ieq(attr, attr_len, "face");
    }

    return 0;
}

/* Parse attribute name token. */
static int parse_attr_name(const char *s, size_t len, size_t *i, size_t *name_start, size_t *name_len) {
    *name_start = *i;
    while (*i < len && is_name_char((unsigned char)s[*i])) (*i)++;
    *name_len = *i - *name_start;
    return *name_len > 0;
}

/* Parse optional attribute value (boolean attributes are allowed). */
static int parse_attr_value(const char *s, size_t len, size_t *i, const char **value_ptr, size_t *value_len, int *has_value) {
    *has_value = 0;
    *value_ptr = NULL;
    *value_len = 0;

    while (*i < len && is_ws(s[*i])) (*i)++;
    if (*i >= len || s[*i] != '=') return 1;

    *has_value = 1;
    (*i)++;
    while (*i < len && is_ws(s[*i])) (*i)++;
    if (*i >= len) return 0;

    if (s[*i] == '"' || s[*i] == '\'') {
        char quote = s[(*i)++];
        size_t start = *i;
        while (*i < len && s[*i] != quote) (*i)++;
        if (*i >= len) return 0;
        *value_ptr = s + start;
        *value_len = *i - start;
        (*i)++;
        return 1;
    }

    size_t start = *i;
    while (*i < len && !is_ws(s[*i]) && s[*i] != '>' &&
           !(s[*i] == '/' && *i + 1 < len && s[*i + 1] == '>')) {
        (*i)++;
    }
    if (*i == start) return 0;

    *value_ptr = s + start;
    *value_len = *i - start;
    return 1;
}

/* Emit one validated attribute into output. */
static void emit_attr(Buf *out,
                      const char *tag, size_t tag_len,
                      const char *attr, size_t attr_len,
                      int has_value,
                      const char *value_ptr, size_t value_len) {
    if (!is_allowed_html_attr(tag, tag_len, attr, attr_len)) return;

    if (!has_value) {
        buf_puts(out, " ");
        buf_append(out, attr, attr_len);
        return;
    }

    if (ascii_ieq(attr, attr_len, "href") || ascii_ieq(attr, attr_len, "src")) {
        const char *safe_url = NULL;
        size_t safe_len = 0;
        if (!inline_sanitize_url(value_ptr, value_len, &safe_url, &safe_len)) return;
        buf_puts(out, " ");
        buf_append(out, attr, attr_len);
        buf_puts(out, "=\"");
        buf_escape(out, safe_url, safe_len);
        buf_puts(out, "\"");
        return;
    }

    buf_puts(out, " ");
    buf_append(out, attr, attr_len);
    buf_puts(out, "=\"");
    buf_escape(out, value_ptr, value_len);
    buf_puts(out, "\"");
}

/* Parse and emit raw inline HTML tag. */
int inline_try_emit_html_tag(Buf *out, const char *s, size_t len, size_t *consumed) {
    size_t i = 0;
    if (len < 3 || s[0] != '<') return 0;

    if (len >= 4 && s[1] == '!' && s[2] == '-' && s[3] == '-') {
        size_t j = 4;
        while (j + 2 < len) {
            if (s[j] == '-' && s[j + 1] == '-' && s[j + 2] == '>') {
                buf_append(out, s, j + 3);
                *consumed = j + 3;
                return 1;
            }
            j++;
        }
        return 0;
    }

    i++;
    int closing = 0;
    if (i < len && s[i] == '/') {
        closing = 1;
        i++;
    }

    size_t tag_start = i;
    while (i < len && is_name_char((unsigned char)s[i])) i++;
    size_t tag_len = i - tag_start;
    if (tag_len == 0 || !is_allowed_html_tag(s + tag_start, tag_len)) return 0;

    while (i < len && is_ws(s[i])) i++;

    if (closing) {
        if (i >= len || s[i] != '>') return 0;
        buf_puts(out, "</");
        buf_append(out, s + tag_start, tag_len);
        buf_puts(out, ">");
        *consumed = i + 1;
        return 1;
    }

    buf_puts(out, "<");
    buf_append(out, s + tag_start, tag_len);

    while (i < len && s[i] != '>' && !(s[i] == '/' && i + 1 < len && s[i + 1] == '>')) {
        while (i < len && is_ws(s[i])) i++;
        if (i >= len || s[i] == '>' || (s[i] == '/' && i + 1 < len && s[i + 1] == '>')) break;

        size_t attr_start = 0, attr_len = 0;
        if (!parse_attr_name(s, len, &i, &attr_start, &attr_len)) return 0;

        const char *value_ptr = NULL;
        size_t value_len = 0;
        int has_value = 0;
        if (!parse_attr_value(s, len, &i, &value_ptr, &value_len, &has_value)) return 0;

        emit_attr(out,
                  s + tag_start, tag_len,
                  s + attr_start, attr_len,
                  has_value,
                  value_ptr, value_len);
    }

    if (i + 1 < len && s[i] == '/' && s[i + 1] == '>') {
        buf_puts(out, " />");
        *consumed = i + 2;
        return 1;
    }
    if (i < len && s[i] == '>') {
        buf_puts(out, ">");
        *consumed = i + 1;
        return 1;
    }

    return 0;
}