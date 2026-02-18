#include "io.h"
#include <stdio.h>
#include <stdlib.h>

char *io_read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size_t sz = (size_t)ftell(f);
    rewind(f);
    char *buf = (char *)malloc(sz + 1);
    size_t r = fread(buf, 1, sz, f); (void)r;
    buf[sz] = '\0';
    fclose(f);
    if (out_len) *out_len = sz;
    return buf;
}

int io_write_file(const char *path, const char *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(data, 1, len, f);
    fclose(f);
    return 1;
}