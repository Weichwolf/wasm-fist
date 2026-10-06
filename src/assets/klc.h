#ifndef FIST_ASSETS_KLC_H
#define FIST_ASSETS_KLC_H

#include "assets/palette.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    /* Owned tightly packed row-major indices, in original decoded row order. */
    uint8_t *pixels;
    /* Embedded RGB bytes, preserved without DAC scaling or palette remapping. */
    uint8_t palette[FIST_PALETTE_SIZE];
} fist_klc_image;

/* Decode an entire KLC1 file, including its palette. Return 0 on success,
 * -1 for invalid/incomplete input or allocation failure. Failure leaves out
 * unchanged. On success out owns pixels; it must not already own an image.
 * The decoded image does not borrow any input bytes. */
int fist_klc_decode(const uint8_t *data, size_t size, fist_klc_image *out);
/* Free pixels and reset every member. NULL and empty images are permitted. */
void fist_klc_destroy(fist_klc_image *image);

#endif
