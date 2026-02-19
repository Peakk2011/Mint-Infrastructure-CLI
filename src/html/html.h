#ifndef HTML_H
#define HTML_H

/* Build complete HTML document.
   Returns heap-allocated string, caller must free(). Returns NULL on error. */
char *html_build(const char *title, const char *css, const char *body);

#endif