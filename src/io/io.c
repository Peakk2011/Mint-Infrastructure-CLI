#include "io.h"
#include <stdio.h>
#include <stdlib.h>

char *io_read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long end_pos = ftell(f);
    
    if (end_pos < 0) {
        fclose(f);
        return NULL;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    size_t sz = (size_t)end_pos;
    char *buf = (char *)malloc(sz + 1);

    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t r = fread(buf, 1, sz, f);
    
    if (r != sz) {
        free(buf);
        fclose(f);
        return NULL;
    }

    buf[sz] = '\0';
    
    if (fclose(f) != 0) {
        free(buf);
        return NULL;
    }

    if (out_len) *out_len = sz;
    return buf;
}

int io_write_file(const char *path, const char *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    size_t w = fwrite(data, 1, len, f);
    int close_ok = fclose(f) == 0;
    
    if (w != len || !close_ok) return 0;
    
    return 1;
}