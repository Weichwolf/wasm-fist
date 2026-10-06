#ifndef FIST_ASSETS_HEIGHTFIELD_H
#define FIST_ASSETS_HEIGHTFIELD_H

#include "assets/klc.h"

#include <stdint.h>

/* Original height-field bc06/bed2 resizing. Input is a complete square plane;
 * target_side differs by a power-of-two ratio, or equals the original side.
 * Doubling averages wrapped columns, then wrapped rows, rounding down at each
 * stage. Halving selects even rows/columns. Palette bytes remain unchanged.
 * Return 0 on success, -1 on invalid/overflow/allocation input, preserving out.
 * Success owns new pixels independently of source, even at unchanged size.
 * source and out must be distinct; out must not already own an image. */
int fist_heightfield_resample(const fist_klc_image *source, uint32_t target_side,
                              fist_klc_image *out);

#endif
