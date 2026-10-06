#include "sim/destruction_updates.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    WRECK_TYPE = 23,
    TARGET_TYPE = 26,
    ARTILLERY_TYPE = 27,
    TARGET_DEAD_FIRST = 4,
    TARGET_DEAD_LAST = 7,
    TARGET_DEAD_SECOND = 6,
    COLLIDABLE_FLAG = 64,
    WRECK_SECONDARY_FLAGS = 68,
    SIDE_FLAG = 8,
    DELETED_FLAG = 1,
    ARTILLERY_CLEAR_FLAGS = 6,
    ARTILLERY_CLEAR_SECONDARY = 24,
    EMISSION_MASK = 63,
    MINIMUM_PARAMETER = 128,
    FAST_DECAY_THRESHOLD = 768,
    FAST_DECAY = 4,
    MODEL_OFFSET = 27,
    CLASS_OFFSET = 29,
    COUNTER_OFFSET = 31,
    PARAMETER_OFFSET = 33,
    PLATOON_OFFSET = 35,
    MEMBER_OFFSET = 36,
    FLAGS_OFFSET = 22,
    SECONDARY_OFFSET = 23,
    SCALE_OFFSET = 20
};

int fist_vehicle_wreck_restore(const fist_unit_definition *definition,
                               fist_pool_allocation allocation, fist_vehicle_wreck *out) {
    if (definition == NULL || out == NULL || definition->type != WRECK_TYPE ||
        allocation.type != WRECK_TYPE || allocation.slot >= FIST_POOL_SHORT_SLOTS ||
        allocation.registry_index != definition->registry_index ||
        allocation.registry_index >= FIST_UNIT_REGISTRY_COUNT ||
        allocation.value != definition->generation || definition->snapshot.data == NULL ||
        definition->snapshot.size != FIST_UNIT_SHORT_SIZE ||
        fist_read_u16le(definition->snapshot.data) != WRECK_TYPE) {
        return -1;
    }
    const uint8_t *raw = definition->snapshot.data;
    *out = (fist_vehicle_wreck){
        .allocation = allocation,
        .pose = {definition->map_x, definition->map_y, definition->altitude, definition->heading},
        .model_code = fist_read_u16le(raw + MODEL_OFFSET),
        .original_type = fist_read_u16le(raw + CLASS_OFFSET),
        .projection_scale = fist_read_u16le(raw + SCALE_OFFSET),
        .parameter = fist_read_u16le(raw + PARAMETER_OFFSET),
        .platoon = raw[PLATOON_OFFSET],
        .member = raw[MEMBER_OFFSET],
        .flags = raw[FLAGS_OFFSET],
        .secondary_flags = raw[SECONDARY_OFFSET],
        .emission_counter = fist_read_u16le(raw + COUNTER_OFFSET)};
    return 0;
}

static int environment_valid(const fist_destruction_environment *environment) {
    return environment != NULL && fist_object_pool_is_valid(environment->pool) &&
           environment->random != NULL && environment->random->next_stream < FIST_RANDOM_STREAMS;
}

static int emit(const fist_object_pose *pose, uint16_t parameter,
                const fist_destruction_environment *environment, fist_destruction_step *result) {
    const fist_smoke_creation request = {parameter, environment->smoke_enabled};
    const int status = fist_drifting_smoke_create(environment->pool, environment->random, pose,
                                                  request, &result->smoke);
    result->has_smoke = status == FIST_POOL_OK;
    return status < 0 ? -1 : 0;
}

int fist_vehicle_wreck_advance(fist_vehicle_wreck *wreck,
                               const fist_destruction_environment *environment,
                               fist_destruction_step *out) {
    if (wreck == NULL || out == NULL || wreck->allocation.type != WRECK_TYPE ||
        !environment_valid(environment) ||
        !fist_object_pool_is_current(environment->pool, wreck->allocation)) {
        return -1;
    }
    fist_vehicle_wreck updated = *wreck;
    fist_object_pool pool = *environment->pool;
    fist_random random = *environment->random;
    const fist_destruction_environment staged = {&pool, &random, environment->smoke_enabled};
    fist_destruction_step result = {0};
    updated.flags |= COLLIDABLE_FLAG;
    updated.secondary_flags |= WRECK_SECONDARY_FLAGS;
    updated.emission_counter = (uint16_t)(updated.emission_counter + 1);
    if ((updated.emission_counter & EMISSION_MASK) == 0 && updated.parameter > MINIMUM_PARAMETER &&
        emit(&updated.pose, updated.parameter, &staged, &result) != 0) {
        return -1;
    }
    *wreck = updated;
    *environment->pool = pool;
    *environment->random = random;
    *out = result;
    return 0;
}

static int target_step(fist_other_actor *actor, const fist_destruction_environment *environment,
                       fist_destruction_step *result) {
    if (actor->mode != TARGET_DEAD_FIRST && actor->mode != TARGET_DEAD_SECOND) {
        actor->flags |= SIDE_FLAG;
        return 0;
    }
    fist_type26_state *state = &actor->state.type26;
    state->emission_counter = (uint8_t)(state->emission_counter + 1);
    if ((state->emission_counter & EMISSION_MASK) != 0 ||
        state->destruction_parameter <= MINIMUM_PARAMETER) {
        return 0;
    }
    if (emit(&actor->pose, state->destruction_parameter, environment, result) != 0) {
        return -1;
    }
    --state->destruction_parameter;
    if (state->destruction_parameter > FAST_DECAY_THRESHOLD) {
        state->destruction_parameter = (uint16_t)(state->destruction_parameter - FAST_DECAY);
    }
    return 0;
}

static int artillery_step(fist_other_actor *actor, const fist_destruction_environment *environment,
                          fist_destruction_step *result) {
    fist_type27_state *state = &actor->state.type27;
    state->emission_counter = (uint16_t)(state->emission_counter + 1);
    if ((state->emission_counter & EMISSION_MASK) == 0 &&
        state->debris_parameter > MINIMUM_PARAMETER) {
        if (emit(&actor->pose, state->debris_parameter, environment, result) != 0) {
            return -1;
        }
        --state->debris_parameter;
    }
    if (actor->mode == 1) {
        actor->flags = (uint8_t)((actor->flags & (uint8_t)~ARTILLERY_CLEAR_FLAGS) | DELETED_FLAG);
        actor->secondary_flags &= (uint8_t)~ARTILLERY_CLEAR_SECONDARY;
    }
    return 0;
}

int fist_destroyed_target_advance(fist_other_actor *actor,
                                  const fist_destruction_environment *environment,
                                  fist_destruction_step *out) {
    if (actor == NULL || out == NULL || !environment_valid(environment) ||
        (actor->allocation.type != ARTILLERY_TYPE &&
         (actor->allocation.type != TARGET_TYPE || actor->mode < TARGET_DEAD_FIRST ||
          actor->mode > TARGET_DEAD_LAST)) ||
        !fist_object_pool_is_current(environment->pool, actor->allocation)) {
        return -1;
    }
    fist_other_actor updated = *actor;
    fist_object_pool pool = *environment->pool;
    fist_random random = *environment->random;
    const fist_destruction_environment staged = {&pool, &random, environment->smoke_enabled};
    fist_destruction_step result = {0};
    const int status = updated.allocation.type == TARGET_TYPE
                           ? target_step(&updated, &staged, &result)
                           : artillery_step(&updated, &staged, &result);
    if (status != 0) {
        return -1;
    }
    *actor = updated;
    *environment->pool = pool;
    *environment->random = random;
    *out = result;
    return 0;
}
