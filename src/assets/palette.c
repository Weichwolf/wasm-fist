#include "assets/palette.h"

#include <stddef.h>
#include <stdint.h>

enum {
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    GREEN_LUMINANCE_WEIGHT = 2,
    SOURCE_DISTANCE_SHIFT = 3,
    MISSION_DISTANCE_SHIFT = 1
};

static int valid_palette(const fist_palette *palette) {
    if (palette == NULL) {
        return 0;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (palette->rgb6[index] > FIST_PALETTE_DAC_MAX) {
            return 0;
        }
    }
    return 1;
}

int fist_palette_decode(const uint8_t *data, size_t size, fist_palette *out) {
    if (data == NULL || out == NULL || size != FIST_PALETTE_SIZE) {
        return -1;
    }
    fist_palette palette = {0};
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (data[index] > FIST_PALETTE_DAC_MAX) {
            return -1;
        }
        palette.rgb6[index] = data[index];
    }
    *out = palette;
    return 0;
}

static unsigned luminance(const fist_palette *palette, size_t index) {
    const uint8_t *color = palette->rgb6 + (index * FIST_PALETTE_CHANNELS);
    return (unsigned)color[RED] + ((unsigned)color[GREEN] * GREEN_LUMINANCE_WEIGHT) + color[BLUE];
}

int fist_palette_prepare(const fist_palette *source, fist_palette *out) {
    if (out == NULL || valid_palette(source) == 0) {
        return -1;
    }
    fist_palette palette = *source;
    for (size_t start = FIST_TERRAIN_PALETTE_START; start < FIST_PALETTE_COLORS - 1; ++start) {
        size_t best = start;
        unsigned minimum = luminance(&palette, start);
        for (size_t index = start + 1; index < FIST_PALETTE_COLORS; ++index) {
            const unsigned value = luminance(&palette, index);
            if (value < minimum) {
                minimum = value;
                best = index;
            }
        }
        for (size_t channel = 0; channel < FIST_PALETTE_CHANNELS; ++channel) {
            const size_t left = (start * FIST_PALETTE_CHANNELS) + channel;
            const size_t right = (best * FIST_PALETTE_CHANNELS) + channel;
            const uint8_t value = palette.rgb6[left];
            palette.rgb6[left] = palette.rgb6[right];
            palette.rgb6[right] = value;
        }
    }
    *out = palette;
    return 0;
}

int fist_palette_build_map(const uint8_t rgb8[FIST_PALETTE_SIZE], const fist_palette *mission,
                           fist_palette_map *out) {
    if (rgb8 == NULL || out == NULL || valid_palette(mission) == 0) {
        return -1;
    }
    /* Verified original tables a060/a460/a860: signed differences squared,
     * weighted by 31^2, 43^2 and 26^2 respectively. */
    static const unsigned weights[FIST_PALETTE_CHANNELS] = {961, 1849, 676};
    fist_palette_map map = {0};
    for (size_t color = 1; color < FIST_PALETTE_COLORS; ++color) {
        unsigned minimum = UINT32_MAX;
        size_t best = FIST_TERRAIN_PALETTE_START;
        for (size_t index = FIST_TERRAIN_PALETTE_START; index < FIST_PALETTE_COLORS; ++index) {
            unsigned distance = 0;
            for (size_t channel = 0; channel < FIST_PALETTE_CHANNELS; ++channel) {
                const unsigned target =
                    rgb8[(color * FIST_PALETTE_CHANNELS) + channel] >> SOURCE_DISTANCE_SHIFT;
                const unsigned candidate =
                    mission->rgb6[(index * FIST_PALETTE_CHANNELS) + channel] >>
                    MISSION_DISTANCE_SHIFT;
                const int delta = (int)candidate - (int)target;
                distance += (unsigned)(delta * delta) * weights[channel];
            }
            if (distance < minimum) {
                minimum = distance;
                best = index;
            }
        }
        map.indices[color] = (uint8_t)best;
    }
    *out = map;
    return 0;
}
