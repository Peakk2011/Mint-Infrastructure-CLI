#include "path.h"
#include <string.h>

#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

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

void path_replace_ext(const char *path, char *out, size_t out_sz) {
    strncpy(out, path, out_sz - 1);
    out[out_sz - 1] = '\0';
    char *dot = strrchr(out, '.');
    char *sep = last_sep(out);
    if (dot && (!sep || dot > sep))
        strcpy(dot, ".html");
    else
        strncat(out, ".html", out_sz - strlen(out) - 1);
}

void path_stem(const char *path, char *out, size_t out_sz) {
    const char *base = path;
    const char *sep  = last_sep_c(path);
    if (sep) base = sep + 1;
    strncpy(out, base, out_sz - 1);
    out[out_sz - 1] = '\0';
    char *dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}

void path_exe_dir(const char *argv0, char *out, size_t out_sz) {
    strncpy(out, argv0, out_sz - 1);
    out[out_sz - 1] = '\0';
    char *sep = last_sep(out);
    if (sep) *sep = '\0';
    else     strcpy(out, ".");
}