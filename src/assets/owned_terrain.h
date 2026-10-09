#ifndef FIST_ASSETS_OWNED_TERRAIN_H
#define FIST_ASSETS_OWNED_TERRAIN_H

#include <stddef.h>
#include <stdint.h>

/* Owned square, periodic authoring planes. Rows increase in map Y. No palette,
 * original-format image, source view or platform storage is retained. */
typedef struct {
    uint32_t side;
    double minimum;
    double maximum;
    double water_level;
    uint16_t *heights;
    uint8_t *colors;
} fist_owned_terrain;

/* Decode the complete version-1 FMAP bundle described in docs/owned-map-format.md.
 * Check dimensions, finite metadata, exact length and CRC before allocation.
 * Failure preserves out. Success owns independent height/RGB planes; out must
 * not already own a terrain. All 16 height bits and RGB bytes are preserved. */
int fist_owned_terrain_decode(const uint8_t *data, size_t size, fist_owned_terrain *out);
void fist_owned_terrain_destroy(fist_owned_terrain *terrain);

/* Height on the two rendered cell triangles, with diagonal top-right to
 * bottom-left. Coordinates are normalized authoring X/Y; both axes wrap.
 * Finite coordinates are required. Return -1 and preserve height on failure.
 * This is the shared geometry sampling owner for owned maps. World-coordinate
 * conversion belongs to the existing world contract, not the asset decoder. */
int fist_owned_terrain_surface(const fist_owned_terrain *terrain, double map_x, double map_y,
                               double *height);

#endif
