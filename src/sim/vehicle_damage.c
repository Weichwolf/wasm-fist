#include "sim/vehicle_damage.h"

#include "assets/units.h"
#include "sim/collision.h"
#include "sim/damage_common.h"
#include "sim/object_pool.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    SIDE_FLAG = 8,
    DELETED_FLAG = 1,
    HIT_CONTROL_FLAG = 32,
    DAMAGE_LIMIT = 100,
    BYTE_RANGE = 256,
    FIXED_SCALE_SHIFT = 8,
    ASPECTS = 16,
    FIRE_FLAGS = 6,
    FIRE_ROLL_MASK = 120,
    TURRET_FLAG = 16,
    TURRET_ROLL_MASK = 248,
    TRACK_FLAG = 8,
    OPERATION_CLEAR_FLAG = 16,
    OPERATION_STOP_FLAG = 32,
    SECONDARY_FEEDBACK_SUPPRESS = 2,
    FIRE_VOICE = 26,
    TURRET_VOICE = 34,
    TRACK_VOICE = 28,
    SMALL_FLASH = 4,
    LARGE_FLASH = 20,
    HIT_SOUND = 18,
    FATAL_SOUND = 45,
    DESTRUCTION_SOUND = 9,
    WRECK_TYPE = 23,
    WRECK_SCALE = 256,
    WRECK_PARAMETER = 768,
    WRECK_FLAGS = 64,
    WRECK_SECONDARY = 4,
    RETIREMENT_UPDATES = 4,
    FIRE_CHOICES = 8
};

/* Profile 0 + byte offset 5, directly reached by the M1 station-0 launcher. */
static const uint8_t damage_records[FIST_UNIT_GROUND_VEHICLE_COUNT][2] = {
    {90, 25}, {120, 90}, {90, 60}, {120, 90}};
static const uint8_t aspect_factors[FIST_UNIT_GROUND_VEHICLE_COUNT][ASPECTS] = {
    {100, 150, 220, 245, 248, 251, 253, 255, 255, 253, 251, 248, 245, 220, 150, 100},
    {200, 210, 220, 230, 240, 245, 250, 255, 255, 250, 245, 240, 230, 220, 210, 200},
    {180, 190, 200, 210, 220, 230, 240, 255, 255, 240, 230, 220, 210, 200, 190, 180},
    {220, 225, 230, 235, 240, 245, 250, 255, 255, 250, 245, 240, 235, 230, 225, 220}};
static const uint8_t fire_flags[FIRE_CHOICES] = {2, 4, 2, 4, 2, 4, 2, 6};
static const uint16_t wreck_models[FIST_UNIT_GROUND_VEHICLE_COUNT] = {10, 22, 34, 46};

static int valid_request(const fist_vehicle_state *vehicle,
                         const fist_damage_environment *environment,
                         fist_vehicle_damage_request request) {
    if (vehicle == NULL || !fist_damage_source_is_valid(environment, request) ||
        vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type) ||
        (vehicle->object_flags & DELETED_FLAG) != 0 ||
        request.hit.registry_index != vehicle->registry_index ||
        request.hit.value != vehicle->generation) {
        return 0;
    }
    const fist_pool_allocation target = {vehicle->type, request.hit.slot,
                                         request.hit.registry_index, request.hit.value};
    return fist_object_pool_is_current(environment->pool, target);
}

static uint8_t damage_roll(fist_random *random, uint16_t type, fist_vehicle_damage_request request,
                           const fist_combat_state *state) {
    const uint16_t rolled = fist_damage_base_roll(random, damage_records[type]);
    const uint16_t angled =
        fist_damage_scale_word(rolled, aspect_factors[type][request.hit.aspect]);
    return fist_damage_scale_source(angled, state, request.projectile);
}

static uint16_t fire_reaction(fist_vehicle_state *vehicle, fist_random *random,
                              fist_vehicle_damage_result *result) {
    uint16_t value = fist_damage_next_random(random);
    if ((value & FIRE_ROLL_MASK) == 0) {
        if ((vehicle->drive.motion_flags & FIRE_FLAGS) == 0) {
            result->voice_requests[result->voice_count++] = FIRE_VOICE;
        }
        /* Actual a02d returns AX with AL replaced by the selected flag and AH zero.
         * Its caller uses this returned AH for the following turret admission. */
        value = fire_flags[value % FIRE_CHOICES];
        vehicle->drive.motion_flags |= (uint8_t)value;
        vehicle->drive.throttle = 0;
        const int speed = vehicle->drive.speed;
        vehicle->drive.speed = (int16_t)(speed >= 0 ? speed / 2 : -((-speed + 1) / 2));
    }
    return value;
}

static void turret_reaction(fist_vehicle_state *vehicle, fist_random *random,
                            fist_vehicle_damage_result *result) {
    if ((vehicle->drive.motion_flags & TURRET_FLAG) == 0 &&
        (fist_damage_next_random(random) & TURRET_ROLL_MASK) == 0) {
        result->voice_requests[result->voice_count++] = TURRET_VOICE;
        vehicle->drive.motion_flags |= TURRET_FLAG;
    }
}

static void track_reaction(fist_vehicle_state *vehicle, fist_random *random,
                           fist_vehicle_damage_result *result) {
    if ((vehicle->drive.motion_flags & TRACK_FLAG) == 0 &&
        (fist_damage_next_random(random) & FIRE_ROLL_MASK) == 0) {
        result->voice_requests[result->voice_count++] = TRACK_VOICE;
        vehicle->drive.motion_flags |= TRACK_FLAG;
        vehicle->operating_flags =
            (uint8_t)((vehicle->operating_flags & (uint8_t)~OPERATION_CLEAR_FLAG) |
                      OPERATION_STOP_FLAG);
    }
}

static void reactions(fist_vehicle_state *vehicle, fist_random *random,
                      fist_vehicle_damage_result *result) {
    const uint8_t *factors = aspect_factors[vehicle->type];
    uint16_t value = fist_damage_next_random(random);
    /* BX was popped back to the aspect table before these admissions. */
    if ((uint8_t)value <= factors[2]) {
        value = fire_reaction(vehicle, random, result);
    }
    if ((value >> FIXED_SCALE_SHIFT) <= factors[3]) {
        turret_reaction(vehicle, random, result);
    }
    value = fist_damage_next_random(random);
    if ((uint8_t)value <= factors[4]) {
        track_reaction(vehicle, random, result);
    }
}

static void selected_feedback(fist_vehicle_state *vehicle, const fist_combat_state *state,
                              fist_vehicle_damage_request request,
                              fist_vehicle_damage_result *result, uint8_t *flash) {
    if (request.hit.slot == state->selected_slot &&
        (vehicle->secondary_flags & SECONDARY_FEEDBACK_SUPPRESS) == 0) {
        *flash = result->applied_damage > 1 ? LARGE_FLASH : SMALL_FLASH;
        vehicle->damage_alarm_countdown = *flash;
        result->sound_request = HIT_SOUND;
    }
}

static int destroy_vehicle(fist_vehicle_state *vehicle, const fist_damage_environment *environment,
                           fist_vehicle_damage_request request,
                           fist_vehicle_damage_result *result) {
    fist_combat_state *state = environment->state;
    const size_t side = (vehicle->object_flags & SIDE_FLAG) != 0;
    state->destroyed_by_side[side] = (uint16_t)(state->destroyed_by_side[side] + 1);
    if (side == 0 && (request.projectile->flags & SIDE_FLAG) == 0) {
        state->clear_side_destroyed_by_clear_source =
            (uint16_t)(state->clear_side_destroyed_by_clear_source + 1);
    }
    const fist_object_pose pose = {vehicle->map_x, vehicle->map_y, vehicle->altitude,
                                   vehicle->drive.heading};
    for (size_t index = 0; index < FIST_DAMAGE_EXPLOSIONS; ++index) {
        const uint8_t template_id =
            index == 0 ? FIST_EXPLOSION_VEHICLE_SHOCK : FIST_EXPLOSION_VEHICLE_FIRE;
        fist_explosion explosion = {0};
        const int status = fist_explosion_create(environment->pool, &pose, template_id, &explosion);
        if (status < 0) {
            return -1;
        }
        if (status == FIST_POOL_OK) {
            result->explosions[result->explosion_count++] = explosion;
        }
    }
    fist_pool_allocation allocation = {0};
    const int status = fist_object_pool_allocate(environment->pool,
                                                 (fist_pool_request){WRECK_TYPE, 0}, &allocation);
    if (status < 0) {
        return -1;
    }
    uint16_t replacement = FIST_POOL_NO_SLOT;
    if (status == FIST_POOL_OK) {
        replacement = allocation.slot;
        result->has_wreck = true;
        result->wreck = (fist_vehicle_wreck){.allocation = allocation,
                                             .pose = pose,
                                             .model_code = wreck_models[vehicle->type],
                                             .original_type = vehicle->type,
                                             .projection_scale = WRECK_SCALE,
                                             .parameter = WRECK_PARAMETER,
                                             .platoon = vehicle->platoon,
                                             .member = vehicle->member,
                                             .flags = WRECK_FLAGS,
                                             .secondary_flags = WRECK_SECONDARY};
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        if (state->roster[index] == request.hit.slot) {
            state->roster[index] = replacement;
        }
    }
    const fist_pool_allocation target = {vehicle->type, request.hit.slot,
                                         request.hit.registry_index, request.hit.value};
    if (fist_object_pool_retype(environment->pool, target, FIST_UNIT_EXTENDED_RESERVED_TYPE,
                                &result->retiring) != 0) {
        return -1;
    }
    vehicle->type = FIST_UNIT_EXTENDED_RESERVED_TYPE;
    vehicle->object_flags |= DELETED_FLAG;
    vehicle->secondary_flags = RETIREMENT_UPDATES;
    for (size_t platoon = 0; platoon < FIST_DAMAGE_COUNTED_PLATOONS; ++platoon) {
        uint16_t count = 0;
        for (size_t member = 0; member < FIST_UNIT_MEMBERS_PER_PLATOON; ++member) {
            const uint16_t slot = state->roster[(platoon * FIST_UNIT_MEMBERS_PER_PLATOON) + member];
            if (slot != FIST_POOL_NO_SLOT && environment->pool->slots[slot].type != WRECK_TYPE) {
                ++count;
            }
        }
        state->platoon_sizes[platoon] = count;
    }
    result->destroyed = true;
    result->selected_destroyed = state->selected_slot == request.hit.slot;
    result->sound_request = FATAL_SOUND;
    result->destruction_sound_request = DESTRUCTION_SOUND;
    return 0;
}

int fist_vehicle_damage_m1(fist_vehicle_state *vehicle, const fist_damage_environment *environment,
                           fist_vehicle_damage_request request, fist_vehicle_damage_result *out) {
    if (out == NULL || !valid_request(vehicle, environment, request)) {
        return -1;
    }
    fist_vehicle_state actor = *vehicle;
    fist_object_pool pool = *environment->pool;
    fist_combat_state state = *environment->state;
    fist_random random = *environment->random;
    const fist_damage_environment updated = {&pool, &random, &state};
    fist_vehicle_damage_result result = {.sound_request = FIST_DAMAGE_NO_REQUEST,
                                         .destruction_sound_request = FIST_DAMAGE_NO_REQUEST,
                                         .refresh_damage_display = true};
    actor.control_flags |= HIT_CONTROL_FLAG;
    result.applied_damage = damage_roll(&random, actor.type, request, &state);
    const unsigned total = (unsigned)actor.damage + result.applied_damage;
    actor.damage = (uint8_t)total;
    if (total >= BYTE_RANGE || actor.damage >= DAMAGE_LIMIT) {
        if (destroy_vehicle(&actor, &updated, request, &result) != 0) {
            return -1;
        }
    } else {
        reactions(&actor, &random, &result);
        selected_feedback(&actor, &state, request, &result, &state.damage_flash);
    }
    *vehicle = actor;
    *environment->pool = pool;
    *environment->random = random;
    *environment->state = state;
    *out = result;
    return 0;
}

int fist_vehicle_retirement_advance(fist_object_pool *pool, fist_pool_allocation allocation,
                                    fist_vehicle_state *vehicle, bool *out) {
    if (vehicle == NULL || out == NULL || vehicle->type != FIST_UNIT_EXTENDED_RESERVED_TYPE ||
        allocation.type != vehicle->type || allocation.registry_index != vehicle->registry_index ||
        allocation.value != vehicle->generation || (vehicle->object_flags & DELETED_FLAG) == 0 ||
        !fist_object_pool_is_current(pool, allocation)) {
        return -1;
    }
    const uint8_t count = (uint8_t)(vehicle->secondary_flags - 1);
    if (count == 0) {
        fist_pool_allocation removed = {0};
        if (fist_object_pool_release(pool, allocation.registry_index, &removed) != FIST_POOL_OK) {
            return -1;
        }
    }
    vehicle->secondary_flags = count;
    *out = count == 0;
    return 0;
}
