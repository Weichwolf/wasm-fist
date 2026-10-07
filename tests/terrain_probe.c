#include "assets/klc.h"
#include "assets/palette.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/terrain.h"
#include "probe_io.h"
#include "probe_source.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { MARKER_WIDTH = 17, CONTRACT_FAILURE = 2 };

static int empty_image(const fist_klc_image *image) {
    if (image->height != 0 || image->pixels != NULL) {
        return 0;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (image->palette[index] != 0) {
            return 0;
        }
    }
    return 1;
}

static int terrain_marker(const fist_terrain *terrain) {
    if (terrain->heightmap.width != MARKER_WIDTH || terrain->colormap.width != 0 ||
        terrain->sky.width != 0 || empty_image(&terrain->heightmap) == 0 ||
        empty_image(&terrain->colormap) == 0 || empty_image(&terrain->sky) == 0) {
        return 0;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (terrain->palette.rgb6[index] != 0) {
            return 0;
        }
    }
    for (size_t index = 0; index < FIST_PALETTE_COLORS; ++index) {
        if (terrain->colormap_map.indices[index] != 0 || terrain->sky_map.indices[index] != 0) {
            return 0;
        }
    }
    return 1;
}

static int write_image(const fist_klc_image *image) {
    const size_t count = (size_t)image->width * image->height;
    return fwrite(image->palette, 1, FIST_PALETTE_SIZE, stdout) == FIST_PALETTE_SIZE &&
           fwrite(image->pixels, 1, count, stdout) == count;
}

static int write_terrain(const fist_terrain *terrain) {
    return printf("%" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
                  terrain->heightmap.width, terrain->heightmap.height, terrain->colormap.width,
                  terrain->colormap.height, terrain->sky.width, terrain->sky.height) > 0 &&
           fwrite(terrain->palette.rgb6, 1, FIST_PALETTE_SIZE, stdout) == FIST_PALETTE_SIZE &&
           fwrite(terrain->colormap_map.indices, 1, FIST_PALETTE_COLORS, stdout) ==
               FIST_PALETTE_COLORS &&
           fwrite(terrain->sky_map.indices, 1, FIST_PALETTE_COLORS, stdout) ==
               FIST_PALETTE_COLORS &&
           write_image(&terrain->heightmap) != 0 && write_image(&terrain->colormap) != 0 &&
           write_image(&terrain->sky) != 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    if (data == NULL || closed != 0 || fist_scenario_decode(data, size, &scenario) != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    fist_probe_source storage = {.directory = argv[2]};
    const fist_asset_source source = {fist_probe_source_read, &storage};
    fist_terrain terrain = {0};
    terrain.heightmap.width = MARKER_WIDTH;
    const int loaded = fist_terrain_load(&scenario, &source, &terrain);
    fist_probe_source_close(&storage);
    free(data); /* Bundle ownership must survive destruction of every input. */
    if (loaded != 0) {
        return terrain_marker(&terrain) != 0 ? EXIT_FAILURE : CONTRACT_FAILURE;
    }
    const int written = write_terrain(&terrain);
    fist_terrain_destroy(&terrain);
    fist_terrain_destroy(&terrain);
    fist_terrain_destroy(NULL);
    terrain.heightmap.width = MARKER_WIDTH;
    if (terrain_marker(&terrain) == 0) {
        return CONTRACT_FAILURE;
    }
    return written != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
