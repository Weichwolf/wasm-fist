#include "assets/klc.h"
#include "assets/bytes.h"
#include "assets/palette.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    TAG_SIZE = 4,
    WIDTH_OFFSET = 4,
    HEIGHT_OFFSET = 8,
    PALETTE_OFFSET = 12,
    HEADER_SIZE = PALETTE_OFFSET + FIST_PALETTE_SIZE,
    BLOCK_SIDE = 4,
    BLOCK_PIXELS = BLOCK_SIDE * BLOCK_SIDE,
    CONTROL_BLOCKS = 4,
    MODE_BITS = 2,
    MODE_MASK = 3,
    BLOCK_FOUR_COLORS = 0,
    BLOCK_TWO_COLORS = 1,
    BLOCK_FILL_OR_RAW = 2,
    BLOCK_ZERO = 3,
    FOUR_COLOR_SIZE = 7,
    TWO_COLOR_SIZE = 4,
    FOUR_COLOR_SELECTOR_OFFSET = 3,
    TWO_COLOR_SELECTOR_OFFSET = 2
};

static const uint8_t *take_bytes(fist_asset_view *input, size_t size) {
    if (size > input->size) {
        return NULL;
    }
    const uint8_t *data = input->data;
    input->data += size;
    input->size -= size;
    return data;
}

static int decode_block(fist_asset_view *input, unsigned mode, uint8_t *pixels) {
    if (mode == BLOCK_ZERO) {
        for (size_t index = 0; index < BLOCK_PIXELS; ++index) {
            pixels[index] = 0;
        }
        return 0;
    }
    if (mode == BLOCK_FILL_OR_RAW) {
        const uint8_t *value = take_bytes(input, 1);
        if (value == NULL) {
            return -1;
        }
        if (*value != 0) {
            for (size_t index = 0; index < BLOCK_PIXELS; ++index) {
                pixels[index] = *value;
            }
            return 0;
        }
        const uint8_t *raw = take_bytes(input, BLOCK_PIXELS);
        if (raw == NULL) {
            return -1;
        }
        for (size_t index = 0; index < BLOCK_PIXELS; ++index) {
            pixels[index] = raw[index];
        }
        return 0;
    }
    const uint8_t *encoded =
        take_bytes(input, mode == BLOCK_FOUR_COLORS ? FOUR_COLOR_SIZE : TWO_COLOR_SIZE);
    if (encoded == NULL) {
        return -1;
    }
    uint8_t colors[CONTROL_BLOCKS] = {0};
    unsigned selector_bits = 1;
    uint32_t selectors = 0;
    if (mode == BLOCK_FOUR_COLORS) {
        for (size_t index = 1; index < CONTROL_BLOCKS; ++index) {
            colors[index] = encoded[index - 1];
        }
        selector_bits = MODE_BITS;
        selectors = fist_read_u32le(encoded + FOUR_COLOR_SELECTOR_OFFSET);
    } else {
        colors[0] = encoded[0];
        colors[1] = encoded[1];
        selectors = fist_read_u16le(encoded + TWO_COLOR_SELECTOR_OFFSET);
    }
    const unsigned selector_mask = (1U << selector_bits) - 1U;
    for (size_t index = 0; index < BLOCK_PIXELS; ++index) {
        pixels[index] = colors[selectors & selector_mask];
        selectors >>= selector_bits;
    }
    return 0;
}

static int decode_blocks(fist_asset_view input, const fist_klc_image *image) {
    const size_t width = image->width;
    const size_t height = image->height;
    unsigned control = 0;
    size_t block_index = 0;
    for (size_t row = 0; row < height; row += BLOCK_SIDE) {
        for (size_t column = 0; column < width; column += BLOCK_SIDE) {
            if (block_index % CONTROL_BLOCKS == 0) {
                const uint8_t *value = take_bytes(&input, 1);
                if (value == NULL) {
                    return -1;
                }
                control = *value;
            }
            uint8_t block[BLOCK_PIXELS] = {0};
            if (decode_block(&input, control & MODE_MASK, block) != 0) {
                return -1;
            }
            control >>= MODE_BITS;
            ++block_index;
            if (image->pixels != NULL) {
                for (size_t offset = 0; offset < BLOCK_PIXELS; ++offset) {
                    image->pixels[((row + (offset / BLOCK_SIDE)) * width) + column +
                                  (offset % BLOCK_SIDE)] = block[offset];
                }
            }
        }
    }
    return input.size == 0 ? 0 : -1;
}

int fist_klc_decode(const uint8_t *data, size_t size, fist_klc_image *out) {
    if (data == NULL || out == NULL || size < HEADER_SIZE || memcmp(data, "KLC1", TAG_SIZE) != 0) {
        return -1;
    }
    const uint32_t width = fist_read_u32le(data + WIDTH_OFFSET);
    const uint32_t height = fist_read_u32le(data + HEIGHT_OFFSET);
    if (width == 0 || height == 0 || width % BLOCK_SIDE != 0 || height % BLOCK_SIDE != 0 ||
        (size_t)height > SIZE_MAX / width) {
        return -1;
    }
    const fist_asset_view input = {data + HEADER_SIZE, size - HEADER_SIZE};
    fist_klc_image image = {.width = width, .height = height};
    /* Validate the complete stream before allocating from untrusted dimensions. */
    if (decode_blocks(input, &image) != 0) {
        return -1;
    }
    image.pixels = malloc((size_t)width * height);
    if (image.pixels == NULL) {
        return -1;
    }
    if (decode_blocks(input, &image) != 0) {
        fist_klc_destroy(&image);
        return -1;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        image.palette[index] = data[PALETTE_OFFSET + index];
    }
    *out = image;
    return 0;
}

void fist_klc_destroy(fist_klc_image *image) {
    if (image != NULL) {
        free(image->pixels);
        *image = (fist_klc_image){0};
    }
}
