#include "assets/bytes.h"
#include "assets/units.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/collision.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    FILE_HEADER = 4,
    CASE_HEADER = 40,
    BINDING_BYTES = 6,
    CURSOR = 8,
    SOURCE_FLAGS = 9,
    ASPECT = 10,
    FINISH = 11,
    SCALES = 12,
    IMPORTS = 16,
    TARGET = 18,
    STEPS = 20,
    EFFECT_TICKS = 22,
    REACH = 24,
    RESERVED = 25,
    CENSUS = 26,
    FLASH = 36,
    RESERVED_END = 37,
    SELECTED = 38,
    FLAGS_OFFSET = 22,
    SECONDARY_OFFSET = 23,
    HEIGHT_OFFSET = 24,
    MODE_OFFSET = 25,
    EXTENT_OFFSET = 18,
    SCALE_OFFSET = 20,
    SOURCE_TYPE = 8,
    PARAMETER = 5,
    WRECK_TYPE = 23,
    DELETED_FLAG = 1,
    FLIGHT_TICKS = 3,
    FLIGHT_Y = 852,
    CLEARANCE = 1000000,
    MARKER = 123,
    PAIR_FIRST = 5,
    PAIR_SECOND = 6,
    TYPE26_TYPE = 26,
    LAST_TYPE = 27
};

typedef struct {
    fist_object_pool pool;
    fist_pool_allocation allocations[FIST_UNIT_REGISTRY_COUNT];
    fist_other_actor actor;
    fist_projectile source;
    fist_random random;
    fist_combat_state combat;
    fist_vehicle_damage_request request;
    uint16_t steps;
    uint16_t effect_ticks;
    uint8_t finish;
    uint8_t reach;
} other_case;

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

static int invalid_restore(const fist_unit_definition *definition,
                           fist_pool_allocation allocation) {
    const fist_other_actor marker = {.flags = MARKER};
    fist_other_actor output = marker;
    fist_unit_definition bad = *definition;
    bad.snapshot.data = NULL;
    int valid = fist_other_actor_restore(NULL, allocation, &output) == -1 &&
                fist_other_actor_restore(definition, allocation, NULL) == -1 &&
                fist_other_actor_restore(&bad, allocation, &output) == -1;
    bad = *definition;
    --bad.snapshot.size;
    valid = valid && fist_other_actor_restore(&bad, allocation, &output) == -1;
    bad = *definition;
    bad.generation = (uint16_t)(bad.generation + 1);
    valid = valid && fist_other_actor_restore(&bad, allocation, &output) == -1;
    return valid && same_bytes(&output, sizeof(output), &marker) ? 0 : -1;
}

static int restore_actor(other_case *value, const uint8_t *raw, fist_pool_allocation allocation) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .registry_index = allocation.registry_index,
                                             .generation = allocation.value,
                                             .map_x = fist_read_i32le(raw + 4),
                                             .map_y = fist_read_i32le(raw + 8),
                                             .altitude = fist_read_i32le(raw + 12),
                                             .heading = fist_read_u16le(raw + 16),
                                             .snapshot = {raw, FIST_UNIT_SHORT_SIZE}};
    if (allocation.type != definition.type || invalid_restore(&definition, allocation) != 0) {
        return -1;
    }
    if (allocation.type != WRECK_TYPE) {
        return fist_other_actor_restore(&definition, allocation, &value->actor);
    }
    /* Read-only fixture view: wreck damage takes no mutable payload argument. */
    value->actor = (fist_other_actor){
        .allocation = allocation,
        .pose = {definition.map_x, definition.map_y, definition.altitude, definition.heading},
        .projection_extent = fist_read_u16le(raw + EXTENT_OFFSET),
        .projection_scale = fist_read_u16le(raw + SCALE_OFFSET),
        .flags = raw[FLAGS_OFFSET],
        .secondary_flags = raw[SECONDARY_OFFSET],
        .ground_height = raw[HEIGHT_OFFSET],
        .mode = raw[MODE_OFFSET]};
    return 0;
}

static int prepare(other_case *value, const uint8_t *input, size_t size) {
    const size_t count = fist_read_u16le(input + IMPORTS);
    const size_t target = fist_read_u16le(input + TARGET);
    if (count == 0 || count > FIST_UNIT_REGISTRY_COUNT || target >= count ||
        size != CASE_HEADER + FIST_UNIT_SHORT_SIZE + (count * BINDING_BYTES) +
                    (FIST_UNIT_ROSTER_COUNT * sizeof(uint16_t)) ||
        input[CURSOR] >= FIST_RANDOM_STREAMS || input[FINISH] > 1 || input[REACH] > 1 ||
        input[RESERVED] != 0 || input[RESERVED_END] != 0 || fist_read_u16le(input + STEPS) == 0) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    const uint8_t *raw = input + CASE_HEADER;
    const uint8_t *bindings = raw + FIST_UNIT_SHORT_SIZE;
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = bindings + (index * BINDING_BYTES);
        const fist_pool_import request = {fist_read_u16le(record), fist_read_u16le(record + 2),
                                          fist_read_u16le(record + 4)};
        if (fist_object_pool_import(&value->pool, request, &value->allocations[index]) != 0) {
            return -1;
        }
    }
    if (value->allocations[0].type != SOURCE_TYPE ||
        restore_actor(value, raw, value->allocations[target]) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        value->random.words[index] = fist_read_u16le(input + (index * sizeof(uint16_t)));
    }
    value->random.next_stream = input[CURSOR];
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        value->combat.source_scale[side] =
            fist_read_u16le(input + SCALES + (side * sizeof(uint16_t)));
        value->combat.destroyed_by_side[side] =
            fist_read_u16le(input + CENSUS + (side * sizeof(uint16_t)));
        value->combat.pair_destroyed_by_side[side] =
            fist_read_u16le(input + CENSUS + ((side + 3) * sizeof(uint16_t)));
    }
    value->combat.clear_side_destroyed_by_clear_source = fist_read_u16le(input + CENSUS + 4);
    value->combat.damage_flash = input[FLASH];
    const uint16_t selected = fist_read_u16le(input + SELECTED);
    if (selected != FIST_POOL_NO_SLOT && selected >= count) {
        return -1;
    }
    value->combat.selected_slot =
        selected == FIST_POOL_NO_SLOT ? FIST_POOL_NO_SLOT : value->allocations[selected].slot;
    const uint8_t *roster = bindings + (count * BINDING_BYTES);
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        const uint16_t ordinal = fist_read_u16le(roster + (index * sizeof(uint16_t)));
        if (ordinal != FIST_POOL_NO_SLOT && ordinal >= count) {
            return -1;
        }
        value->combat.roster[index] =
            ordinal == FIST_POOL_NO_SLOT ? FIST_POOL_NO_SLOT : value->allocations[ordinal].slot;
    }
    value->source = (fist_projectile){.allocation = value->allocations[0],
                                      .pose = value->actor.pose,
                                      .target_slot = FIST_POOL_NO_SLOT,
                                      .origin_slot = FIST_POOL_NO_SLOT,
                                      .flags = input[SOURCE_FLAGS],
                                      .launch_parameter = PARAMETER,
                                      .phase = FIST_PROJECTILE_UNIT_IMPACT};
    value->request = (fist_vehicle_damage_request){
        &value->source,
        {value->allocations[target].slot, value->allocations[target].registry_index,
         value->allocations[target].value, input[ASPECT]}};
    value->steps = fist_read_u16le(input + STEPS);
    value->effect_ticks = fist_read_u16le(input + EFFECT_TICKS);
    value->finish = input[FINISH];
    value->reach = input[REACH];
    return 0;
}

static void write_actor(const fist_other_actor *actor) {
    const fist_pool_allocation allocation = actor->allocation;
    printf("actor %u %u %u %u %ld %ld %ld %u %u %u %u %u %u %u\n", (unsigned)allocation.type,
           (unsigned)allocation.slot, (unsigned)allocation.registry_index,
           (unsigned)allocation.value, (long)actor->pose.x, (long)actor->pose.y,
           (long)actor->pose.altitude, (unsigned)actor->pose.heading,
           (unsigned)actor->projection_extent, (unsigned)actor->projection_scale,
           (unsigned)actor->flags, (unsigned)actor->secondary_flags, (unsigned)actor->ground_height,
           (unsigned)actor->mode);
    if (allocation.type == PAIR_FIRST || allocation.type == PAIR_SECOND) {
        const fist_pair_actor_state *state = &actor->state.pair;
        printf("pair %u %u %u %u\n", (unsigned)state->animation_parameter,
               (unsigned)state->behavior, (unsigned)state->animation_frame,
               (unsigned)state->damage);
    } else if (allocation.type == TYPE26_TYPE) {
        const fist_type26_state *state = &actor->state.type26;
        printf("type26 %u %u %u\n", (unsigned)state->damage, (unsigned)state->limit,
               (unsigned)state->destruction_parameter);
    } else if (allocation.type == LAST_TYPE) {
        const fist_type27_state *state = &actor->state.type27;
        printf("type27 %u %u %u\n", (unsigned)state->damage, (unsigned)state->debris_parameter,
               (unsigned)state->animation_counter);
    }
}

static void write_effect(const fist_explosion *effect) {
    printf("effect %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)effect->allocation.slot, (unsigned)effect->allocation.registry_index,
           (unsigned)effect->allocation.value, (long)effect->pose.x, (long)effect->pose.y,
           (long)effect->pose.altitude, (unsigned)effect->model_code, (unsigned)effect->extent,
           (unsigned)effect->projection_scale, (unsigned)effect->callback_selector,
           (unsigned)effect->height_offset, (unsigned)effect->frame, (unsigned)effect->last_frame,
           (unsigned)effect->period, (unsigned)effect->countdown, (unsigned)effect->flags);
}

static void write_shared(const other_case *value) {
    const fist_combat_state *state = &value->combat;
    printf("combat %u %u %u %u %u %u %u\nroster", (unsigned)state->selected_slot,
           (unsigned)state->damage_flash, (unsigned)state->destroyed_by_side[0],
           (unsigned)state->destroyed_by_side[1],
           (unsigned)state->clear_side_destroyed_by_clear_source,
           (unsigned)state->pair_destroyed_by_side[0], (unsigned)state->pair_destroyed_by_side[1]);
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
    fist_probe_write_object_pool(&value->pool);
}

static int apply(other_case *value, const fist_damage_environment *environment,
                 fist_vehicle_damage_request request, fist_other_damage_result *out) {
    return value->actor.allocation.type == WRECK_TYPE
               ? fist_wreck_damage_m1(environment, request, out)
               : fist_other_damage_m1(&value->actor, environment, request, out);
}

static int rejected(other_case *value, const fist_damage_environment *environment,
                    fist_vehicle_damage_request request) {
    const other_case before = *value;
    const fist_other_damage_result marker = {.applied_damage = MARKER};
    fist_other_damage_result out = marker;
    return apply(value, environment, request, &out) == -1 &&
           same_bytes(value, sizeof(*value), &before) && same_bytes(&out, sizeof(out), &marker);
}

static int invalid_inputs(other_case *value) {
    const other_case before = *value;
    const fist_damage_environment environment = {&value->pool, &value->random, &value->combat};
    int valid = rejected(value, NULL, value->request);
    fist_other_damage_result output = {.applied_damage = MARKER};
    const fist_other_damage_result marker = output;
    valid = valid && apply(value, &environment, value->request, NULL) == -1 &&
            fist_other_damage_m1(NULL, &environment, value->request, &output) == -1 &&
            same_bytes(&output, sizeof(output), &marker) &&
            same_bytes(value, sizeof(*value), &before);
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
    bad.projectile = NULL;
    valid = valid && rejected(value, &environment, bad);
    value->source.collision_profile = MARKER;
    valid = valid && rejected(value, &environment, value->request);
    *value = before;
    value->source.launch_parameter = 0;
    valid = valid && rejected(value, &environment, value->request);
    *value = before;
    value->random.next_stream = FIST_RANDOM_STREAMS;
    valid = valid && rejected(value, &environment, value->request);
    *value = before;
    value->source.allocation.value = (uint16_t)(value->source.allocation.value + 1);
    valid = valid && rejected(value, &environment, value->request);
    *value = before;
    if (value->actor.allocation.type != WRECK_TYPE) {
        value->actor.allocation.value = (uint16_t)(value->actor.allocation.value + 1);
        valid = valid && rejected(value, &environment, value->request);
        *value = before;
    }
    return valid ? 0 : -1;
}

static int check_destroyed_transition(other_case *value, const fist_damage_environment *environment,
                                      const fist_other_damage_result *result) {
    if (result->released) {
        return rejected(value, environment, value->request) ? 0 : -1;
    }
    const other_case before = *value;
    fist_other_damage_result repeated = {0};
    return apply(value, environment, value->request, &repeated) == 0 &&
                   same_bytes(value, sizeof(*value), &before) && repeated.applied_damage == 0 &&
                   !repeated.destroyed && !repeated.released && !repeated.has_explosion &&
                   repeated.sound_request == FIST_DAMAGE_NO_REQUEST &&
                   repeated.voice_request == FIST_DAMAGE_NO_REQUEST &&
                   repeated.refresh_damage_display
               ? 0
               : -1;
}

static int reach_hit(other_case *value) {
    fist_collision_body bodies[FIST_UNIT_REGISTRY_COUNT] = {0};
    fist_object_pose poses[FIST_UNIT_REGISTRY_COUNT] = {0};
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        poses[index] = (fist_object_pose){fist_position_add(value->actor.pose.x, CLEARANCE),
                                          fist_position_add(value->actor.pose.y, CLEARANCE),
                                          value->actor.pose.altitude, 0};
        bodies[index].pose = &poses[index];
    }
    bodies[value->actor.allocation.slot] = (fist_collision_body){
        &value->actor.pose, value->actor.projection_scale, value->actor.flags, value->actor.mode};
    value->source.pose.heading = 0;
    value->source.pose.y = fist_position_add(value->actor.pose.y, -FLIGHT_Y * FLIGHT_TICKS);
    value->source.velocity.y = FLIGHT_Y;
    value->source.collision_grace = 2;
    value->source.origin_slot = FIST_POOL_SHORT_SLOTS;
    value->source.phase = FIST_PROJECTILE_FLYING;
    bodies[value->source.allocation.slot] =
        (fist_collision_body){&value->source.pose, 0, value->source.flags, 0};
    const fist_projectile_environment environment = {
        &value->pool, {&value->pool, bodies, FIST_UNIT_REGISTRY_COUNT}, NULL, &value->random};
    fist_projectile_step step = {0};
    for (size_t tick = 0; tick < FLIGHT_TICKS; ++tick) {
        if (fist_projectile_advance(&value->source, &environment, &step) != 0) {
            return -1;
        }
        printf("flight %u %u %u %ld %ld %ld %u %u\n", (unsigned)step.phase, (unsigned)step.hit.slot,
               (unsigned)step.hit.aspect, (long)value->source.pose.x, (long)value->source.pose.y,
               (long)value->source.pose.altitude, (unsigned)value->source.age,
               (unsigned)value->source.collision_grace);
    }
    if (step.phase != FIST_PROJECTILE_UNIT_IMPACT ||
        step.hit.slot != value->actor.allocation.slot) {
        return -1;
    }
    value->request.hit = step.hit;
    return 0;
}

static int finish_and_animate(other_case *value, fist_other_damage_result *result) {
    fist_projectile_impact impact = {0};
    if (value->finish != 0) {
        if (fist_projectile_finish_impact(&value->pool, &value->source, &impact) != 0) {
            return -1;
        }
        printf("impact %u %u %u %u\n", (unsigned)impact.has_explosion, (unsigned)impact.notice,
               (unsigned)impact.sound_request, (unsigned)impact.hit_voice);
        if (impact.has_explosion) {
            write_effect(&impact.explosion);
        }
        fist_probe_write_object_pool(&value->pool);
    }
    for (size_t tick = 0; tick < value->effect_ticks; ++tick) {
        fist_explosion *effects[] = {&result->explosion, &impact.explosion};
        const bool present[] = {result->has_explosion, impact.has_explosion};
        for (size_t index = 0; index < 2; ++index) {
            if (present[index]) {
                if ((effects[index]->flags & DELETED_FLAG) == 0 &&
                    fist_explosion_advance(&value->pool, effects[index]) != 0) {
                    return -1;
                }
                write_effect(effects[index]);
            }
        }
        fist_probe_write_object_pool(&value->pool);
    }
    return 0;
}

static int observe(other_case *value) {
    if (value->reach != 0 && reach_hit(value) != 0) {
        return -1;
    }
    const fist_damage_environment environment = {&value->pool, &value->random, &value->combat};
    fist_other_damage_result result = {0};
    for (size_t index = 0; index < value->steps; ++index) {
        const fist_projectile before = value->source;
        if (apply(value, &environment, value->request, &result) != 0 ||
            !same_bytes(&before, sizeof(before), &value->source)) {
            return -1;
        }
        printf("result %u %u %u %u %u %u %u\n", (unsigned)result.applied_damage,
               (unsigned)result.destroyed, (unsigned)result.released,
               (unsigned)result.has_explosion, (unsigned)result.sound_request,
               (unsigned)result.voice_request, (unsigned)result.refresh_damage_display);
        if (result.has_explosion) {
            write_effect(&result.explosion);
        }
        write_actor(&value->actor);
        write_shared(value);
        if (result.destroyed || result.released) {
            if (check_destroyed_transition(value, &environment, &result) != 0) {
                return -1;
            }
            break;
        }
    }
    return finish_and_animate(value, &result);
}

static int run(uint8_t *input, size_t size) {
    if (size < FILE_HEADER) {
        free(input);
        return -1;
    }
    const size_t count = fist_read_u32le(input);
    if (count == 0 || count > (size - FILE_HEADER) / CASE_HEADER) {
        free(input);
        return -1;
    }
    other_case *values = calloc(count, sizeof(*values));
    if (values == NULL) {
        free(input);
        return -1;
    }
    size_t offset = FILE_HEADER;
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        if (size - offset < CASE_HEADER) {
            status = -1;
            break;
        }
        const size_t extent = CASE_HEADER + FIST_UNIT_SHORT_SIZE +
                              (fist_read_u16le(input + offset + IMPORTS) * BINDING_BYTES) +
                              (FIST_UNIT_ROSTER_COUNT * sizeof(uint16_t));
        if (extent > size - offset || prepare(&values[index], input + offset, extent) != 0) {
            status = -1;
            break;
        }
        offset += extent;
    }
    free(input);
    if (offset != size) {
        status = -1;
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        if (invalid_inputs(&values[index]) != 0 || observe(&values[index]) != 0) {
            status = -1;
        }
    }
    free(values);
    return status;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *input = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (input == NULL || closed != 0) {
        free(input);
        return EXIT_FAILURE;
    }
    return run(input, size) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
