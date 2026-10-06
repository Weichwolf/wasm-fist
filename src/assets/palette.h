#ifndef FIST_ASSETS_PALETTE_H
#define FIST_ASSETS_PALETTE_H

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_PALETTE_COLORS = 256,
    FIST_PALETTE_CHANNELS = 3,
    FIST_PALETTE_SIZE = FIST_PALETTE_COLORS * FIST_PALETTE_CHANNELS,
    FIST_PALETTE_DAC_MAX = 63,
    /* Original engine db47: the normal mission terrain band begins at 0x50. */
    FIST_TERRAIN_PALETTE_START = 80
};

typedef struct {
    /* Original VGA DAC components: R, G, B for each index, each in [0, 63]. */
    uint8_t rgb6[FIST_PALETTE_SIZE];
} fist_palette;

typedef struct {
    uint8_t indices[FIST_PALETTE_COLORS];
} fist_palette_map;

/* Decode exactly one original .PAL member. Return 0 on success, -1 for invalid
 * input. Failure leaves out unchanged; success copies the complete palette. */
int fist_palette_decode(const uint8_t *data, size_t size, fist_palette *out);

/* Original mission preparation: preserve the reserved prefix and selection-sort
 * the terrain band by R + 2G + B. Input/output may alias. */
int fist_palette_prepare(const fist_palette *source, fist_palette *out);
/* Map a complete embedded KLC RGB8 palette into the prepared mission palette.
 * Index zero stays zero. Other indices search the terrain band using original
 * quantization, weighted distance and first-minimum tie handling. */
int fist_palette_build_map(const uint8_t rgb8[FIST_PALETTE_SIZE], const fist_palette *mission,
                           fist_palette_map *out);

#endif
