#ifndef FIST_ASSETS_BYTES_H
#define FIST_ASSETS_BYTES_H

#include <stdint.h>

/* Callers must first prove that the complete word is in their input view. */
static inline uint16_t fist_read_u16le(const uint8_t *data) {
    enum { BYTE_BITS = 8 };
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << BYTE_BITS));
}

static inline uint32_t fist_read_u32le(const uint8_t *data) {
    enum { DWORD_SIZE = 4, BYTE_BITS = 8 };
    uint32_t value = 0;
    for (unsigned index = 0; index < DWORD_SIZE; ++index) {
        value |= (uint32_t)data[index] << (index * BYTE_BITS);
    }
    return value;
}

#endif
