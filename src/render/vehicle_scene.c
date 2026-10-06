#include "render/vehicle_scene.h"

#include "assets/model.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "assets/vehicle.h"
#include "render/model_bitmap.h"
#include "render/renderer.h"
#include "render/terrain_scene.h"
#include "sim/ground.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

static const float full_turn = 6.2831853071795864769F;

static int follow_view(const fist_terrain *terrain, const fist_ground_pose *pose,
                       fist_terrain_view *out) {
    if (terrain == NULL || terrain->heightmap.pixels == NULL || terrain->heightmap.width == 0 ||
        pose == NULL || out == NULL) {
        return -1;
    }
    /* Preview framing choices, not physical vehicle constants. */
    static const float follow_distance = 64.0F;
    static const float follow_height = 32.0F;
    const float angle = (float)pose->heading * full_turn / FIST_TURN_SIZE;
    const int32_t back_x = (int32_t)lroundf(sinf(angle) * follow_distance * FIST_POSITION_SCALE);
    const int32_t back_y = (int32_t)lroundf(cosf(angle) * follow_distance * FIST_POSITION_SCALE);
    /* Reduce before subtracting so all signed original positions remain safe. */
    const int32_t map_x = (int32_t)((uint32_t)pose->map_x % FIST_MAP_PERIOD) - back_x;
    const int32_t map_y = (int32_t)((uint32_t)pose->map_y % FIST_MAP_PERIOD) - back_y;
    const float vehicle_height = fist_terrain_surface(terrain, pose->map_x, pose->map_y);
    const float camera_ground = fist_terrain_surface(terrain, map_x, map_y);
    const float altitude = fmaxf(vehicle_height, camera_ground) + follow_height;
    *out = (fist_terrain_view){.map_x = map_x,
                               .map_y = map_y,
                               .altitude = altitude,
                               .heading = pose->heading,
                               .pitch = -atan2f(altitude - vehicle_height, follow_distance)};
    return 0;
}

static uint16_t view_bearing(const fist_terrain_view *view, const fist_ground_pose *pose) {
    const float map_x = (float)fist_map_delta(pose->map_x, view->map_x);
    const float map_y = (float)fist_map_delta(pose->map_y, view->map_y);
    const float angle = atan2f(map_x, map_y);
    /* Original 0731/059a uses a coarse atan table. Use a continuous bearing
     * with the same axes, then retain the original nearest-32-facing rule. */
    const int32_t turn = (int32_t)lroundf(angle * FIST_TURN_SIZE / full_turn);
    return (uint16_t)turn;
}

typedef struct {
    uint8_t type;
    fist_ground_pose pose;
    fist_vehicle_visual visual;
} rendered_vehicle;

static int prepare_vehicle(const fist_terrain *terrain, const rendered_vehicle *rendered,
                           const fist_model *model, const fist_terrain_view *view,
                           fist_scene_vehicle *out) {
    enum { SPRITE_CAMERA_SCALE = 176, WORLD_TEXEL_DENOMINATOR = 131072, HORIZONTAL_DIVISOR = 2 };
    /* Executed original 15f15..15fc9 C-family entries, posted by 3ed4 to TCB480.
     * de70 supplies TCBca=176; ad74 and bb71 give their world projection ratio. */
    static const uint16_t scales[FIST_UNIT_GROUND_VEHICLE_COUNT] = {272, 272, 336, 304};
    if (terrain == NULL || terrain->heightmap.pixels == NULL || terrain->heightmap.width == 0 ||
        model == NULL || view == NULL || out == NULL) {
        return -1;
    }
    fist_model_pose poses[FIST_VEHICLE_MODEL_PARTS] = {0};
    const uint16_t bearing = view_bearing(view, &rendered->pose);
    for (size_t part = 0; part < FIST_VEHICLE_MODEL_PARTS; ++part) {
        if (fist_vehicle_part_pose(part, &rendered->visual, bearing, &poses[part]) != 0) {
            return -1;
        }
    }
    fist_scene_vehicle vehicle = {.map_x = rendered->pose.map_x, .map_y = rendered->pose.map_y};
    const fist_model_selection selection = {.poses = poses, .count = FIST_VEHICLE_MODEL_PARTS};
    if (fist_model_compose(model, &selection, &vehicle.bitmap) != 0) {
        return -1;
    }
    vehicle.altitude = fist_terrain_surface(terrain, vehicle.map_x, vehicle.map_y);
    vehicle.texel_height =
        (float)scales[rendered->type] * SPRITE_CAMERA_SCALE / WORLD_TEXEL_DENOMINATOR;
    vehicle.texel_width = vehicle.texel_height / HORIZONTAL_DIVISOR;
    *out = vehicle;
    return 0;
}

int fist_vehicle_inspection_view(const fist_terrain *terrain, const fist_unit_definition *vehicle,
                                 uint16_t heading, fist_terrain_view *out) {
    if (vehicle == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    const fist_ground_pose pose = {vehicle->map_x, vehicle->map_y, heading};
    return follow_view(terrain, &pose, out);
}

int fist_vehicle_follow_view(const fist_terrain *terrain, const fist_vehicle_state *state,
                             fist_terrain_view *out) {
    if (state == NULL || state->type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    const fist_ground_pose pose = {state->map_x, state->map_y, state->drive.heading};
    return follow_view(terrain, &pose, out);
}

int fist_scene_vehicle_prepare(const fist_terrain *terrain, const fist_unit_definition *definition,
                               const fist_model *model, const fist_terrain_view *view,
                               fist_scene_vehicle *out) {
    rendered_vehicle rendered = {0};
    if (fist_vehicle_visual_decode(definition, &rendered.visual) != 0) {
        return -1;
    }
    rendered.type = definition->type;
    rendered.pose = (fist_ground_pose){definition->map_x, definition->map_y, 0};
    return prepare_vehicle(terrain, &rendered, model, view, out);
}

int fist_scene_actor_prepare(const fist_terrain *terrain, const fist_scene_actor *actor,
                             const fist_terrain_view *view, fist_scene_vehicle *out) {
    if (actor == NULL || actor->state == NULL || actor->visual == NULL ||
        actor->state->type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    const fist_vehicle_state *state = actor->state;
    rendered_vehicle rendered = {.type = state->type,
                                 .pose = {state->map_x, state->map_y, state->drive.heading},
                                 .visual = *actor->visual};
    rendered.visual.headings[0] = state->drive.heading;
    rendered.visual.headings[1] = state->turret.heading;
    for (size_t part = 0; part < FIST_VEHICLE_MODEL_PARTS; ++part) {
        rendered.visual.parts[part] = state->animation_selectors[part] ^ FIST_MODEL_PART_VARIANTS;
    }
    return prepare_vehicle(terrain, &rendered, actor->model, view, out);
}
