#ifndef PATH_H
#define PATH_H

#include <stddef.h>

/* Replace file extension with .html */
void path_replace_ext(
    const char *path,
    char *out,
    size_t out_sz
);

/* Get filename without extension (for fallback title) */
void path_stem(
    const char *path,
    char *out,
    size_t out_sz
);

/* Get directory of the executable */
void path_exe_dir(
    const char *argv0,
    char *out,
    size_t out_sz
);

#endif