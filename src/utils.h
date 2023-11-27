#ifndef UTILS_H_
#define UTILS_H_
#include "constants.h"

err_t utils_write_binary(char *path, void *content, uint32_t size, uint32_t n);
err_t utils_read_binary(char *path, void *content, uint32_t size, uint32_t n);
err_t utils_string_to_long(const char *str, long *num);

#endif
