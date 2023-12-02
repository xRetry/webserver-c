#include "constants.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

err_t utils_string_to_long(const char *str, long *num) {
    errno = 0;
    char *endptr = NULL;
    *num = strtol(str, &endptr, 10);
    if (errno == 0 && str && !*endptr) {
        return 0;
    } else if (errno == 0 && str && *endptr != 0) {
        return 0;
    }
    return 1;
}

err_t utils_string_to_double(const char *str, double *num) {
    errno = 0;
    char *endptr = NULL;
    *num = strtod(str, &endptr);
    if (errno == 0 && str && !*endptr) {
        return 0;
    } else if (errno == 0 && str && *endptr != 0) {
        return 0;
    }
    return 1;
}

err_t utils_write_binary(char *path, void *content, uint32_t size, uint32_t n) {
    FILE *fptr = fopen(path, "wb");
    if (fptr == NULL) {
        printf("Error opening file `%s`\n", path);
        return 1;
    }

    fwrite(content, size, n, fptr);
    fclose(fptr);
    return 0;
}

err_t utils_read_binary(char *path, void *content, uint32_t size, uint32_t n) {
    FILE *fptr = fopen(path, "rb");
    if (fptr == NULL) {
        printf("Error opening file `%s`\n", path);
        return 1;
    }

    int ret = fread(content, size, n, fptr);
    if (ret != NUM_PINS) {
        printf("Error reading file `%s`: %d\n", path, ret);
        return 1;
    }
    fclose(fptr);
    return 0;
}

void utils_array_to_json(char *str, char *arr[], int arr_len, int str_len) {
    strcat(str, "[");
    for (int i=0; i<arr_len; ++i) {
        strcat(str, arr[i]);

        char delimiter[] = ",";
        if (i == arr_len-1) { delimiter[0] = '\0'; }
        strcat(str, delimiter);
    }
    strcat(str, "]");
}



