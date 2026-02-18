#include "html.h"
#include "../buf/buf.h"
#include <string.h>

char *html_build(const char *title, const char *css, const char *body) {
    Buf h;
    buf_init(&h);

    buf_puts(&h, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    buf_puts(&h, "  <meta charset=\"UTF-8\" />\n");
    buf_puts(&h, "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n");
    buf_puts(&h, "  <title>");
    buf_escape(&h, title, strlen(title));
    buf_puts(&h, "</title>\n");
    if (css && css[0]) {
        buf_puts(&h, "  <style>\n");
        buf_puts(&h, css);
        buf_puts(&h, "\n  </style>\n");
    }
    buf_puts(&h, "</head>\n<body>\n");
    buf_puts(&h, body);
    buf_puts(&h, "</body>\n</html>\n");

    return h.data;
}