#ifndef FIST_SEQUENCE_ENDPOINT_H
#define FIST_SEQUENCE_ENDPOINT_H

#include "fist_sequence_capture.h"

static inline uint64_t fist_sequence_end_ms(void) {
    static bool initialized;
    static uint64_t end;
    if (initialized) return end;
    initialized = true;
    const char *value = getenv("FIST_SEQUENCE_END_MS");
    if (!value) return 0;
    const char *prefix = getenv("FIST_SEQUENCE");
    if (!prefix || !*prefix || !*value) fist_sequence_fail("invalid endpoint configuration");
    for (; *value; ++value) {
        if (*value < '0' || *value > '9') fist_sequence_fail("invalid endpoint milliseconds");
        end = end * 10 + (unsigned)(*value - '0');
        if (end > UINT32_MAX) fist_sequence_fail("endpoint milliseconds overflow");
    }
    if (!end) fist_sequence_fail("endpoint must be positive");
    return end;
}

static inline void fist_sequence_endpoint_complete(void) {
    uint64_t end = fist_sequence_end_ms();
    if (!end) fist_sequence_fail("endpoint not configured");
    const char *prefix = getenv("FIST_SEQUENCE");
    size_t length = strlen(prefix);
    if (length > SIZE_MAX - 5) fist_sequence_fail("endpoint path too long");
    char *path = (char *)malloc(length + 5);
    if (!path) fist_sequence_fail("endpoint path allocation failed");
    memcpy(path, prefix, length);
    strcpy(path + length, ".end");
    FILE *file = fopen(path, "wb");
    free(path);
    if (!file) fist_sequence_fail("endpoint open failed");
    if (fprintf(file, "FISTEND1\n%llu\n", (unsigned long long)end) < 0 || fclose(file))
        fist_sequence_fail("endpoint write failed");
    fprintf(stderr, "FIST_SEQUENCE_END ms=%llu\n", (unsigned long long)end);
}

#endif
