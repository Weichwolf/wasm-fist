#include "assets/terrain.h"
#include "assets/klc.h"
#include "assets/palette.h"
#include "assets/resource.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>

enum { HEIGHT_NAME = 0, COLOR_NAME = 1, PALETTE_NAME = 2, SKY_NAME = 3 };

static int load_image(const fist_asset_source *source, const char *name, fist_klc_image *out) {
    fist_asset_view input = {0};
    if (source->read(source->context, name, &input) != FIST_ASSET_READ_OK) {
        return -1;
    }
    return fist_klc_decode(input.data, input.size, out);
}

static int load_palette(const fist_asset_source *source, const char *name, fist_palette *out) {
    fist_asset_view input = {0};
    const fist_asset_read_result result = source->read(source->context, name, &input);
    if (result == FIST_ASSET_READ_NOT_FOUND) {
        if (source->read(source->context, "PAL.RES", &input) != FIST_ASSET_READ_OK ||
            fist_resource_find(input.data, input.size, name, &input) != 0) {
            return -1;
        }
    } else if (result != FIST_ASSET_READ_OK) {
        return -1;
    }
    return fist_palette_decode(input.data, input.size, out);
}

static int wrapped_plane(const fist_klc_image *image) {
    return image->width == image->height && (image->width & (image->width - 1)) == 0;
}

int fist_terrain_load(const fist_scenario *scenario, const fist_asset_source *source,
                      fist_terrain *out) {
    if (scenario == NULL || source == NULL || source->read == NULL || out == NULL) {
        return -1;
    }
    fist_terrain terrain = {0};
    if (load_image(source, scenario->asset_names[HEIGHT_NAME], &terrain.heightmap) != 0 ||
        load_image(source, scenario->asset_names[COLOR_NAME], &terrain.colormap) != 0 ||
        wrapped_plane(&terrain.heightmap) == 0 || wrapped_plane(&terrain.colormap) == 0 ||
        load_palette(source, scenario->asset_names[PALETTE_NAME], &terrain.palette) != 0 ||
        fist_palette_prepare(&terrain.palette, &terrain.palette) != 0 ||
        load_image(source, scenario->asset_names[SKY_NAME], &terrain.sky) != 0 ||
        fist_palette_build_map(terrain.colormap.palette, &terrain.palette, &terrain.colormap_map) !=
            0 ||
        fist_palette_build_map(terrain.sky.palette, &terrain.palette, &terrain.sky_map) != 0) {
        fist_terrain_destroy(&terrain);
        return -1;
    }
    *out = terrain;
    return 0;
}

void fist_terrain_destroy(fist_terrain *terrain) {
    if (terrain != NULL) {
        fist_klc_destroy(&terrain->heightmap);
        fist_klc_destroy(&terrain->colormap);
        fist_klc_destroy(&terrain->sky);
        *terrain = (fist_terrain){0};
    }
}
