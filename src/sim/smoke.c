#include "sim/smoke.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/smoke_animation.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    SMOKE_TYPE = 17,
    SMOKE_ENABLED = 1,
    EXTENT_OFFSET = 18,
    SCALE_OFFSET = 20,
    FLAGS_OFFSET = 22,
    SECONDARY_OFFSET = 23,
    HEIGHT_OFFSET = 24,
    FRAME_OFFSET = 25,
    COUNTER_OFFSET = 26,
    SMOKE_HEIGHT = 768,
    EXTENT_RANDOM_MASK = 63,
    SCALE_SHIFT = 2,
    RISE = 8,
    DELETED_FLAG = 1,
    PERIOD = 12,
    LAST_FRAME = 30
};

int fist_drifting_smoke_restore(const fist_unit_definition *definition,
                                fist_pool_allocation allocation, fist_drifting_smoke *out) {
    if (definition == NULL || out == NULL || definition->type != SMOKE_TYPE ||
        allocation.type != SMOKE_TYPE || allocation.slot >= FIST_POOL_SHORT_SLOTS ||
        allocation.registry_index != definition->registry_index ||
        allocation.registry_index >= FIST_UNIT_REGISTRY_COUNT ||
        allocation.value != definition->generation || definition->snapshot.data == NULL ||
        definition->snapshot.size != FIST_UNIT_SHORT_SIZE ||
        fist_read_u16le(definition->snapshot.data) != SMOKE_TYPE) {
        return -1;
    }
    const uint8_t *raw = definition->snapshot.data;
    *out = (fist_drifting_smoke){
        .allocation = allocation,
        .pose = {definition->map_x, definition->map_y, definition->altitude, definition->heading},
        .extent = fist_read_u16le(raw + EXTENT_OFFSET),
        .projection_scale = fist_read_u16le(raw + SCALE_OFFSET),
        .animation_counter = fist_read_u16le(raw + COUNTER_OFFSET),
        .animation_frame = raw[FRAME_OFFSET],
        .flags = raw[FLAGS_OFFSET],
        .secondary_flags = raw[SECONDARY_OFFSET],
        .ground_height = raw[HEIGHT_OFFSET]};
    return 0;
}

int fist_drifting_smoke_create(fist_object_pool *pool, fist_random *random,
                               const fist_object_pose *pose, fist_smoke_creation request,
                               fist_drifting_smoke *out) {
    if (!fist_object_pool_is_valid(pool) || random == NULL ||
        random->next_stream >= FIST_RANDOM_STREAMS || pose == NULL || out == NULL) {
        return -1;
    }
    if (request.enabled != SMOKE_ENABLED) {
        return FIST_POOL_UNAVAILABLE;
    }
    /* Preserve emitter lifetime before allocation can reuse its world slot. */
    const fist_object_pose source = *pose;
    fist_pool_allocation allocation = {0};
    const int status =
        fist_object_pool_allocate(pool, (fist_pool_request){SMOKE_TYPE, 1}, &allocation);
    if (status != FIST_POOL_OK) {
        return status;
    }
    uint16_t value = 0;
    (void)fist_random_next(random, &value);
    const uint16_t extent = (uint16_t)(request.extent_base + (value & EXTENT_RANDOM_MASK));
    *out = (fist_drifting_smoke){
        .allocation = allocation,
        .pose = {source.x, source.y, fist_position_add(source.altitude, SMOKE_HEIGHT), 0},
        .extent = extent,
        .projection_scale = (uint16_t)(extent << SCALE_SHIFT)};
    return 0;
}

static void drift(fist_drifting_smoke *smoke, fist_smoke_weather weather) {
    smoke->pose.x = fist_position_add(smoke->pose.x, weather.wind_x);
    smoke->pose.y = fist_position_add(smoke->pose.y, weather.wind_y);
    const uint16_t low = (uint16_t)smoke->pose.altitude;
    const uint16_t raised = (uint16_t)(low + RISE);
    smoke->pose.altitude = fist_position_add(smoke->pose.altitude, (int32_t)raised - low);
}

int fist_drifting_smoke_advance(fist_object_pool *pool, fist_drifting_smoke *smoke,
                                fist_smoke_weather weather) {
    if (smoke == NULL || smoke->allocation.type != SMOKE_TYPE ||
        !fist_object_pool_is_current(pool, smoke->allocation)) {
        return -1;
    }
    fist_drifting_smoke updated = *smoke;
    int released = weather.enabled != SMOKE_ENABLED;
    if (!released) {
        drift(&updated, weather);
        const fist_smoke_animation_rule rule = {PERIOD, LAST_FRAME};
        released =
            fist_smoke_animation_step(&updated.animation_counter, &updated.animation_frame, rule);
    }
    if (released) {
        fist_pool_allocation removed = {0};
        if (fist_object_pool_release(pool, updated.allocation.registry_index, &removed) != 0) {
            return -1;
        }
        updated.flags |= DELETED_FLAG;
    }
    *smoke = updated;
    return 0;
}
