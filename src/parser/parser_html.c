#include "parser_html.h"
#include "parser_line.h"
#include <string.h>
#include <ctype.h>

/* Case-insensitive compare against string literals. */
static int ascii_ieq_n(const char *a, size_t a_len, const char *b) {
    size_t i = 0;

    while (i < a_len && b[i]) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return 0;
        i++;
    }
    
    return i == a_len && b[i] == '\0';
}

/* HTML identifier character policy used by this parser. */
static int is_name_char(unsigned char c) {
    return isalnum(c) || c == '-' || c == '_' || c == ':';
}

/* Space characters accepted inside HTML tag parsing in block mode. */
static int is_tag_ws(char c) {
    return c == ' ' || c == '\t';
}

/* Allowed HTML tags that can start raw block handling. */
static int is_block_html_tag(const char *name, size_t len) {
    static const char *tags[] = {
        "p", "img", "h1", "h2", "h3", "h4", "h5", "h6",
        "div", "blockquote", "ul", "ol", "li",
        "table", "thead", "tbody", "tfoot", "tr", "th", "td", "caption",
        "pre", "center", "font", "hr"
    };

    for (size_t i = 0; i < sizeof(tags) / sizeof(tags[0]); i++) {
        if (ascii_ieq_n(name, len, tags[i])) return 1;
    }
    return 0;
}

/* Public helper: true for tags that are self-contained by definition. */
int is_void_html_tag(const char *name, size_t len) {
    return ascii_ieq_n(name, len, "img") ||
           ascii_ieq_n(name, len, "hr")  ||
           ascii_ieq_n(name, len, "br");
}

/* Parse one attribute name token while validating syntax. */
static int parse_attr_name(const char *t, size_t limit, size_t *i) {
    size_t start = *i;
    
    while (*i < limit && is_name_char((unsigned char)t[*i])) (*i)++;
    return *i > start;
}

/* Parse one attribute value after '=' (quoted or unquoted). */
static int parse_attr_value(const char *t, size_t limit, size_t *i) {
    while (*i < limit && is_tag_ws(t[*i])) (*i)++;
    if (*i >= limit) return 0;

    if (t[*i] == '"' || t[*i] == '\'') {
        char quote = t[(*i)++];
        while (*i < limit && t[*i] != quote) {
            if (t[*i] == '<') return 0;
            (*i)++;
        }
        if (*i >= limit) return 0;
        (*i)++;
        return 1;
    }

    size_t val_start = *i;
    
    while (*i < limit && !is_tag_ws(t[*i]) && t[*i] != '>') {
        if (t[*i] == '<' || t[*i] == '"' || t[*i] == '\'' || t[*i] == '=') return 0;
        (*i)++;
    }
    
    return *i > val_start;
}

/*
 * Validate whether a line starts with a well-formed HTML tag usable as a block.
 * The function validates the first tag token only, which lets lines such as
 * <p>inline text</p> qualify as block HTML starts.
 */
int parse_html_tag_line(const char *ptr, size_t len, char *name_out, size_t name_out_sz, int *is_closing, int *is_self_close) {
    size_t tlen = 0;
    const char *t = ltrim(ptr, len, &tlen);

    if (tlen < 3 || t[0] != '<') return 0;
    if (tlen >= 4 && t[1] == '!' && t[2] == '-' && t[3] == '-') return 0;

    size_t tag_end = 0;
    char quote = 0;
    
    for (size_t k = 1; k < tlen; k++) {
        if (quote) {
            if (t[k] == quote) quote = 0;
            continue;
        }
        if (t[k] == '"' || t[k] == '\'') {
            quote = t[k];
            continue;
        }
        if (t[k] == '>') {
            tag_end = k;
            break;
        }
    }
    
    if (tag_end == 0) return 0;

    *is_closing = 0;
    *is_self_close = 0;

    size_t i = 1;
    
    if (i < tag_end && t[i] == '/') {
        *is_closing = 1;
        i++;
    }

    size_t tag_start = i;
    while (i < tag_end && is_name_char((unsigned char)t[i])) i++;
    size_t tag_len = i - tag_start;

    if (tag_len == 0) return 0;
    if (!is_block_html_tag(t + tag_start, tag_len)) return 0;

    if (name_out_sz > 0) {
        size_t copy_len = tag_len;
        if (copy_len >= name_out_sz) copy_len = name_out_sz - 1;
        memcpy(name_out, t + tag_start, copy_len);
        name_out[copy_len] = '\0';
    }

    size_t content_limit = tag_end;
    size_t j = content_limit;
    
    while (j > 0 && is_tag_ws(t[j - 1])) j--;
    
    if (j > 0 && t[j - 1] == '/') {
        *is_self_close = 1;
        content_limit = j - 1;
    
        while (content_limit > 0 && is_tag_ws(t[content_limit - 1])) content_limit--;
    }

    while (i < content_limit && is_tag_ws(t[i])) i++;
    
    if (*is_closing) {
        return !*is_self_close && i == content_limit;
    }

    while (i < content_limit) {
        while (i < content_limit && is_tag_ws(t[i])) i++;
        if (i >= content_limit) break;

        if (t[i] == '<') return 0;
        if (!parse_attr_name(t, content_limit, &i)) return 0;

        while (i < content_limit && is_tag_ws(t[i])) i++;
        if (i >= content_limit || t[i] != '=') return 0;
        i++;

        if (!parse_attr_value(t, content_limit, &i)) return 0;
    }

    return 1;
}

/* Check if a line contains a closing token for a given tag (e.g. </div>). */
int contains_closing_html_tag(const char *ptr, size_t len, const char *name) {
    size_t name_len = strlen(name);

    for (size_t i = 0; i + 3 + name_len <= len; i++) {
        if (ptr[i] != '<' || ptr[i + 1] != '/') continue;

        size_t j = 0;
 
        while (j < name_len && i + 2 + j < len &&
               tolower((unsigned char)ptr[i + 2 + j]) == tolower((unsigned char)name[j])) {
            j++;
        }
 
        if (j == name_len && i + 2 + j < len &&
            !is_name_char((unsigned char)ptr[i + 2 + j])) {
            return 1;
        }
    }

    return 0;
}