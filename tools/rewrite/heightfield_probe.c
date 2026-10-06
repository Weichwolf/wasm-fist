#include "assets/bytes.h"
#include "assets/heightfield.h"
#include "assets/klc.h"
#include "assets/palette.h"
#include "probe_io.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { HEADER_SIZE = 12, WORD_SIZE = 4, COUNT_OFFSET = 8, MARKER = 0xa5, CONTRACT_FAILURE = 2 };

static fist_klc_image image_marker(void) {
    fist_klc_image image = {.width = MARKER, .height = MARKER};
    for (size_t index = 0; index < sizeof(image.palette); ++index) {
        image.palette[index] = MARKER;
    }
    return image;
}

static int is_marker(const fist_klc_image *image) {
    if (image->width != MARKER || image->height != MARKER || image->pixels != NULL) {
        return 0;
    }
    for (size_t index = 0; index < sizeof(image->palette); ++index) {
        if (image->palette[index] != MARKER) {
            return 0;
        }
    }
    return 1;
}

static int equal_image(const fist_klc_image *first, const fist_klc_image *second) {
    if (first->width != second->width || first->height != second->height ||
        first->pixels != second->pixels) {
        return 0;
    }
    for (size_t index = 0; index < sizeof(first->palette); ++index) {
        if (first->palette[index] != second->palette[index]) {
            return 0;
        }
    }
    return 1;
}

static int contracts(void) {
    uint8_t pixel = MARKER;
    fist_klc_image source = {.width = 1, .height = 1, .pixels = &pixel};
    fist_klc_image out = image_marker();
    if (fist_heightfield_resample(NULL, 1, &out) != -1 || is_marker(&out) == 0 ||
        fist_heightfield_resample(&source, 1, NULL) != -1 ||
        fist_heightfield_resample(&source, 1, &source) != -1 || source.pixels != &pixel ||
        source.width != 1 || source.height != 1 || pixel != MARKER) {
        return CONTRACT_FAILURE;
    }
    const uint32_t invalid[][3] = {{0, 0, 1},
                                   {1, 0, 1},
                                   {1, 2, 1},
                                   {1, 1, 0},
                                   {1, 1, 3},
                                   {3, 3, 2},
                                   {UINT32_MAX, UINT32_MAX, 0},
                                   {1, 1, UINT32_MAX}};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        source.width = invalid[index][0];
        source.height = invalid[index][1];
        const fist_klc_image before = source;
        if (fist_heightfield_resample(&source, invalid[index][2], &out) != -1 ||
            is_marker(&out) == 0 || equal_image(&source, &before) == 0 || pixel != MARKER) {
            return CONTRACT_FAILURE;
        }
    }
    source = (fist_klc_image){.width = 1, .height = 1};
    if (fist_heightfield_resample(&source, 1, &out) != -1 || is_marker(&out) == 0) {
        return CONTRACT_FAILURE;
    }
#if SIZE_MAX == UINT32_MAX
    source =
        (fist_klc_image){.width = UINT32_C(65536), .height = UINT32_C(65536), .pixels = &pixel};
    if (fist_heightfield_resample(&source, source.width, &out) != -1 || is_marker(&out) == 0) {
        return CONTRACT_FAILURE;
    }
    source = (fist_klc_image){.width = 1, .height = 1, .pixels = &pixel};
    if (fist_heightfield_resample(&source, UINT32_C(65536), &out) != -1 || is_marker(&out) == 0) {
        return CONTRACT_FAILURE;
    }
#endif
    return EXIT_SUCCESS;
}

static void erase_source(fist_klc_image *source) {
    for (size_t index = 0; index < (size_t)source->width * source->height; ++index) {
        source->pixels[index] ^= UINT8_MAX;
    }
    for (size_t index = 0; index < sizeof(source->palette); ++index) {
        source->palette[index] ^= UINT8_MAX;
    }
    fist_klc_destroy(source);
    fist_klc_destroy(source);
}

static int preserved_source(const fist_klc_image *source, const fist_klc_image *before,
                            const uint8_t *pixels) {
    if (equal_image(source, before) == 0) {
        return 0;
    }
    for (size_t index = 0; index < (size_t)source->width * source->height; ++index) {
        if (source->pixels[index] != pixels[index]) {
            return 0;
        }
    }
    return 1;
}

static int stages(fist_klc_image *image, const uint8_t *targets, uint32_t count) {
    for (uint32_t step = 0; step < count; ++step) {
        const fist_klc_image before = *image;
        const size_t size = (size_t)image->width * image->height;
        uint8_t *backup = malloc(size);
        if (backup == NULL) {
            return CONTRACT_FAILURE;
        }
        for (size_t index = 0; index < size; ++index) {
            backup[index] = image->pixels[index];
        }
        fist_klc_image out = image_marker();
        const int status = fist_heightfield_resample(
            image, fist_read_u32le(targets + ((size_t)step * WORD_SIZE)), &out);
        const int preserved = preserved_source(image, &before, backup);
        free(backup);
        if (preserved == 0 || (status != 0 && is_marker(&out) == 0)) {
            fist_klc_destroy(&out);
            return CONTRACT_FAILURE;
        }
        if (status != 0) {
            return EXIT_FAILURE;
        }
        if (out.pixels == image->pixels) {
            return CONTRACT_FAILURE;
        }
        erase_source(image);
        *image = out;
    }
    return EXIT_SUCCESS;
}

static int probe(const uint8_t *data, size_t size) {
    if (size < HEADER_SIZE) {
        return EXIT_FAILURE;
    }
    const uint32_t width = fist_read_u32le(data);
    const uint32_t height = fist_read_u32le(data + WORD_SIZE);
    const uint32_t count = fist_read_u32le(data + COUNT_OFFSET);
    if (width == 0 || height == 0 || (size_t)width > SIZE_MAX / height || count == 0 ||
        count > (size - HEADER_SIZE) / WORD_SIZE) {
        return EXIT_FAILURE;
    }
    const size_t offset = HEADER_SIZE + ((size_t)count * WORD_SIZE);
    const size_t plane_size = (size_t)width * height;
    if (size - offset < FIST_PALETTE_SIZE || size - offset - FIST_PALETTE_SIZE != plane_size) {
        return EXIT_FAILURE;
    }
    fist_klc_image image = {.width = width, .height = height};
    image.pixels = malloc(plane_size);
    if (image.pixels == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t index = 0; index < sizeof(image.palette); ++index) {
        image.palette[index] = data[offset + index];
    }
    for (size_t index = 0; index < plane_size; ++index) {
        image.pixels[index] = data[offset + FIST_PALETTE_SIZE + index];
    }
    int result = stages(&image, data + HEADER_SIZE, count);
    if (result == EXIT_SUCCESS) {
        const size_t output_size = (size_t)image.width * image.height;
        const int written =
            printf("%" PRIu32 " %" PRIu32 "\n", image.width, image.height) > 0 &&
            fwrite(image.palette, 1, sizeof(image.palette), stdout) == sizeof(image.palette) &&
            fwrite(image.pixels, 1, output_size, stdout) == output_size;
        result = written != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    fist_klc_destroy(&image);
    return result;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "contracts") == 0) {
        return contracts();
    }
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
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
    const int result = probe(data, size);
    free(data);
    return result;
}
