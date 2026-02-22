#include "path.h"
#include <string.h>

static char *last_sep(char *s) {
    char *a = strrchr(s, '/');
#ifdef _WIN32
    char *b = strrchr(s, '\\');
    if (!a || (b && b > a)) a = b;
#endif
    return a;
}

static const char *last_sep_c(const char *s) {
    const char *a = strrchr(s, '/');
#ifdef _WIN32
    const char *b = strrchr(s, '\\');
    if (!a || (b && b > a)) a = b;
#endif
    return a;
}

static void copy_trunc(char *dst, size_t dst_sz, const char *src) {
    if (dst_sz == 0) return;
    strncpy(dst, src, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
}

void path_replace_ext(const char *path, char *out, size_t out_sz) {
    const char *ext = ".html";
    size_t ext_len = strlen(ext);

    const char *sep = last_sep_c(path);
    const char *dot = strrchr(path, '.');
    
    size_t base_len = strlen(path);

    if (out_sz == 0) return;
    if (dot && (!sep || dot > sep)) base_len = (size_t)(dot - path);

    size_t copy_len = base_len;
    if (copy_len > out_sz - 1) copy_len = out_sz - 1;

    memcpy(out, path, copy_len);

    size_t pos = copy_len;

    if (pos < out_sz - 1) {
        size_t remain = out_sz - pos - 1;
        size_t append_len = ext_len < remain ? ext_len : remain;

        memcpy(out + pos, ext, append_len);
        pos += append_len;
    }

    out[pos] = '\0';
}

void path_stem(const char *path, char *out, size_t out_sz) {
    if (out_sz == 0) return;
    
    const char *base = path;
    const char *sep  = last_sep_c(path);
    
    if (sep) base = sep + 1;
    
    copy_trunc(out, out_sz, base);
    char *dot = strrchr(out, '.');
    
    if (dot) *dot = '\0';
}

void path_exe_dir(const char *argv0, char *out, size_t out_sz) {
    if (out_sz == 0) return;
    
    copy_trunc(out, out_sz, argv0);
    char *sep = last_sep(out);
    
    if (sep) {
        *sep = '\0';
    } else {
        if (out_sz > 1) {
            out[0] = '.';
            out[1] = '\0';
        } else {
            out[0] = '\0';
        }
    }
}