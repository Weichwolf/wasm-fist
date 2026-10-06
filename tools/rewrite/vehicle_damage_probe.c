#include "assets/bytes.h"
#include "assets/units.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/collision.h"
#include "sim/object_pool.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"
#include "vehicle_probe_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    FILE_HEADER = 4,
    CASE_HEADER = 40,
    CURSOR = 8,
    SOURCE_FLAGS = 9,
    ASPECT = 10,
    SELECTED = 11,
    SCALE_CLEAR = 12,
    SCALE_SET = 14,
    IMPORTS = 16,
    TARGET_ORDINAL = 18,
    STEPS = 20,
    RETIRE_TICKS = 22,
    FINISH_SHELL = 23,
    FLASH = 24,
    RESERVED = 25,
    COUNTERS = 26,
    PLATOON_SIZES = 32,
    BINDING_BYTES = 6,
    BINDING_INDEX = 2,
    BINDING_VALUE = 4,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    OBJECT_FLAGS = 22,
    SECONDARY_FLAGS = 23,
    OPERATING_FLAGS = 26,
    CONTROL_FLAGS = 64,
    RANDOM_PHASE_A = 109,
    RANDOM_PHASE_B = 66,
    AMMO = 173,
    T80_AMMO = 172,
    RELOAD = 168,
    STOCK = 187,
    CYCLE = 181,
    SOURCE_TYPE = 8,
    PARAMETER = 5,
    MARKER = 123,
    COLLIDABLE = 64,
    DAMAGE_SCALE = 256
};

typedef struct {
    fist_object_pool pool;
    fist_pool_allocation allocations[FIST_UNIT_REGISTRY_COUNT];
    fist_vehicle_state vehicle;
    fist_projectile projectile;
    fist_random random;
    fist_combat_state combat;
    fist_vehicle_damage_request request;
    uint16_t steps;
    uint8_t retire_ticks;
    uint8_t finish_shell;
} damage_case;

static int same_bytes(const void *left, size_t size, const void *right) {
    const unsigned char *first = left;
    const unsigned char *second = right;
    for (size_t index = 0; index < size; ++index) {
        if (first[index] != second[index]) {
            return 0;
        }
    }
    return 1;
}

static int restore(const uint8_t *raw, fist_pool_allocation allocation, fist_vehicle_state *out) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .registry_index = allocation.registry_index,
                                             .generation = allocation.value,
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    fist_random random = {0};
    if (fist_vehicle_initialize(&definition, &random, 0, out) != 0) {
        return -1;
    }
    out->object_flags = raw[OBJECT_FLAGS];
    out->secondary_flags = raw[SECONDARY_FLAGS];
    out->operating_flags = raw[OPERATING_FLAGS];
    out->control_flags = fist_read_u16le(raw + CONTROL_FLAGS);
    out->random_phases[0] = raw[RANDOM_PHASE_A];
    out->random_phases[1] = raw[RANDOM_PHASE_B];
    out->reload_countdown = raw[RELOAD];
    const size_t ammo = out->type == 2 ? T80_AMMO : AMMO;
    for (size_t index = 0; index < FIST_VEHICLE_WEAPON_SLOTS; ++index) {
        out->weapons.rounds[index] = fist_read_u16le(raw + ammo + (index * sizeof(uint16_t)));
    }
    static const size_t parameters[] = {181, 250, 180, 249};
    out->weapons.class_parameter =
        out->type == 2 ? fist_read_u16le(raw + parameters[out->type]) : raw[parameters[out->type]];
    if (out->type == 1 || out->type == 3) {
        out->weapons.ready_stock = raw[STOCK];
        out->weapons.cycle[0] = raw[CYCLE];
        out->weapons.cycle[1] = raw[CYCLE + 1];
    }
    for (size_t index = 0; index < out->component_size; ++index) {
        out->components[index] = raw[fist_probe_component_offset(out->type) + index];
    }
    return 0;
}

static int prepare(damage_case *value, const uint8_t *input, size_t size) {
    const size_t count = fist_read_u16le(input + IMPORTS);
    const size_t target = fist_read_u16le(input + TARGET_ORDINAL);
    if (count == 0 || count > FIST_UNIT_REGISTRY_COUNT || target >= count ||
        size != CASE_HEADER + FIST_UNIT_EXTENDED_SIZE + (count * BINDING_BYTES) +
                    (FIST_UNIT_ROSTER_COUNT * sizeof(uint16_t)) ||
        input[CURSOR] >= FIST_RANDOM_STREAMS || input[SELECTED] > 1 || input[FINISH_SHELL] > 1 ||
        input[RESERVED] != 0 || fist_read_u16le(input + STEPS) == 0) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    const uint8_t *raw = input + CASE_HEADER;
    const uint8_t *bindings = raw + FIST_UNIT_EXTENDED_SIZE;
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = bindings + (index * BINDING_BYTES);
        const fist_pool_import request = {fist_read_u16le(record),
                                          fist_read_u16le(record + BINDING_INDEX),
                                          fist_read_u16le(record + BINDING_VALUE)};
        if (fist_object_pool_import(&value->pool, request, &value->allocations[index]) != 0) {
            return -1;
        }
    }
    if (value->allocations[0].type != SOURCE_TYPE ||
        restore(raw, value->allocations[target], &value->vehicle) != 0 ||
        value->vehicle.type != value->allocations[target].type) {
        return -1;
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        value->random.words[index] = fist_read_u16le(input + (index * sizeof(uint16_t)));
    }
    value->random.next_stream = input[CURSOR];
    value->combat.source_scale[0] = fist_read_u16le(input + SCALE_CLEAR);
    value->combat.source_scale[1] = fist_read_u16le(input + SCALE_SET);
    value->combat.selected_slot =
        input[SELECTED] != 0 ? value->allocations[target].slot : FIST_POOL_NO_SLOT;
    value->combat.damage_flash = input[FLASH];
    for (size_t index = 0; index < FIST_DAMAGE_SIDES; ++index) {
        value->combat.destroyed_by_side[index] =
            fist_read_u16le(input + COUNTERS + (index * sizeof(uint16_t)));
    }
    value->combat.clear_side_destroyed_by_clear_source =
        fist_read_u16le(input + COUNTERS + (FIST_DAMAGE_SIDES * sizeof(uint16_t)));
    for (size_t index = 0; index < FIST_DAMAGE_COUNTED_PLATOONS; ++index) {
        value->combat.platoon_sizes[index] =
            fist_read_u16le(input + PLATOON_SIZES + (index * sizeof(uint16_t)));
    }
    const uint8_t *roster = bindings + (count * BINDING_BYTES);
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        const uint16_t ordinal = fist_read_u16le(roster + (index * sizeof(uint16_t)));
        if (ordinal != FIST_POOL_NO_SLOT && ordinal >= count) {
            return -1;
        }
        value->combat.roster[index] =
            ordinal == FIST_POOL_NO_SLOT ? FIST_POOL_NO_SLOT : value->allocations[ordinal].slot;
    }
    value->projectile = (fist_projectile){
        .allocation = value->allocations[0],
        .pose = {value->vehicle.map_x, value->vehicle.map_y, value->vehicle.altitude, 0},
        .target_slot = FIST_POOL_NO_SLOT,
        .origin_slot = FIST_POOL_NO_SLOT,
        .flags = input[SOURCE_FLAGS],
        .launch_parameter = PARAMETER,
        .phase = FIST_PROJECTILE_UNIT_IMPACT};
    value->request = (fist_vehicle_damage_request){
        &value->projectile,
        {value->allocations[target].slot, value->allocations[target].registry_index,
         value->allocations[target].value, input[ASPECT]}};
    value->steps = fist_read_u16le(input + STEPS);
    value->retire_ticks = input[RETIRE_TICKS];
    value->finish_shell = input[FINISH_SHELL];
    return 0;
}

static void write_combat(const damage_case *value) {
    const fist_combat_state *state = &value->combat;
    printf("combat %u %u %u %u %u\nroster", (unsigned)state->selected_slot,
           (unsigned)state->damage_flash, (unsigned)state->destroyed_by_side[0],
           (unsigned)state->destroyed_by_side[1],
           (unsigned)state->clear_side_destroyed_by_clear_source);
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        printf(" %u", (unsigned)state->roster[index]);
    }
    printf("\nplatoons");
    for (size_t index = 0; index < FIST_DAMAGE_COUNTED_PLATOONS; ++index) {
        printf(" %u", (unsigned)state->platoon_sizes[index]);
    }
    printf("\nrandom %u", (unsigned)value->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)value->random.words[index]);
    }
    printf("\n");
}

static void write_explosion(const fist_explosion *effect) {
    printf("effect %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)effect->allocation.slot, (unsigned)effect->allocation.registry_index,
           (unsigned)effect->allocation.value, (long)effect->pose.x, (long)effect->pose.y,
           (long)effect->pose.altitude, (unsigned)effect->model_code, (unsigned)effect->extent,
           (unsigned)effect->projection_scale, (unsigned)effect->callback_selector,
           (unsigned)effect->height_offset, (unsigned)effect->frame, (unsigned)effect->last_frame,
           (unsigned)effect->period, (unsigned)effect->countdown, (unsigned)effect->flags);
}

static void write_result(const fist_vehicle_damage_result *result) {
    printf("result %u %u %u %u %u %u %u %u %u\nvoices", (unsigned)result->applied_damage,
           (unsigned)result->destroyed, (unsigned)result->has_wreck,
           (unsigned)result->selected_destroyed, (unsigned)result->refresh_damage_display,
           (unsigned)result->sound_request, (unsigned)result->destruction_sound_request,
           (unsigned)result->explosion_count, (unsigned)result->voice_count);
    for (size_t index = 0; index < result->voice_count; ++index) {
        printf(" %u", (unsigned)result->voice_requests[index]);
    }
    printf("\n");
    for (size_t index = 0; index < result->explosion_count; ++index) {
        write_explosion(&result->explosions[index]);
    }
    if (result->has_wreck) {
        const fist_vehicle_wreck *wreck = &result->wreck;
        printf("wreck %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u %u\n",
               (unsigned)wreck->allocation.slot, (unsigned)wreck->allocation.registry_index,
               (unsigned)wreck->allocation.value, (long)wreck->pose.x, (long)wreck->pose.y,
               (long)wreck->pose.altitude, (unsigned)wreck->pose.heading,
               (unsigned)wreck->model_code, (unsigned)wreck->original_type,
               (unsigned)wreck->projection_scale, (unsigned)wreck->parameter,
               (unsigned)wreck->platoon, (unsigned)wreck->member, (unsigned)wreck->flags,
               (unsigned)wreck->secondary_flags);
    }
}

static int retirement_rejected(damage_case *value, fist_pool_allocation allocation) {
    const damage_case before = *value;
    bool removed = true;
    return fist_vehicle_retirement_advance(&value->pool, allocation, &value->vehicle, &removed) ==
               -1 &&
           removed && same_bytes(value, sizeof(*value), &before);
}

static int invalid_retirement(damage_case *value, fist_pool_allocation allocation) {
    const damage_case before = *value;
    bool removed = true;
    int valid =
        fist_vehicle_retirement_advance(&value->pool, allocation, NULL, &removed) == -1 &&
        fist_vehicle_retirement_advance(&value->pool, allocation, &value->vehicle, NULL) == -1 &&
        removed && same_bytes(value, sizeof(*value), &before);
    value->vehicle.registry_index = (uint16_t)(value->vehicle.registry_index + 1);
    valid = valid && retirement_rejected(value, allocation);
    *value = before;
    value->vehicle.generation = (uint16_t)(value->vehicle.generation + 1);
    valid = valid && retirement_rejected(value, allocation);
    *value = before;
    allocation.value = (uint16_t)(allocation.value + 1);
    valid = valid && retirement_rejected(value, allocation);
    return valid ? 0 : -1;
}

static int advance_retirement(damage_case *value, fist_pool_allocation allocation, size_t ticks) {
    if (invalid_retirement(value, allocation) != 0) {
        return -1;
    }
    for (size_t tick = 0; tick < ticks; ++tick) {
        bool removed = false;
        if (fist_vehicle_retirement_advance(&value->pool, allocation, &value->vehicle, &removed) !=
            0) {
            return -1;
        }
        printf("retire %u %u\n", (unsigned)value->vehicle.secondary_flags, (unsigned)removed);
        fist_probe_write_object_pool(&value->pool);
        if (removed) {
            return retirement_rejected(value, allocation) ? 0 : -1;
        }
    }
    return 0;
}

static int observe(damage_case *value) {
    const fist_damage_environment environment = {&value->pool, &value->random, &value->combat};
    fist_vehicle_damage_result result = {0};
    for (size_t index = 0; index < value->steps; ++index) {
        const fist_projectile saved = value->projectile;
        if (fist_vehicle_damage_m1(&value->vehicle, &environment, value->request, &result) != 0 ||
            !same_bytes(&saved, sizeof(saved), &value->projectile)) {
            return -1;
        }
        write_result(&result);
        fist_probe_write_vehicle_state(&value->vehicle);
        printf("damage %u %u %u %u\n", (unsigned)value->vehicle.damage,
               (unsigned)value->vehicle.damage_alarm_countdown, (unsigned)value->vehicle.platoon,
               (unsigned)value->vehicle.member);
        write_combat(value);
        fist_probe_write_object_pool(&value->pool);
        if (result.destroyed) {
            break;
        }
    }
    if (value->finish_shell != 0) {
        fist_projectile_impact impact = {0};
        if (fist_projectile_finish_impact(&value->pool, &value->projectile, &impact) != 0) {
            return -1;
        }
        printf("impact %u %u %u %u\n", (unsigned)impact.has_explosion, (unsigned)impact.notice,
               (unsigned)impact.sound_request, (unsigned)impact.hit_voice);
        if (impact.has_explosion) {
            write_explosion(&impact.explosion);
        }
        fist_probe_write_object_pool(&value->pool);
    }
    if (result.destroyed && advance_retirement(value, result.retiring, value->retire_ticks) != 0) {
        return -1;
    }
    return 0;
}

static int rejected(damage_case *value, const fist_damage_environment *environment,
                    fist_vehicle_damage_request request) {
    const damage_case before = *value;
    const fist_vehicle_damage_result marker = {.applied_damage = MARKER};
    fist_vehicle_damage_result output = marker;
    return fist_vehicle_damage_m1(&value->vehicle, environment, request, &output) == -1 &&
           same_bytes(value, sizeof(*value), &before) &&
           same_bytes(&output, sizeof(output), &marker);
}

static int invalid_owners(damage_case *value, const fist_damage_environment *environment) {
    const damage_case before = *value;
    fist_vehicle_damage_result output = {.applied_damage = MARKER};
    const fist_vehicle_damage_result marker = output;
    int valid = fist_vehicle_damage_m1(NULL, environment, value->request, &output) == -1 &&
                fist_vehicle_damage_m1(&value->vehicle, environment, value->request, NULL) == -1 &&
                same_bytes(value, sizeof(*value), &before) &&
                same_bytes(&output, sizeof(output), &marker);
    value->random.next_stream = FIST_RANDOM_STREAMS;
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    value->combat.selected_slot = FIST_UNIT_REGISTRY_COUNT;
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    value->combat.roster[0] = FIST_UNIT_REGISTRY_COUNT;
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    value->vehicle.component_size = 0;
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    value->projectile.allocation.value = (uint16_t)(value->projectile.allocation.value + 1);
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    value->projectile.launch_parameter = 0;
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    ++value->pool.short_count;
    valid = valid && rejected(value, environment, value->request);
    *value = before;
    return valid ? 0 : -1;
}

static int invalid_inputs(damage_case *value) {
    const damage_case before = *value;
    const fist_damage_environment environment = {&value->pool, &value->random, &value->combat};
    int valid = invalid_owners(value, &environment) == 0 && rejected(value, NULL, value->request);
    fist_damage_environment broken = environment;
    broken.random = NULL;
    valid = valid && rejected(value, &broken, value->request);
    broken = environment;
    broken.state = NULL;
    valid = valid && rejected(value, &broken, value->request);
    fist_vehicle_damage_request bad = value->request;
    bad.hit.value = (uint16_t)(bad.hit.value + 1);
    valid = valid && rejected(value, &environment, bad);
    bad = value->request;
    bad.hit.aspect = UINT8_MAX;
    valid = valid && rejected(value, &environment, bad);
    bad.projectile = NULL;
    valid = valid && rejected(value, &environment, bad);
    value->projectile.phase = FIST_PROJECTILE_FLYING;
    valid = valid && rejected(value, &environment, value->request);
    value->projectile.phase = FIST_PROJECTILE_UNIT_IMPACT;
    value->projectile.collision_profile = MARKER;
    valid = valid && rejected(value, &environment, value->request);
    *value = before;
    fist_pool_allocation output = {0};
    valid = valid &&
            fist_object_pool_retype(&value->pool, value->projectile.allocation, 0, &output) == -1 &&
            same_bytes(value, sizeof(*value), &before);
    return valid ? 0 : -1;
}

static int run(const uint8_t *input, size_t size) {
    if (size < FILE_HEADER) {
        return -1;
    }
    const size_t count = fist_read_u32le(input);
    if (count == 0 || count > (size - FILE_HEADER) / CASE_HEADER) {
        return -1;
    }
    size_t offset = FILE_HEADER;
    damage_case *value = calloc(1, sizeof(*value));
    if (value == NULL) {
        return -1;
    }
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        if (size - offset < CASE_HEADER) {
            status = -1;
            break;
        }
        const size_t extent = CASE_HEADER + FIST_UNIT_EXTENDED_SIZE +
                              (fist_read_u16le(input + offset + IMPORTS) * BINDING_BYTES) +
                              (FIST_UNIT_ROSTER_COUNT * sizeof(uint16_t));
        *value = (damage_case){0};
        if (extent > size - offset || prepare(value, input + offset, extent) != 0 ||
            invalid_inputs(value) != 0 || observe(value) != 0) {
            status = -1;
            break;
        }
        offset += extent;
    }
    free(value);
    return status == 0 && offset == size ? 0 : -1;
}

static int pipeline_vehicle(fist_object_pool *pool, fist_pool_import request,
                            const fist_object_pose *pose, fist_vehicle_state *out) {
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE] = {0};
    raw[0] = (uint8_t)request.type;
    raw[OBJECT_FLAGS] = COLLIDABLE;
    fist_pool_allocation allocation = {0};
    if (fist_object_pool_import(pool, request, &allocation) != 0) {
        return -1;
    }
    const fist_unit_definition definition = {.type = request.type,
                                             .registry_index = request.registry_index,
                                             .generation = request.value,
                                             .map_x = pose->x,
                                             .map_y = pose->y,
                                             .altitude = pose->altitude,
                                             .snapshot = {raw, sizeof(raw)}};
    fist_random random = {0};
    return fist_vehicle_initialize(&definition, &random, 0, out);
}

static int pipeline_cleanup(damage_case *value, fist_vehicle_damage_result *result,
                            fist_projectile_impact *impact, fist_muzzle_smoke *muzzle) {
    enum { RETIREMENT_TICKS = 4, EFFECT_TICKS = 132, DELETED = 1 };
    if (advance_retirement(value, result->retiring, RETIREMENT_TICKS) != 0) {
        return -1;
    }
    for (size_t tick = 0; tick < EFFECT_TICKS; ++tick) {
        for (size_t index = 0; index < result->explosion_count; ++index) {
            if ((result->explosions[index].flags & DELETED) == 0 &&
                fist_explosion_advance(&value->pool, &result->explosions[index]) != 0) {
                return -1;
            }
        }
        if (((impact->explosion.flags & DELETED) == 0 &&
             fist_explosion_advance(&value->pool, &impact->explosion) != 0) ||
            ((muzzle->flags & DELETED) == 0 &&
             fist_muzzle_smoke_advance(&value->pool, muzzle) != 0)) {
            return -1;
        }
    }
    printf("pipeline_effects %u %u %u %u\n", (unsigned)result->explosions[0].flags,
           (unsigned)result->explosions[1].flags, (unsigned)impact->explosion.flags,
           (unsigned)muzzle->flags);
    fist_probe_write_object_pool(&value->pool);
    return value->pool.short_count == 1 && value->pool.extended_count == 1 ? 0 : -1;
}

static int pipeline(void) {
    enum { FLIGHT_TICKS = 3 };
    damage_case value = {0};
    fist_object_pool_reset(&value.pool);
    const fist_object_pose origin_pose = {0, 0, 65536, 0};
    const fist_object_pose target_pose = {0, 2556, 66560, 0};
    fist_vehicle_state origin = {0};
    if (pipeline_vehicle(&value.pool, (fist_pool_import){0, 3, 1}, &origin_pose, &origin) != 0 ||
        pipeline_vehicle(&value.pool, (fist_pool_import){2, 4, 1}, &target_pose, &value.vehicle) !=
            0) {
        return -1;
    }
    const uint16_t origin_slot = value.pool.registry[3].slot;
    const uint16_t target_slot = value.pool.registry[4].slot;
    fist_launch_result launch = {0};
    if (fist_m1_launch_untargeted(&value.pool, &origin, (fist_launch_request){origin_slot, false},
                                  &launch) != 0 ||
        launch.outcome != FIST_LAUNCH_FIRED || !launch.has_muzzle) {
        return -1;
    }
    value.projectile = launch.projectile;
    printf("pipeline_launch %u %u %u\n", (unsigned)launch.outcome, (unsigned)launch.has_muzzle,
           (unsigned)launch.sound_request);
    fist_probe_write_vehicle_state(&origin);
    fist_collision_body bodies[FIST_UNIT_REGISTRY_COUNT] = {0};
    bodies[origin_slot] = (fist_collision_body){&origin_pose, origin.projection_scale,
                                                origin.object_flags, origin.drive.motion_flags};
    bodies[target_slot] =
        (fist_collision_body){&target_pose, value.vehicle.projection_scale,
                              value.vehicle.object_flags, value.vehicle.drive.motion_flags};
    bodies[value.projectile.allocation.slot] =
        (fist_collision_body){&value.projectile.pose, 0, value.projectile.flags, 0};
    bodies[launch.muzzle.allocation.slot] = (fist_collision_body){
        &launch.muzzle.pose, launch.muzzle.projection_scale, launch.muzzle.flags, 0};
    const fist_projectile_environment environment = {
        &value.pool, {&value.pool, bodies, FIST_UNIT_REGISTRY_COUNT}, NULL, &value.random};
    fist_projectile_step step = {0};
    for (size_t tick = 0; tick < FLIGHT_TICKS; ++tick) {
        if (fist_projectile_advance(&value.projectile, &environment, &step) != 0) {
            return -1;
        }
        printf("pipeline_flight %u %u %u %ld %ld %ld %u %u\n", (unsigned)step.phase,
               (unsigned)step.hit.slot, (unsigned)step.hit.aspect, (long)value.projectile.pose.x,
               (long)value.projectile.pose.y, (long)value.projectile.pose.altitude,
               (unsigned)value.projectile.age, (unsigned)value.projectile.collision_grace);
    }
    if (step.phase != FIST_PROJECTILE_UNIT_IMPACT || step.hit.slot != target_slot) {
        return -1;
    }
    value.request = (fist_vehicle_damage_request){&value.projectile, step.hit};
    value.combat.source_scale[0] = value.combat.source_scale[1] = DAMAGE_SCALE;
    value.combat.selected_slot = origin_slot;
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        value.combat.roster[index] = FIST_POOL_NO_SLOT;
    }
    value.combat.roster[0] = target_slot;
    value.combat.roster[1] = origin_slot;
    const fist_damage_environment damage = {&value.pool, &value.random, &value.combat};
    const fist_projectile saved = value.projectile;
    fist_vehicle_damage_result result = {0};
    if (fist_vehicle_damage_m1(&value.vehicle, &damage, value.request, &result) != 0 ||
        !result.destroyed || !result.has_wreck ||
        result.explosion_count != FIST_DAMAGE_EXPLOSIONS ||
        !same_bytes(&saved, sizeof(saved), &value.projectile)) {
        return -1;
    }
    write_result(&result);
    fist_probe_write_vehicle_state(&value.vehicle);
    printf("damage %u %u %u %u\n", (unsigned)value.vehicle.damage,
           (unsigned)value.vehicle.damage_alarm_countdown, (unsigned)value.vehicle.platoon,
           (unsigned)value.vehicle.member);
    write_combat(&value);
    fist_probe_write_object_pool(&value.pool);
    fist_projectile_impact impact = {0};
    if (fist_projectile_finish_impact(&value.pool, &value.projectile, &impact) != 0 ||
        !impact.has_explosion) {
        return -1;
    }
    printf("impact %u %u %u %u\n", (unsigned)impact.has_explosion, (unsigned)impact.notice,
           (unsigned)impact.sound_request, (unsigned)impact.hit_voice);
    write_explosion(&impact.explosion);
    fist_probe_write_object_pool(&value.pool);
    return pipeline_cleanup(&value, &result, &impact, &launch.muzzle);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    if (argv[1][0] == '-' && argv[1][1] == 'p' && argv[1][2] == '\0') {
        return pipeline() == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *input = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    const int status = input != NULL && closed == 0 ? run(input, size) : -1;
    free(input);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
