#include "sim/collision.h"

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    COLLIDABLE_FLAG = 64,
    RANGE_MARGIN = 256,
    PROBABILISTIC_HEIGHT = 512,
    PROBABILISTIC_TYPE_A = 5,
    PROBABILISTIC_TYPE_B = 6,
    TYPE26_TYPE = 26,
    TYPE26_MODES = 8,
    REDUCED_HEIGHT_FLAG = 4,
    REDUCED_HEIGHT_DIVISOR = 4,
    ASPECT_SHIFT = 12
};

/* Original e518 dispatch: only these classes have deterministic height gates.
 * Type 21's method is the clc/ret at 9c97, not the separate 9c99 method. */
static const uint16_t height_limits[FIST_UNIT_TYPE_COUNT] = {
    2048, 2304, 2048, 1792, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0,
    0,    0,    0,    0,    0, 0, 0, 0, 0, 1280, 0, 0, 0, 3072};

/* Complete 9ebf..9ece table: destroyed type-26 modes retain the height
 * of their corresponding live subtype. Following 9ecf begins the next table. */
static const uint16_t type26_heights[TYPE26_MODES] = {4096, 4608, 2816, 3328,
                                                      4096, 4608, 2816, 3328};

/* Type-5/6 handler a0a1 indexes 95e4 by the SOURCE type, not the candidate. */
static const uint8_t hit_thresholds[FIST_UNIT_TYPE_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 128, 38, 38, 38, 76, 38, 38, 0, 253, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int fist_collision_world_is_valid(const fist_collision_world *world) {
    if (world == NULL || !fist_object_pool_is_valid(world->pool) || world->bodies == NULL ||
        world->body_count != FIST_UNIT_REGISTRY_COUNT) {
        return 0;
    }
    for (size_t index = 0; index < world->body_count; ++index) {
        const fist_pool_slot slot = world->pool->slots[index];
        if (slot.used != 0 &&
            (world->bodies[index].pose == NULL ||
             (slot.type == TYPE26_TYPE && world->bodies[index].mode >= TYPE26_MODES))) {
            return 0;
        }
    }
    return 1;
}

static uint32_t magnitude_difference(int32_t source, int32_t target) {
    const uint32_t difference = (uint32_t)source - (uint32_t)target;
    return difference <= INT32_MAX ? difference : 0U - difference;
}

static int in_range(const fist_object_pose *source, const fist_collision_body *target) {
    const uint16_t bound = (uint16_t)(target->projection_scale + RANGE_MARGIN);
    return magnitude_difference(source->x, target->pose->x) <= bound &&
           magnitude_difference(source->y, target->pose->y) <= bound;
}

static int interacts(const fist_collision_body *source, uint16_t source_type,
                     const fist_collision_body *target, uint16_t target_type, fist_random *random) {
    const uint32_t height = (uint32_t)source->pose->altitude - (uint32_t)target->pose->altitude;
    if (target_type == PROBABILISTIC_TYPE_A || target_type == PROBABILISTIC_TYPE_B) {
        uint16_t sample = 0;
        if (fist_random_next(random, &sample) != 0) {
            return -1;
        }
        return (uint8_t)sample < hit_thresholds[source_type] &&
               (height <= PROBABILISTIC_HEIGHT || height >= 0U - PROBABILISTIC_HEIGHT);
    }
    uint16_t limit = height_limits[target_type];
    if (target_type == TYPE26_TYPE) {
        limit = type26_heights[target->mode];
        if ((source->mode & REDUCED_HEIGHT_FLAG) != 0) {
            limit /= REDUCED_HEIGHT_DIVISOR;
        }
    }
    return height < limit;
}

int fist_collision_find(const fist_collision_world *world, uint16_t source_slot,
                        fist_random *random, fist_collision_hit *out) {
    if (!fist_collision_world_is_valid(world) || random == NULL || out == NULL ||
        random->next_stream >= FIST_RANDOM_STREAMS || source_slot >= FIST_UNIT_REGISTRY_COUNT ||
        world->pool->slots[source_slot].used == 0) {
        return -1;
    }
    fist_random advanced = *random;
    fist_collision_hit result = {FIST_POOL_NO_SLOT, FIST_POOL_NO_SLOT, 0, 0};
    const fist_collision_body *source = &world->bodies[source_slot];
    const uint16_t source_type = world->pool->slots[source_slot].type;
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const fist_pool_entry entry = world->pool->registry[index];
        if (entry.slot == FIST_POOL_NO_SLOT || entry.slot == source_slot) {
            continue;
        }
        const fist_collision_body *target = &world->bodies[entry.slot];
        if ((target->flags & COLLIDABLE_FLAG) == 0 || !in_range(source->pose, target)) {
            continue;
        }
        const int hit =
            interacts(source, source_type, target, world->pool->slots[entry.slot].type, &advanced);
        if (hit < 0) {
            return -1;
        }
        if (hit != 0) {
            const uint16_t relative =
                (uint16_t)(0U - source->pose->heading - target->pose->heading);
            result = (fist_collision_hit){entry.slot, (uint16_t)index, entry.value,
                                          (uint8_t)(relative >> ASPECT_SHIFT)};
            break;
        }
    }
    *random = advanced;
    *out = result;
    return 0;
}
