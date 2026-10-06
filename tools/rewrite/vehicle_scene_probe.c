#include "assets/palette.h"
#include "assets/terrain.h"
#include "render/model_bitmap.h"
#include "render/renderer.h"
#include "render/terrain_scene.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    WIDTH = 320,
    HEIGHT = 200,
    CHANNELS = 4,
    PLANE_SIDE = 4,
    SPRITE_SIDE = 8,
    FLAT_HEIGHT = 32,
    CAMERA_ALTITUDE = 48,
    CAMERA_DISTANCE = 64,
    BELOW_GROUND = -100,
    TEXEL_SIZE = 2,
    OPAQUE = 255
};

static int draw_copy(fist_renderer *renderer, const fist_terrain *terrain,
                     const fist_terrain_view *view, uint8_t *out) {
    if (fist_renderer_draw_terrain(renderer, terrain, view) != 0) {
        return -1;
    }
    const uint8_t *pixels = fist_renderer_pixels(renderer);
    if (pixels == NULL) {
        return -1;
    }
    for (size_t index = 0; index < (size_t)WIDTH * HEIGHT * CHANNELS; ++index) {
        out[index] = pixels[index];
    }
    return 0;
}

static int compare_frame(fist_renderer *renderer, const uint8_t *expected) {
    const uint8_t *pixels = fist_renderer_pixels(renderer);
    return pixels != NULL && memcmp(pixels, expected, (size_t)WIDTH * HEIGHT * CHANNELS) == 0 ? 0
                                                                                              : -1;
}

static int check_colored_frame(fist_renderer *renderer, const uint8_t *background) {
    const uint8_t *pixels = fist_renderer_pixels(renderer);
    size_t changed = 0;
    size_t red_count = 0;
    size_t blue_count = 0;
    size_t red_rows = 0;
    size_t blue_rows = 0;
    for (size_t index = 0; index < (size_t)WIDTH * HEIGHT; ++index) {
        const uint8_t *pixel = pixels + (index * CHANNELS);
        if (pixel[CHANNELS - 1] != OPAQUE) {
            return -1;
        }
        if (memcmp(pixel, background + (index * CHANNELS), CHANNELS) != 0) {
            ++changed;
        }
        if (pixel[0] == OPAQUE && pixel[1] == 0 && pixel[2] == 0) {
            ++red_count;
            red_rows += index / WIDTH;
        }
        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == OPAQUE) {
            ++blue_count;
            blue_rows += index / WIDTH;
        }
    }
    /* Entire output must preserve all background except actual colored texels.
     * A zero stripe is transparent, and source row 0 is below source row 7. */
    return red_count != 0 && blue_count != 0 && changed == red_count + blue_count &&
                   red_rows * blue_count < blue_rows * red_count
               ? 0
               : -1;
}

static int check_vehicle(fist_renderer *renderer, const fist_terrain *terrain,
                         const fist_terrain_view *view, const uint8_t *background) {
    uint8_t indices[SPRITE_SIDE * SPRITE_SIDE] = {0};
    fist_scene_vehicle vehicle = {.altitude = FLAT_HEIGHT,
                                  .texel_width = TEXEL_SIZE,
                                  .texel_height = TEXEL_SIZE,
                                  .bitmap = {.indices = indices,
                                             .width = SPRITE_SIDE,
                                             .height = SPRITE_SIDE,
                                             .left = -(SPRITE_SIDE / 2)}};
    vehicle.bitmap.palette.rgb6[FIST_PALETTE_CHANNELS] = FIST_PALETTE_DAC_MAX;
    vehicle.bitmap.palette.rgb6[(2 * FIST_PALETTE_CHANNELS) + 2] = FIST_PALETTE_DAC_MAX;
    if (fist_renderer_draw_vehicle(renderer, terrain, view, &vehicle) != 0 ||
        compare_frame(renderer, background) != 0) {
        return -1;
    }
    for (size_t row = 0; row < SPRITE_SIDE; ++row) {
        for (size_t column = 0; column < SPRITE_SIDE; ++column) {
            if (column != SPRITE_SIDE / 2) {
                indices[(row * SPRITE_SIDE) + column] = row < SPRITE_SIDE / 2 ? 1 : 2;
            }
        }
    }
    if (fist_renderer_draw_vehicle(renderer, terrain, view, &vehicle) != 0 ||
        check_colored_frame(renderer, background) != 0) {
        return -1;
    }
    vehicle.altitude = BELOW_GROUND;
    return fist_renderer_draw_vehicle(renderer, terrain, view, &vehicle) == 0 &&
                   compare_frame(renderer, background) == 0 &&
                   fist_renderer_draw_terrain(renderer, terrain, view) == 0 &&
                   compare_frame(renderer, background) == 0
               ? 0
               : -1;
}

int main(void) {
    uint8_t heights[PLANE_SIDE * PLANE_SIDE] = {0};
    uint8_t colors[PLANE_SIDE * PLANE_SIDE] = {0};
    for (size_t index = 0; index < sizeof(heights); ++index) {
        heights[index] = FLAT_HEIGHT;
    }
    fist_terrain terrain = {
        .heightmap = {.width = PLANE_SIDE, .height = PLANE_SIDE, .pixels = heights},
        .colormap = {.width = PLANE_SIDE, .height = PLANE_SIDE, .pixels = colors},
        .sky = {.width = PLANE_SIDE, .height = PLANE_SIDE, .pixels = colors}};
    static const float downward_pitch = -0.25F;
    fist_terrain_view view = {.map_y = -CAMERA_DISTANCE * FIST_POSITION_SCALE,
                              .altitude = CAMERA_ALTITUDE,
                              .pitch = downward_pitch};
    fist_renderer *renderer = fist_renderer_create(WIDTH, HEIGHT);
    uint8_t *background = calloc((size_t)WIDTH * HEIGHT * CHANNELS, 1);
    int result = EXIT_FAILURE;
    if (renderer != NULL && background != NULL &&
        draw_copy(renderer, &terrain, &view, background) == 0 &&
        check_vehicle(renderer, &terrain, &view, background) == 0) {
        puts("vehicle scene: complete transparency, terrain depth, vertical orientation and state "
             "reuse pass");
        result = EXIT_SUCCESS;
    }
    free(background);
    fist_renderer_destroy(renderer);
    return result;
}
