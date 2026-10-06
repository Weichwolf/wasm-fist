#include "sim/other_damage.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/damage_common.h"
#include "sim/object_pool.h"
#include "sim/projectile_flight.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    PAIR_FIRST = 5,
    PAIR_SECOND = 6,
    WRECK_TYPE = 23,
    TYPE26_TYPE = 26,
    LAST_TYPE = 27,
    FLAGS_OFFSET = 22,
    SECONDARY_OFFSET = 23,
    GROUND_OFFSET = 24,
    MODE_OFFSET = 25,
    DAMAGE_OFFSET = 26,
    LIMIT_OFFSET = 27,
    DESTRUCTION_OFFSET = 28,
    PAIR_SPEED_OFFSET = 27,
    PAIR_COUNTDOWN_OFFSET = 35,
    PAIR_HEADING_OFFSET = 46,
    PAIR_MOTION_HEADING_OFFSET = 48,
    PAIR_ROTOR_OFFSET = 26,
    TYPE26_EMISSION_OFFSET = 30,
    TYPE27_EMISSION_OFFSET = 27,
    PARAMETER_OFFSET = 29,
    COUNTER_OFFSET = 31,
    BEHAVIOR_OFFSET = 37,
    FRAME_OFFSET = 50,
    PAIR_DAMAGE_OFFSET = 51,
    EXTENT_OFFSET = 18,
    SCALE_OFFSET = 20,
    DELETED_FLAG = 1,
    SIDE_FLAG = 8,
    REACTED_FLAG = 2,
    PAIR_DEATH_BEHAVIOR = 12,
    PAIR_REACTION_BEHAVIOR = 4,
    PAIR_REACTION_PARAMETER = 56,
    PAIR_DEATH_PARAMETER = 32,
    PAIR_DAMAGE_LIMIT = 100,
    LAST_DAMAGE_LIMIT = 80,
    PAIR_RELEASE_THRESHOLD = 128,
    TYPE26_DESTROYED = 4,
    TYPE26_MODES = 8,
    MOTION_FLAGS = 6,
    TYPE26_SECONDARY_CLEAR = 8,
    LAST_SECONDARY_CLEAR = 24,
    TYPE26_DEATH_PARAMETER = 1152,
    LAST_DEATH_PARAMETER = 768,
    LAST_DEATH_EXTENT = 320,
    BYTE_RANGE = 256,
    FRIENDLY_REACTION_VOICE = 38,
    DESTRUCTION_SOUND = 9
};

static int pair_type(uint16_t type) {
    return type == PAIR_FIRST || type == PAIR_SECOND;
}

static int supported_type(uint16_t type) {
    return pair_type(type) || type == TYPE26_TYPE || type == LAST_TYPE;
}

int fist_other_actor_restore(const fist_unit_definition *definition,
                             fist_pool_allocation allocation, fist_other_actor *out) {
    if (definition == NULL || out == NULL || !supported_type(definition->type) ||
        allocation.type != definition->type || allocation.slot >= FIST_POOL_SHORT_SLOTS ||
        allocation.registry_index != definition->registry_index ||
        allocation.registry_index >= FIST_UNIT_REGISTRY_COUNT ||
        allocation.value != definition->generation || definition->snapshot.data == NULL ||
        definition->snapshot.size != FIST_UNIT_SHORT_SIZE ||
        fist_read_u16le(definition->snapshot.data) != definition->type) {
        return -1;
    }
    const uint8_t *raw = definition->snapshot.data;
    fist_other_actor actor = {
        .allocation = allocation,
        .pose = {definition->map_x, definition->map_y, definition->altitude, definition->heading},
        .projection_extent = fist_read_u16le(raw + EXTENT_OFFSET),
        .projection_scale = fist_read_u16le(raw + SCALE_OFFSET),
        .flags = raw[FLAGS_OFFSET],
        .secondary_flags = raw[SECONDARY_OFFSET],
        .ground_height = raw[GROUND_OFFSET],
        .mode = raw[MODE_OFFSET]};
    if (pair_type(allocation.type)) {
        actor.state.pair = (fist_pair_actor_state){
            .target_speed = fist_read_i16le(raw + PARAMETER_OFFSET),
            .behavior = raw[BEHAVIOR_OFFSET],
            .altitude_offset = raw[FRAME_OFFSET],
            .damage = raw[PAIR_DAMAGE_OFFSET],
            .speed = fist_read_i16le(raw + PAIR_SPEED_OFFSET),
            .behavior_countdown = fist_read_u16le(raw + PAIR_COUNTDOWN_OFFSET),
            .target_heading = fist_read_u16le(raw + PAIR_HEADING_OFFSET),
            .motion_heading = raw[PAIR_MOTION_HEADING_OFFSET],
            .rotor_frame = raw[PAIR_ROTOR_OFFSET]};
    } else if (allocation.type == TYPE26_TYPE) {
        if (actor.mode >= TYPE26_MODES) {
            return -1;
        }
        actor.state.type26 = (fist_type26_state){raw[DAMAGE_OFFSET], raw[LIMIT_OFFSET],
                                                 fist_read_u16le(raw + DESTRUCTION_OFFSET),
                                                 raw[TYPE26_EMISSION_OFFSET]};
    } else {
        actor.state.type27 = (fist_type27_state){
            raw[DAMAGE_OFFSET], fist_read_u16le(raw + PARAMETER_OFFSET),
            fist_read_u16le(raw + COUNTER_OFFSET), fist_read_u16le(raw + TYPE27_EMISSION_OFFSET)};
    }
    *out = actor;
    return 0;
}

static int valid_request(const fist_other_actor *actor, const fist_damage_environment *environment,
                         fist_vehicle_damage_request request) {
    return actor != NULL && supported_type(actor->allocation.type) &&
           (actor->allocation.type != TYPE26_TYPE || actor->mode < TYPE26_MODES) &&
           fist_damage_source_is_valid(environment, request) &&
           actor->allocation.slot == request.hit.slot &&
           actor->allocation.registry_index == request.hit.registry_index &&
           actor->allocation.value == request.hit.value &&
           fist_object_pool_is_current(environment->pool, actor->allocation);
}

static int create_effect(const fist_object_pose *pose, fist_object_pool *pool, uint8_t template_id,
                         fist_other_damage_result *result) {
    const int status = fist_explosion_create(pool, pose, template_id, &result->explosion);
    if (status < 0) {
        return -1;
    }
    result->has_explosion = status == FIST_POOL_OK;
    result->sound_request = DESTRUCTION_SOUND;
    return 0;
}

static int pair_damage(fist_other_actor *actor, const fist_damage_environment *environment,
                       fist_vehicle_damage_request request, fist_other_damage_result *result) {
    fist_pair_actor_state *state = &actor->state.pair;
    if (state->behavior == PAIR_DEATH_BEHAVIOR) {
        return 0;
    }
    static const uint8_t record[] = {90, 50};
    const uint16_t rolled = fist_damage_base_roll(environment->random, record);
    result->applied_damage =
        fist_damage_scale_source(rolled, environment->state, request.projectile);
    /* a0c8 compares the wrapped byte only; it deliberately ignores carry. */
    state->damage = (uint8_t)(state->damage + result->applied_damage);
    if (state->damage < PAIR_DAMAGE_LIMIT) {
        if ((actor->flags & SIDE_FLAG) == 0 && (actor->secondary_flags & REACTED_FLAG) == 0) {
            actor->secondary_flags |= REACTED_FLAG;
            state->behavior = PAIR_REACTION_BEHAVIOR;
            state->target_speed = PAIR_REACTION_PARAMETER;
            result->voice_request = FRIENDLY_REACTION_VOICE;
        }
        return 0;
    }
    result->destroyed = true;
    const size_t side = (actor->flags & SIDE_FLAG) != 0;
    environment->state->pair_destroyed_by_side[side] =
        (uint16_t)(environment->state->pair_destroyed_by_side[side] + 1);
    if ((uint8_t)fist_damage_next_random(environment->random) < PAIR_RELEASE_THRESHOLD) {
        state->behavior = PAIR_DEATH_BEHAVIOR;
        state->target_speed = PAIR_DEATH_PARAMETER;
        state->altitude_offset = 0;
        return 0;
    }
    if (create_effect(&actor->pose, environment->pool, FIST_EXPLOSION_PAIR_DESTRUCTION, result) !=
        0) {
        return -1;
    }
    fist_pool_allocation released = {0};
    if (fist_object_pool_release(environment->pool, actor->allocation.registry_index, &released) !=
        0) {
        return -1;
    }
    actor->flags |= DELETED_FLAG;
    result->released = true;
    return 0;
}

static int type26_damage(fist_other_actor *actor, const fist_damage_environment *environment,
                         fist_vehicle_damage_request request, fist_other_damage_result *result) {
    if ((actor->mode & TYPE26_DESTROYED) != 0) {
        return 0;
    }
    static const uint8_t records[][2] = {{100, 40}, {60, 40}, {100, 40}, {40, 30}};
    static const uint16_t dead_scales[] = {640, 640, 896, 1152};
    static const uint8_t templates[] = {FIST_EXPLOSION_TYPE26, FIST_EXPLOSION_SHELL_GROUND,
                                        FIST_EXPLOSION_TYPE26, FIST_EXPLOSION_TYPE26_WIDE};
    const uint8_t subtype = actor->mode;
    const uint16_t rolled = fist_damage_base_roll(environment->random, records[subtype]);
    result->applied_damage =
        fist_damage_scale_source(rolled, environment->state, request.projectile);
    fist_type26_state *state = &actor->state.type26;
    const unsigned total = (unsigned)state->damage + result->applied_damage;
    state->damage = (uint8_t)total;
    if (total < BYTE_RANGE && state->damage < state->limit) {
        return 0;
    }
    actor->flags = (uint8_t)((actor->flags | DELETED_FLAG) & (uint8_t)~MOTION_FLAGS);
    actor->secondary_flags &= (uint8_t)~TYPE26_SECONDARY_CLEAR;
    actor->mode |= TYPE26_DESTROYED;
    state->destruction_parameter = TYPE26_DEATH_PARAMETER;
    actor->projection_scale = dead_scales[subtype];
    result->destroyed = true;
    return create_effect(&actor->pose, environment->pool, templates[subtype], result);
}

static int last_damage(fist_other_actor *actor, const fist_damage_environment *environment,
                       fist_vehicle_damage_request request, fist_other_damage_result *result) {
    if (actor->mode != 0) {
        return 0;
    }
    static const uint8_t record[] = {120, 90};
    const uint16_t rolled = fist_damage_base_roll(environment->random, record);
    result->applied_damage =
        fist_damage_scale_source(rolled, environment->state, request.projectile);
    fist_type27_state *state = &actor->state.type27;
    const unsigned total = (unsigned)state->damage + result->applied_damage;
    state->damage = (uint8_t)total;
    if (total < BYTE_RANGE && state->damage < LAST_DAMAGE_LIMIT) {
        return 0;
    }
    actor->mode = 1;
    state->debris_parameter = LAST_DEATH_PARAMETER;
    state->animation_counter = 0;
    actor->projection_extent = LAST_DEATH_EXTENT;
    actor->flags = (uint8_t)((actor->flags & (uint8_t)~MOTION_FLAGS) | DELETED_FLAG);
    actor->secondary_flags &= (uint8_t)~LAST_SECONDARY_CLEAR;
    result->destroyed = true;
    return create_effect(&actor->pose, environment->pool, FIST_EXPLOSION_VEHICLE_SHOCK, result);
}

int fist_other_damage_m1(fist_other_actor *actor, const fist_damage_environment *environment,
                         fist_vehicle_damage_request request, fist_other_damage_result *out) {
    if (out == NULL || !valid_request(actor, environment, request)) {
        return -1;
    }
    fist_other_actor updated = *actor;
    fist_object_pool pool = *environment->pool;
    fist_combat_state state = *environment->state;
    fist_random random = *environment->random;
    const fist_damage_environment staged = {&pool, &random, &state};
    fist_other_damage_result result = {.sound_request = FIST_DAMAGE_NO_REQUEST,
                                       .voice_request = FIST_DAMAGE_NO_REQUEST,
                                       .refresh_damage_display = true};
    int status = 0;
    if (pair_type(updated.allocation.type)) {
        status = pair_damage(&updated, &staged, request, &result);
    } else if (updated.allocation.type == TYPE26_TYPE) {
        status = type26_damage(&updated, &staged, request, &result);
    } else {
        status = last_damage(&updated, &staged, request, &result);
    }
    if (status != 0) {
        return -1;
    }
    *actor = updated;
    *environment->pool = pool;
    *environment->state = state;
    *environment->random = random;
    *out = result;
    return 0;
}

int fist_wreck_damage_m1(const fist_damage_environment *environment,
                         fist_vehicle_damage_request request, fist_other_damage_result *out) {
    const fist_pool_allocation target = {WRECK_TYPE, request.hit.slot, request.hit.registry_index,
                                         request.hit.value};
    if (out == NULL || !fist_damage_source_is_valid(environment, request) ||
        !fist_object_pool_is_current(environment->pool, target)) {
        return -1;
    }
    *out = (fist_other_damage_result){.sound_request = FIST_DAMAGE_NO_REQUEST,
                                      .voice_request = FIST_DAMAGE_NO_REQUEST,
                                      .refresh_damage_display = true};
    return 0;
}
