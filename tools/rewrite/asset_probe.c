#include "assets/klc.h"
#include "assets/palette.h"
#include "assets/resource.h"
#include "assets/view.h"
#include "probe_io.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MARKER_WIDTH = 17, MARKER_HEIGHT = 19, MARKER_COMPONENT = 0xa5, CONTRACT_FAILURE = 2 };

static fist_klc_image image_marker(void) {
    fist_klc_image image = {.width = MARKER_WIDTH, .height = MARKER_HEIGHT};
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        image.palette[index] = MARKER_COMPONENT;
    }
    return image;
}

static int is_image_marker(const fist_klc_image *image) {
    if (image->width != MARKER_WIDTH || image->height != MARKER_HEIGHT || image->pixels != NULL) {
        return 0;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (image->palette[index] != MARKER_COMPONENT) {
            return 0;
        }
    }
    return 1;
}

static int probe_image(int prefixes, const uint8_t *data, size_t size) {
    fist_klc_image image = image_marker();
    if (fist_klc_decode(data, size, &image) != 0) {
        return is_image_marker(&image) != 0 ? EXIT_FAILURE : CONTRACT_FAILURE;
    }
    if (prefixes != 0) {
        fist_klc_destroy(&image);
        for (size_t length = 0; length < size; ++length) {
            image = image_marker();
            if (fist_klc_decode(data, length, &image) != -1 || is_image_marker(&image) == 0) {
                fist_klc_destroy(&image);
                return CONTRACT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }
    const size_t count = (size_t)image.width * image.height;
    const int written = printf("%" PRIu32 " %" PRIu32 "\n", image.width, image.height) > 0 &&
                        fwrite(image.palette, 1, FIST_PALETTE_SIZE, stdout) == FIST_PALETTE_SIZE &&
                        fwrite(image.pixels, 1, count, stdout) == count;
    fist_klc_destroy(&image);
    return written != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int probe_resource(const uint8_t *data, size_t size, const char *name) {
    fist_asset_view member = {data, size};
    if (fist_resource_find(data, size, name, &member) != 0) {
        return member.data == data && member.size == size ? EXIT_FAILURE : CONTRACT_FAILURE;
    }
    return fwrite(member.data, 1, member.size, stdout) == member.size ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int probe_palette(int prepared, const uint8_t *data, size_t size) {
    fist_palette palette = {0};
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        palette.rgb6[index] = MARKER_COMPONENT;
    }
    if (fist_palette_decode(data, size, &palette) != 0) {
        for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
            if (palette.rgb6[index] != MARKER_COMPONENT) {
                return CONTRACT_FAILURE;
            }
        }
        return EXIT_FAILURE;
    }
    if (prepared != 0 && fist_palette_prepare(&palette, &palette) != 0) {
        return CONTRACT_FAILURE;
    }
    return fwrite(palette.rgb6, 1, FIST_PALETTE_SIZE, stdout) == FIST_PALETTE_SIZE ? EXIT_SUCCESS
                                                                                   : EXIT_FAILURE;
}

static int probe_palette_map(const uint8_t *data, size_t size) {
    fist_palette palette = {0};
    fist_palette_map map = {0};
    if (size != (size_t)FIST_PALETTE_SIZE * 2 ||
        fist_palette_decode(data, FIST_PALETTE_SIZE, &palette) != 0 ||
        fist_palette_prepare(&palette, &palette) != 0 ||
        fist_palette_build_map(data + FIST_PALETTE_SIZE, &palette, &map) != 0) {
        return EXIT_FAILURE;
    }
    return fwrite(palette.rgb6, 1, FIST_PALETTE_SIZE, stdout) == FIST_PALETTE_SIZE &&
                   fwrite(map.indices, 1, FIST_PALETTE_COLORS, stdout) == FIST_PALETTE_COLORS
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

static int probe_null_contracts(void) {
    fist_klc_image image = image_marker();
    fist_palette palette = {0};
    fist_palette_map map = {0};
    fist_asset_view member = {NULL, 0};
    const uint8_t data[] = {0};
    if (fist_klc_decode(NULL, 0, &image) != -1 || is_image_marker(&image) == 0 ||
        fist_klc_decode(data, sizeof(data), NULL) != -1 ||
        fist_palette_decode(NULL, 0, &palette) != -1 ||
        fist_palette_decode(data, sizeof(data), NULL) != -1 ||
        fist_palette_prepare(NULL, &palette) != -1 || fist_palette_prepare(&palette, NULL) != -1 ||
        fist_palette_build_map(NULL, &palette, &map) != -1 ||
        fist_palette_build_map(palette.rgb6, NULL, &map) != -1 ||
        fist_palette_build_map(palette.rgb6, &palette, NULL) != -1 ||
        fist_resource_find(NULL, 0, "A.PAL", &member) != -1 ||
        fist_resource_find(data, sizeof(data), NULL, &member) != -1 ||
        fist_resource_find(data, sizeof(data), "A.PAL", NULL) != -1) {
        return CONTRACT_FAILURE;
    }
    fist_klc_destroy(NULL);
    fist_klc_destroy(&image);
    fist_klc_destroy(&image);
    if (image.width != 0 || image.height != 0 || image.pixels != NULL) {
        return CONTRACT_FAILURE;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (image.palette[index] != 0) {
            return CONTRACT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "contracts") == 0) {
        return probe_null_contracts();
    }
    if (argc < 3 || argc > 4) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[2], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (data == NULL || closed != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    int result = EXIT_FAILURE;
    if (argc == 3 && (strcmp(argv[1], "klc") == 0 || strcmp(argv[1], "klc-prefixes") == 0)) {
        result = probe_image(strcmp(argv[1], "klc-prefixes") == 0, data, size);
    } else if (argc == 3 &&
               (strcmp(argv[1], "palette") == 0 || strcmp(argv[1], "palette-prepared") == 0)) {
        result = probe_palette(strcmp(argv[1], "palette-prepared") == 0, data, size);
    } else if (argc == 3 && strcmp(argv[1], "palette-map") == 0) {
        result = probe_palette_map(data, size);
    } else if (argc == 4 && strcmp(argv[1], "resource") == 0) {
        result = probe_resource(data, size, argv[3]);
    }
    free(data);
    return result;
}
