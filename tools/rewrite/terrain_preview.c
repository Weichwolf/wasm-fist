#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "probe_io.h"
#include "probe_source.h"
#include "render/renderer.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include "platform/wasm/present.h"
#endif

enum {
    PREVIEW_WIDTH = 640,
    PREVIEW_HEIGHT = 400,
    RGBA_CHANNELS = 4,
    RGB_CHANNELS = 3,
    OPAQUE_ALPHA = 255,
    HEADING_ARGUMENT_COUNT = 5,
    DECIMAL_BASE = 10
};

static int write_number(FILE *file, unsigned value) {
    char digits[sizeof(value) * CHAR_BIT] = {0};
    size_t count = 0;
    do {
        digits[count++] = (char)('0' + (value % DECIMAL_BASE));
        value /= DECIMAL_BASE;
    } while (value != 0);
    while (count != 0) {
        if (fwrite(digits + --count, 1, 1, file) != 1) {
            return -1;
        }
    }
    return 0;
}

static int write_frame(const char *path, const uint8_t *pixels) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        return -1;
    }
    int result = -1;
    if (fputs("P6\n", file) != EOF && write_number(file, PREVIEW_WIDTH) == 0 &&
        fputs(" ", file) != EOF && write_number(file, PREVIEW_HEIGHT) == 0 &&
        fputs("\n", file) != EOF && write_number(file, OPAQUE_ALPHA) == 0 &&
        fputs("\n", file) != EOF) {
        result = 0;
    }
    for (size_t row = PREVIEW_HEIGHT; row > 0 && result == 0; --row) {
        for (size_t column = 0; column < PREVIEW_WIDTH; ++column) {
            const uint8_t *pixel =
                pixels + ((((row - 1) * PREVIEW_WIDTH) + column) * RGBA_CHANNELS);
            if (fwrite(pixel, 1, RGB_CHANNELS, file) != RGB_CHANNELS) {
                result = -1;
                break;
            }
        }
    }
    return fclose(file) == 0 ? result : -1;
}

static int draw_preview(const char *heading, const fist_terrain *terrain,
                        const fist_unit_definition *vehicle, const char *output) {
    fist_terrain_view view = {0};
    if (fist_terrain_inspection_view(terrain, vehicle, &view) != 0) {
        return -1;
    }
    if (heading != NULL) {
        errno = 0;
        char *end = NULL;
        const unsigned long value = strtoul(heading, &end, 10);
        if (errno != 0 || end == heading || *end != '\0' || value > UINT16_MAX) {
            return -1;
        }
        view.heading = (uint16_t)value;
    }
    fist_renderer *renderer = fist_renderer_create(PREVIEW_WIDTH, PREVIEW_HEIGHT);
    if (renderer == NULL) {
        return -1;
    }
    if (fist_renderer_draw_terrain(renderer, terrain, &view) != 0) {
        fist_renderer_destroy(renderer);
        return -1;
    }
    const uint8_t *pixels = fist_renderer_pixels(renderer);
    int result = -1;
    if (pixels != NULL) {
        const size_t size = (size_t)PREVIEW_WIDTH * PREVIEW_HEIGHT * RGBA_CHANNELS;
        const uint32_t fnv_offset = UINT32_C(2166136261);
        const uint32_t fnv_prime = UINT32_C(16777619);
        uint32_t hash = fnv_offset;
        int opaque = 1;
        for (size_t index = 0; index < size; ++index) {
            hash = (hash ^ pixels[index]) * fnv_prime;
            if (index % RGBA_CHANNELS == RGBA_CHANNELS - 1 && pixels[index] != OPAQUE_ALPHA) {
                opaque = 0;
            }
        }
        if (opaque != 0 && write_frame(output, pixels) == 0) {
#ifdef __EMSCRIPTEN__
            fist_present_rgba(pixels, PREVIEW_WIDTH, PREVIEW_HEIGHT);
#endif
            printf("terrain inspection: %dx%d RGBA8 fnv1a=%08" PRIx32 "\n", PREVIEW_WIDTH,
                   PREVIEW_HEIGHT, hash);
            result = 0;
        }
    }
    fist_renderer_destroy(renderer);
    return result;
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > HEADING_ARGUMENT_COUNT) {
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
    fist_units units = {0};
    const int loaded = fist_units_decode(&scenario, &units) == 0
                           ? fist_terrain_load(&scenario, &source, &terrain)
                           : -1;
    fist_probe_source_close(&storage);
    free(data);
    const fist_unit_definition *vehicle = fist_units_roster_get(&units, 0);
    const int result = loaded == 0 ? draw_preview(argc == HEADING_ARGUMENT_COUNT ? argv[4] : NULL,
                                                  &terrain, vehicle, argv[3])
                                   : -1;
    fist_terrain_destroy(&terrain);
    fist_units_destroy(&units);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
