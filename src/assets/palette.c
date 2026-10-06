#include "assets/palette.h"

#include <stddef.h>
#include <stdint.h>

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
