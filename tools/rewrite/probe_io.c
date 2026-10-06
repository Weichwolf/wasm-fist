#include "probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Read through EOF rather than guessing an asset's size or truncating a prefix. */
uint8_t *fist_probe_read_file(FILE *file, size_t *size) {
    enum { INITIAL_CAPACITY = 1024 };
    size_t capacity = INITIAL_CAPACITY;
    uint8_t *data = malloc(capacity);
    *size = 0;
    if (data == NULL) {
        return NULL;
    }
    for (;;) {
        *size += fread(data + *size, 1, capacity - *size, file);
        if (ferror(file) != 0) {
            free(data);
            return NULL;
        }
        if (feof(file) != 0) {
            return data;
        }
        if (capacity > SIZE_MAX / 2) {
            free(data);
            return NULL;
        }
        capacity *= 2;
        uint8_t *grown = realloc(data, capacity);
        if (grown == NULL) {
            free(data);
            return NULL;
        }
        data = grown;
    }
}
