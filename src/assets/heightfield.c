#include "assets/heightfield.h"

#include "assets/klc.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static int plane_size(uint32_t side, size_t *out) {
    if (side == 0 || (size_t)side > SIZE_MAX / side) {
        return -1;
    }
    *out = (size_t)side * side;
    return 0;
}

static int scale_ratio(uint32_t source, uint32_t target) {
    const uint32_t smaller = source < target ? source : target;
    const uint32_t larger = source < target ? target : source;
    if (smaller == 0 || larger % smaller != 0) {
        return -1;
    }
    const uint32_t ratio = larger / smaller;
    return (ratio & (ratio - 1)) == 0 ? 0 : -1;
}

static void enlarge(const uint8_t *source, size_t side, uint8_t *out) {
    const size_t width = side * 2;
    for (size_t row = 0; row < side; ++row) {
        uint8_t *expanded = out + (row * 2 * width);
        for (size_t column = 0; column < side; ++column) {
            const uint8_t current = source[(row * side) + column];
            const uint8_t next = source[(row * side) + ((column + 1) % side)];
            expanded[column * 2] = current;
            expanded[(column * 2) + 1] = (uint8_t)(((unsigned)current + next) / 2);
        }
    }
    for (size_t row = 0; row < side; ++row) {
        const uint8_t *current = out + (row * 2 * width);
        const uint8_t *next = out + (((row + 1) % side) * 2 * width);
        uint8_t *between = out + (((row * 2) + 1) * width);
        for (size_t column = 0; column < width; ++column) {
            between[column] = (uint8_t)(((unsigned)current[column] + next[column]) / 2);
        }
    }
}

static void reduce(const uint8_t *source, size_t side, uint8_t *out) {
    const size_t width = side / 2;
    for (size_t row = 0; row < width; ++row) {
        for (size_t column = 0; column < width; ++column) {
            out[(row * width) + column] = source[(row * 2 * side) + (column * 2)];
        }
    }
}

int fist_heightfield_resample(const fist_klc_image *source, uint32_t target_side,
                              fist_klc_image *out) {
    size_t size = 0;
    size_t target_size = 0;
    if (source == NULL || out == NULL || source == out || source->pixels == NULL ||
        source->width != source->height || plane_size(source->width, &size) != 0 ||
        plane_size(target_side, &target_size) != 0 ||
        scale_ratio(source->width, target_side) != 0) {
        return -1;
    }
    fist_klc_image image = {.width = source->width, .height = source->height};
    image.pixels = malloc(size);
    if (image.pixels == NULL) {
        return -1;
    }
    for (size_t index = 0; index < size; ++index) {
        image.pixels[index] = source->pixels[index];
    }
    for (size_t index = 0; index < sizeof(image.palette); ++index) {
        image.palette[index] = source->palette[index];
    }
    while (image.width != target_side) {
        const uint32_t next_side = image.width < target_side ? image.width * 2 : image.width / 2;
        size_t next_size = 0;
        if (plane_size(next_side, &next_size) != 0) {
            fist_klc_destroy(&image);
            return -1;
        }
        uint8_t *pixels = malloc(next_size);
        if (pixels == NULL) {
            fist_klc_destroy(&image);
            return -1;
        }
        if (next_side > image.width) {
            enlarge(image.pixels, image.width, pixels);
        } else {
            reduce(image.pixels, image.width, pixels);
        }
        free(image.pixels);
        image.pixels = pixels;
        image.width = next_side;
        image.height = next_side;
    }
    *out = image;
    return 0;
}
