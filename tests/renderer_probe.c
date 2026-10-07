#include "render/renderer.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include "platform/wasm/present.h"
#endif

enum { PROBE_WIDTH = 320, PROBE_HEIGHT = 200, RGBA_CHANNELS = 4, OPAQUE_ALPHA = 255 };

int main(void) {
    fist_renderer *renderer = fist_renderer_create(PROBE_WIDTH, PROBE_HEIGHT);
    if (renderer == NULL) {
        return EXIT_FAILURE;
    }
    if (fist_renderer_draw_probe(renderer) != 0) {
        fist_renderer_destroy(renderer);
        return EXIT_FAILURE;
    }
    const uint8_t *pixels = fist_renderer_pixels(renderer);
    const size_t bytes = (size_t)PROBE_WIDTH * PROBE_HEIGHT * RGBA_CHANNELS;
    const size_t center =
        ((size_t)(PROBE_HEIGHT / 2) * PROBE_WIDTH + PROBE_WIDTH / 2) * RGBA_CHANNELS;
    const uint32_t fnv_offset = UINT32_C(2166136261);
    const uint32_t fnv_prime = UINT32_C(16777619);
    uint32_t checksum = fnv_offset;
    if (pixels == NULL || pixels[0] != 0 || pixels[1] != 0 || pixels[2] != 0 ||
        pixels[3] != OPAQUE_ALPHA ||
        (pixels[center] == 0 && pixels[center + 1] == 0 && pixels[center + 2] == 0)) {
        fist_renderer_destroy(renderer);
        return EXIT_FAILURE;
    }
    for (size_t index = 0; index < bytes; ++index) {
        if (index % RGBA_CHANNELS == RGBA_CHANNELS - 1 && pixels[index] != OPAQUE_ALPHA) {
            fist_renderer_destroy(renderer);
            return EXIT_FAILURE;
        }
        checksum = (checksum ^ pixels[index]) * fnv_prime;
    }
#ifdef __EMSCRIPTEN__
    fist_present_rgba(pixels, PROBE_WIDTH, PROBE_HEIGHT);
#endif
    printf("softgl integration: %dx%d RGBA8 fnv1a=%08" PRIx32 "\n", PROBE_WIDTH, PROBE_HEIGHT,
           checksum);
    fist_renderer_destroy(renderer);
    return EXIT_SUCCESS;
}
