#include "assets/owned_terrain.h"
#include "assets/bytes.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAGIC_SIZE = 8,
    VERSION_OFFSET = 8,
    HEADER_LENGTH_OFFSET = 12,
    SIDE_OFFSET = 16,
    HEIGHT_LENGTH_OFFSET = 20,
    COLOR_LENGTH_OFFSET = 24,
    FLAGS_OFFSET = 28,
    MINIMUM_OFFSET = 32,
    MAXIMUM_OFFSET = 40,
    WATER_OFFSET = 48,
    RESERVED_OFFSET = 56,
    CRC_OFFSET = 60,
    HEADER_SIZE = 64,
    FORMAT_VERSION = 1,
    MINIMUM_SIDE = 16,
    MAXIMUM_SIDE = 4096,
    HEIGHT_BYTES = 2,
    COLOR_BYTES = 3,
    DWORD_BITS = 32,
    BYTE_BITS = 8,
    CRC_ENTRIES = 256,
    MINIMUM_ELEVATION = -10000,
    MAXIMUM_ELEVATION = 20000,
    BINARY64_SIGNIFICAND_BITS = 53,
    BINARY64_MAXIMUM_EXPONENT = 1024
};

_Static_assert(sizeof(double) == sizeof(uint64_t) && DBL_MANT_DIG == BINARY64_SIGNIFICAND_BITS &&
                   DBL_MAX_EXP == BINARY64_MAXIMUM_EXPONENT,
               "Owned map metadata requires IEEE-754 binary64");

static int finite_double(double value) {
    static const uint64_t exponent_mask = UINT64_C(0x7ff0000000000000);
    uint64_t bits = 0;
    unsigned char *target = (unsigned char *)&bits;
    const volatile unsigned char *source = (const volatile unsigned char *)&value;
    for (size_t index = 0; index < sizeof(bits); ++index) {
        target[index] = source[index];
    }
    /* Ordinary bit copies are also folded to an always-true floating-class
     * check under production fast math. Volatile representation reads preserve
     * the integer exponent test without changing simulation compiler flags. */
    return (bits & exponent_mask) != exponent_mask;
}

static double read_double(const uint8_t *data) {
    const uint64_t bits = (uint64_t)fist_read_u32le(data) |
                          ((uint64_t)fist_read_u32le(data + sizeof(uint32_t)) << DWORD_BITS);
    double value = 0;
    unsigned char *target = (unsigned char *)&value;
    const unsigned char *source = (const unsigned char *)&bits;
    for (size_t index = 0; index < sizeof(value); ++index) {
        target[index] = source[index];
    }
    return value;
}

static uint32_t crc_part(const uint8_t *data, size_t size, const uint32_t *table, uint32_t state) {
    for (size_t index = 0; index < size; ++index) {
        state = table[(state ^ data[index]) & UINT8_MAX] ^ (state >> BYTE_BITS);
    }
    return state;
}

static uint32_t bundle_crc(const uint8_t *data, size_t size) {
    static const uint32_t polynomial = UINT32_C(0xedb88320);
    uint32_t table[CRC_ENTRIES] = {0};
    for (uint32_t index = 0; index < CRC_ENTRIES; ++index) {
        uint32_t value = index;
        for (unsigned bit = 0; bit < BYTE_BITS; ++bit) {
            value = (value >> 1) ^ ((value & 1U) != 0 ? polynomial : 0);
        }
        table[index] = value;
    }
    const uint32_t prefix = crc_part(data, CRC_OFFSET, table, UINT32_MAX);
    return crc_part(data + HEADER_SIZE, size - HEADER_SIZE, table, prefix) ^ UINT32_MAX;
}

static int valid_side(uint32_t side) {
    return side >= MINIMUM_SIDE && side <= MAXIMUM_SIDE && (side & (side - 1)) == 0;
}

static int valid_metadata(const fist_owned_terrain *terrain) {
    return finite_double(terrain->minimum) != 0 && finite_double(terrain->maximum) != 0 &&
           finite_double(terrain->water_level) != 0 && terrain->minimum >= MINIMUM_ELEVATION &&
           terrain->maximum <= MAXIMUM_ELEVATION && terrain->minimum < terrain->maximum &&
           terrain->water_level >= MINIMUM_ELEVATION && terrain->water_level <= MAXIMUM_ELEVATION;
}

int fist_owned_terrain_decode(const uint8_t *data, size_t size, fist_owned_terrain *out) {
    if (data == NULL || out == NULL || size < HEADER_SIZE ||
        memcmp(data, "FISTMAP\0", MAGIC_SIZE) != 0 ||
        fist_read_u32le(data + VERSION_OFFSET) != FORMAT_VERSION ||
        fist_read_u32le(data + HEADER_LENGTH_OFFSET) != HEADER_SIZE ||
        fist_read_u32le(data + FLAGS_OFFSET) != 0 || fist_read_u32le(data + RESERVED_OFFSET) != 0) {
        return -1;
    }
    fist_owned_terrain terrain = {.side = fist_read_u32le(data + SIDE_OFFSET),
                                  .minimum = read_double(data + MINIMUM_OFFSET),
                                  .maximum = read_double(data + MAXIMUM_OFFSET),
                                  .water_level = read_double(data + WATER_OFFSET)};
    if (valid_side(terrain.side) == 0 || valid_metadata(&terrain) == 0) {
        return -1;
    }
    const size_t count = (size_t)terrain.side * terrain.side;
    const size_t height_size = count * HEIGHT_BYTES;
    const size_t color_size = count * COLOR_BYTES;
    if (fist_read_u32le(data + HEIGHT_LENGTH_OFFSET) != height_size ||
        fist_read_u32le(data + COLOR_LENGTH_OFFSET) != color_size ||
        size != HEADER_SIZE + height_size + color_size ||
        fist_read_u32le(data + CRC_OFFSET) != bundle_crc(data, size)) {
        return -1;
    }
    terrain.heights = malloc(count * sizeof(*terrain.heights));
    terrain.colors = malloc(color_size);
    if (terrain.heights == NULL || terrain.colors == NULL) {
        fist_owned_terrain_destroy(&terrain);
        return -1;
    }
    for (size_t index = 0; index < count; ++index) {
        terrain.heights[index] = fist_read_u16le(data + HEADER_SIZE + (index * HEIGHT_BYTES));
    }
    for (size_t index = 0; index < color_size; ++index) {
        terrain.colors[index] = data[HEADER_SIZE + height_size + index];
    }
    *out = terrain;
    return 0;
}

void fist_owned_terrain_destroy(fist_owned_terrain *terrain) {
    if (terrain != NULL) {
        free(terrain->heights);
        free(terrain->colors);
        *terrain = (fist_owned_terrain){0};
    }
}

static double height_at(const fist_owned_terrain *terrain, size_t column, size_t row) {
    const size_t mask = terrain->side - 1;
    const size_t index = ((row & mask) * terrain->side) + (column & mask);
    return terrain->minimum +
           ((double)terrain->heights[index] / UINT16_MAX * (terrain->maximum - terrain->minimum));
}

int fist_owned_terrain_surface(const fist_owned_terrain *terrain, double map_x, double map_y,
                               double *height) {
    if (terrain == NULL || height == NULL || terrain->heights == NULL || terrain->colors == NULL ||
        valid_side(terrain->side) == 0 || valid_metadata(terrain) == 0 ||
        finite_double(map_x) == 0 || finite_double(map_y) == 0) {
        return -1;
    }
    const double local_x = (map_x - floor(map_x)) * terrain->side;
    const double local_y = (map_y - floor(map_y)) * terrain->side;
    const size_t column = (size_t)local_x;
    const size_t row = (size_t)local_y;
    const double fraction_x = local_x - (double)column;
    const double fraction_y = local_y - (double)row;
    const double top_left = height_at(terrain, column, row);
    const double top_right = height_at(terrain, column + 1, row);
    const double bottom_left = height_at(terrain, column, row + 1);
    const double bottom_right = height_at(terrain, column + 1, row + 1);
    if (fraction_x + fraction_y <= 1.0) {
        *height = top_left + (fraction_x * (top_right - top_left)) +
                  (fraction_y * (bottom_left - top_left));
    } else {
        *height = bottom_right + ((1.0 - fraction_x) * (bottom_left - bottom_right)) +
                  ((1.0 - fraction_y) * (top_right - bottom_right));
    }
    return 0;
}
