#ifndef FIST_ASSETS_TERRAIN_H
#define FIST_ASSETS_TERRAIN_H

#include "assets/klc.h"
#include "assets/palette.h"
#include "assets/scenario.h"
#include "assets/source.h"

typedef struct {
    /* Owned raw decoded planes; embedded palettes and source indices retained. */
    fist_klc_image heightmap;
    fist_klc_image colormap;
    fist_klc_image sky;
    fist_palette palette;
    fist_palette_map colormap_map;
    fist_palette_map sky_map;
} fist_terrain;

/* Resolve the four scenario asset names through source, decode all required
 * inputs and prepare the original mission palette maps. Height and color planes
 * must be square powers of two, as required by the original wrapped samplers.
 * Return 0 on success, -1 on any missing/invalid input, I/O or allocation error.
 * Failure leaves out unchanged. On success out owns its planes and must not
 * already own terrain. No input/source views are retained after this call. */
int fist_terrain_load(const fist_scenario *scenario, const fist_asset_source *source,
                      fist_terrain *out);
void fist_terrain_destroy(fist_terrain *terrain);

#endif
