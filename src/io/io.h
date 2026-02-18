#ifndef IO_H
#define IO_H

#include <stddef.h>

/* Returns heap-allocated string, caller must free(). Returns NULL on error. */
char *io_read_file(const char *path, size_t *out_len);

/* Returns 1 on success, 0 on error. */
int io_write_file(const char *path, const char *data, size_t len);

#endif