#ifndef FIST_ASSETS_PALETTE_H
#define FIST_ASSETS_PALETTE_H

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_PALETTE_COLORS = 256,
    FIST_PALETTE_CHANNELS = 3,
    FIST_PALETTE_SIZE = FIST_PALETTE_COLORS * FIST_PALETTE_CHANNELS,
    FIST_PALETTE_DAC_MAX = 63
};

typedef struct {
    /* Original VGA DAC components: R, G, B for each index, each in [0, 63]. */
    uint8_t rgb6[FIST_PALETTE_SIZE];
} fist_palette;

/* Decode exactly one original .PAL member. Return 0 on success, -1 for invalid
 * input. Failure leaves out unchanged; success copies the complete palette. */
int fist_palette_decode(const uint8_t *data, size_t size, fist_palette *out);

#endif
