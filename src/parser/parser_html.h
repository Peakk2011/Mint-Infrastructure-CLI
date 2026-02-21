#ifndef PARSER_HTML_H
#define PARSER_HTML_H

#include <stddef.h>

int parse_html_tag_line(const char *ptr, size_t len, char *name_out, size_t name_out_sz, int *is_closing, int *is_self_close);
int contains_closing_html_tag(const char *ptr, size_t len, const char *name);
int is_void_html_tag(const char *name, size_t len);

#endif