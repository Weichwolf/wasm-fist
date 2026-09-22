#ifndef FIST_SEQUENCE_CAPTURE_H
#define FIST_SEQUENCE_CAPTURE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct FistSequenceState {
    FILE *file;
    uint64_t records;
    uint64_t samples;
};

static FistSequenceState fist_sequence;

static inline void fist_sequence_fail(const char *message) {
    fprintf(stderr, "FIST_SEQUENCE: %s\n", message);
    abort();
}

static inline void fist_sequence_byte(unsigned value) {
    if (fputc(value, fist_sequence.file) == EOF) fist_sequence_fail("write failed");
}

static inline void fist_sequence_u32(uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) fist_sequence_byte(value >> (8 * i));
}

static inline void fist_sequence_u64(uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) fist_sequence_byte(value >> (8 * i));
}

static inline void fist_sequence_bytes(const void *data, size_t size) {
    if (fwrite(data, 1, size, fist_sequence.file) != size) fist_sequence_fail("write failed");
}

static inline void fist_sequence_close() {
    if (!fist_sequence.file) return;
    fist_sequence_byte('E');
    fist_sequence_u64(fist_sequence.records);
    fist_sequence_u64(fist_sequence.samples);
    if (fclose(fist_sequence.file)) fist_sequence_fail("close failed");
    fist_sequence.file = NULL;
}

static inline bool fist_sequence_open(char kind) {
    const char *prefix = getenv("FIST_SEQUENCE");
    if (!prefix) return false;
    if (fist_sequence.file) return true;
    size_t length = strlen(prefix);
    if (length > SIZE_MAX - 8) fist_sequence_fail("path too long");
    char *path = (char *)malloc(length + 8);
    if (!path) fist_sequence_fail("path allocation failed");
    memcpy(path, prefix, length);
    strcpy(path + length, kind == 'F' ? ".frames" : ".pcm");
    fist_sequence.file = fopen(path, "wb");
    free(path);
    if (!fist_sequence.file) fist_sequence_fail("open failed");
    fist_sequence_bytes("FISTSEQ1", 8);
    fist_sequence_byte(kind);
    if (atexit(fist_sequence_close)) fist_sequence_fail("atexit registration failed");
    return true;
}

static inline uint64_t fist_sequence_time(double milliseconds) {
    if (milliseconds < 0) fist_sequence_fail("negative time");
    return (uint64_t)(milliseconds * 1000.0 + 0.5);
}

static inline void fist_sequence_frame(double milliseconds, uint32_t width, uint32_t height,
                                       uint32_t pitch, const uint8_t *pixels, const uint8_t *palette) {
    if (!fist_sequence_open('F')) return;
    if (pitch < width) fist_sequence_fail("frame pitch shorter than width");
    fist_sequence_byte('F');
    fist_sequence_u64(fist_sequence_time(milliseconds));
    fist_sequence_u32(width);
    fist_sequence_u32(height);
    for (unsigned index = 0; index < 256; ++index)
        fist_sequence_bytes(palette + index * 4, 3);
    for (uint32_t row = 0; row < height; ++row)
        fist_sequence_bytes(pixels + (size_t)row * pitch, width);
    ++fist_sequence.records;
}

static inline void fist_sequence_audio(double milliseconds, uint32_t rate,
                                       const int16_t *stereo, uint32_t frames) {
    if (!frames || !fist_sequence_open('A')) return;
    fist_sequence_byte('A');
    fist_sequence_u64(fist_sequence_time(milliseconds));
    fist_sequence_u64(fist_sequence.samples);
    fist_sequence_u32(rate);
    fist_sequence_u32(frames);
    fist_sequence_bytes(stereo, (size_t)frames * 4);
    fist_sequence.samples += frames;
    ++fist_sequence.records;
}

#endif
