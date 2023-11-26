#include "constants.h"
#include <stdint.h>
#include <stdio.h>

err_t utils_write_binary(char *path, void *content, uint32_t length) {
    FILE *fptr = fopen(path, "wb");
    if (fptr == NULL) {
        printf("Error opening config file\n");
        return 1;
    }

    fwrite(content, length, 1, fptr);
    fclose(fptr);
    return 0;
}

err_t utils_read_binary(char *path, void *content, uint32_t length) {
    FILE *fptr = fopen("config", "rb");
    if (fptr == NULL) {
        return 1;
    }

    int ret = fread(content, length, 1, fptr);
    if (ret != NUM_PINS) {
        printf("Error reading file: %d\n", ret);
        return 1;
    }
    fclose(fptr);
    return 0;
}

