#include "assets/palette.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "render/model_bitmap.h"
#include "render/renderer.h"
#include "render/terrain_scene.h"
#include "render/vehicle_scene.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"
#include <math.h>

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

static int check_follow_camera(void) {
    static const float height_tolerance = 0.001F;
    enum {
        HILL_SIDE = 256,
        HILL_START = 8,
        PLAYER_HEIGHT = 10,
        HILL_HEIGHT = 250,
        FOLLOW_CLEARANCE = 32
    };
    uint8_t *heights = calloc((size_t)HILL_SIDE * HILL_SIDE, 1);
    if (heights == NULL) {
        return -1;
    }
    for (size_t row = 0; row < HILL_SIDE; ++row) {
        for (size_t column = 0; column < HILL_SIDE; ++column) {
            heights[(row * HILL_SIDE) + column] = row >= HILL_START ? HILL_HEIGHT : PLAYER_HEIGHT;
        }
    }
    const fist_terrain terrain = {
        .heightmap = {.width = HILL_SIDE, .height = HILL_SIDE, .pixels = heights}};
    const fist_vehicle_state player = {.type = 0};
    const fist_unit_definition definition = {.type = 0};
    fist_terrain_view live = {0};
    fist_terrain_view reference = {0};
    int result = -1;
    if (fist_vehicle_follow_view(&terrain, &player, &live) == 0 &&
        fist_vehicle_inspection_view(&terrain, &definition, 0, &reference) == 0) {
        const float ground = fist_terrain_surface(&terrain, live.map_x, live.map_y);
        const float altitude = fist_terrain_surface(&terrain, player.map_x, player.map_y);
        const float aimed_height = live.altitude + (tanf(live.pitch) * CAMERA_DISTANCE);
        /* A reached hill places the old camera below its own terrain sample.
         * Both immutable/live adapters must clear it and aim at the actor. */
        if (ground > altitude + FOLLOW_CLEARANCE && live.altitude >= ground + FOLLOW_CLEARANCE &&
            fabsf(aimed_height - altitude) < height_tolerance && live.map_x == reference.map_x &&
            live.map_y == reference.map_y && live.altitude == reference.altitude &&
            live.pitch == reference.pitch) {
            result = 0;
        }
    }
    free(heights);
    return result;
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
        check_vehicle(renderer, &terrain, &view, background) == 0 && check_follow_camera() == 0) {
        puts("vehicle scene: complete transparency, terrain depth, vertical orientation and state "
             "reuse and hill-safe follow camera pass");
        result = EXIT_SUCCESS;
    }
    free(background);
    fist_renderer_destroy(renderer);
    return result;
}
