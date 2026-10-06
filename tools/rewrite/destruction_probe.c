#include "assets/bytes.h"
#include "assets/units.h"
#include "combat_probe_io.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/destruction_updates.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 32,
    RANDOM_OFFSET = 2,
    STREAM_OFFSET = 10,
    RESERVED_BYTE_OFFSET = 11,
    COUNT_OFFSET = 12,
    TICKS_OFFSET = 14,
    PARENT_TICKS_OFFSET = 16,
    STRENGTH_OFFSET = 18,
    WIND_X_OFFSET = 20,
    WIND_Y_OFFSET = 24,
    PRIMARY_OFFSET = 28,
    RESERVED_WORD_OFFSET = 30,
    X_OFFSET = 4,
    Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HEADING_OFFSET = 16,
    BINDING_SIZE = 6,
    WRECK = 23,
    TARGET = 26,
    ARTILLERY = 27,
    SMOKE = 17,
    SOURCE = 8,
    MARKER = 123,
    DAMAGE_SCALE = 256,
    PRIMARY_PARAMETER = 5
};

typedef struct {
    fist_object_pool pool;
    fist_random random;
    fist_vehicle_wreck wreck;
    fist_other_actor actor;
    fist_drifting_smoke smoke[FIST_POOL_SHORT_SLOTS];
    bool present[FIST_POOL_SHORT_SLOTS];
    fist_pool_allocation primary;
    fist_pool_allocation source;
    fist_object_pose pose;
    uint16_t ticks;
    uint16_t parent_ticks;
    uint16_t strength;
    fist_smoke_weather weather;
    uint8_t operation;
} destruction_case;

/* Capture bytes from the same object before an invalid call. This tests writes
 * without comparing padding from independently initialized/copied structures. */
static void snapshot_bytes(const void *object, uint8_t *snapshot, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        snapshot[index] = bytes[index];
    }
}

static int unchanged_bytes(const void *object, const uint8_t *snapshot, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        if (bytes[index] != snapshot[index]) {
            return 0;
        }
    }
    return 1;
}

static int invalid_snapshot(const fist_unit_definition *definition,
                            fist_pool_allocation allocation) {
    fist_unit_definition bad = *definition;
    --bad.snapshot.size;
    if (definition->type == SMOKE) {
        const fist_drifting_smoke marker = {.flags = MARKER};
        fist_drifting_smoke out = marker;
        uint8_t before[sizeof(out)];
        snapshot_bytes(&out, before, sizeof(before));
        return fist_drifting_smoke_restore(NULL, allocation, &out) == -1 &&
                       fist_drifting_smoke_restore(definition, allocation, NULL) == -1 &&
                       fist_drifting_smoke_restore(&bad, allocation, &out) == -1 &&
                       unchanged_bytes(&out, before, sizeof(before))
                   ? 0
                   : -1;
    }
    if (definition->type == WRECK) {
        const fist_vehicle_wreck marker = {.flags = MARKER};
        fist_vehicle_wreck out = marker;
        uint8_t before[sizeof(out)];
        snapshot_bytes(&out, before, sizeof(before));
        return fist_vehicle_wreck_restore(NULL, allocation, &out) == -1 &&
                       fist_vehicle_wreck_restore(definition, allocation, NULL) == -1 &&
                       fist_vehicle_wreck_restore(&bad, allocation, &out) == -1 &&
                       unchanged_bytes(&out, before, sizeof(before))
                   ? 0
                   : -1;
    }
    return 0;
}

static int definition_restore(destruction_case *value, const uint8_t *raw) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .registry_index = value->primary.registry_index,
                                             .generation = value->primary.value,
                                             .map_x = fist_read_i32le(raw + X_OFFSET),
                                             .map_y = fist_read_i32le(raw + Y_OFFSET),
                                             .altitude = fist_read_i32le(raw + ALTITUDE_OFFSET),
                                             .heading = fist_read_u16le(raw + HEADING_OFFSET),
                                             .snapshot = {raw, FIST_UNIT_SHORT_SIZE}};
    value->pose = (fist_object_pose){definition.map_x, definition.map_y, definition.altitude,
                                     definition.heading};
    if (definition.type != value->primary.type ||
        invalid_snapshot(&definition, value->primary) != 0) {
        return -1;
    }
    if (value->operation == 0) {
        return 0;
    }
    if (value->operation == 1) {
        value->present[value->primary.slot] = true;
        return fist_drifting_smoke_restore(&definition, value->primary,
                                           &value->smoke[value->primary.slot]);
    }
    return definition.type == WRECK
               ? fist_vehicle_wreck_restore(&definition, value->primary, &value->wreck)
               : fist_other_actor_restore(&definition, value->primary, &value->actor);
}

static int prepare(destruction_case *value, const uint8_t *input, size_t size) {
    const size_t count = fist_read_u16le(input + COUNT_OFFSET);
    if (input[0] > 3 || input[STREAM_OFFSET] >= FIST_RANDOM_STREAMS ||
        input[RESERVED_BYTE_OFFSET] || fist_read_u16le(input + PRIMARY_OFFSET) >= count ||
        fist_read_u16le(input + RESERVED_WORD_OFFSET) || count == 0 ||
        count > FIST_UNIT_REGISTRY_COUNT ||
        size != HEADER + FIST_UNIT_SHORT_SIZE + (count * BINDING_SIZE)) {
        return -1;
    }
    value->operation = input[0];
    value->weather = (fist_smoke_weather){fist_read_i32le(input + WIND_X_OFFSET),
                                          fist_read_i32le(input + WIND_Y_OFFSET), input[1]};
    value->random.next_stream = input[STREAM_OFFSET];
    for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
        value->random.words[stream] =
            fist_read_u16le(input + RANDOM_OFFSET + (stream * sizeof(uint16_t)));
    }
    value->ticks = fist_read_u16le(input + TICKS_OFFSET);
    value->parent_ticks = fist_read_u16le(input + PARENT_TICKS_OFFSET);
    value->strength = fist_read_u16le(input + STRENGTH_OFFSET);
    if (value->parent_ticks > value->ticks) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    const uint8_t *bindings = input + HEADER + FIST_UNIT_SHORT_SIZE;
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = bindings + (index * BINDING_SIZE);
        const fist_pool_import request = {fist_read_u16le(record), fist_read_u16le(record + 2),
                                          fist_read_u16le(record + 4)};
        fist_pool_allocation allocation = {0};
        if (fist_object_pool_import(&value->pool, request, &allocation) != 0) {
            return -1;
        }
        if (index == fist_read_u16le(input + PRIMARY_OFFSET)) {
            value->primary = allocation;
        }
        if (index == 0 && value->operation == 3) {
            value->source = allocation;
        }
    }
    if (value->primary.slot >= FIST_POOL_SHORT_SLOTS ||
        !fist_object_pool_is_current(&value->pool, value->primary) ||
        (value->operation == 1 && value->primary.type != SMOKE) ||
        (value->operation >= 2 && value->primary.type != WRECK && value->primary.type != TARGET &&
         value->primary.type != ARTILLERY) ||
        (value->operation == 3 && (value->primary.type == WRECK || value->source.type != SOURCE ||
                                   !fist_object_pool_is_current(&value->pool, value->source)))) {
        return -1;
    }
    return definition_restore(value, input + HEADER);
}

static void write_parent(const destruction_case *value) {
    if (value->primary.type == WRECK) {
        const fist_vehicle_wreck *wreck = &value->wreck;
        printf("wreck %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u %u %u\n",
               (unsigned)wreck->allocation.slot, (unsigned)wreck->allocation.registry_index,
               (unsigned)wreck->allocation.value, (long)wreck->pose.x, (long)wreck->pose.y,
               (long)wreck->pose.altitude, (unsigned)wreck->pose.heading,
               (unsigned)wreck->model_code, (unsigned)wreck->original_type,
               (unsigned)wreck->projection_scale, (unsigned)wreck->parameter,
               (unsigned)wreck->platoon, (unsigned)wreck->member, (unsigned)wreck->flags,
               (unsigned)wreck->secondary_flags, (unsigned)wreck->emission_counter);
        return;
    }
    fist_probe_write_other_actor(&value->actor);
}

static void write_shared(const destruction_case *value) {
    printf("random %u", (unsigned)value->random.next_stream);
    for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
        printf(" %u", (unsigned)value->random.words[stream]);
    }
    printf("\n");
    fist_probe_write_object_pool(&value->pool);
}

static int parent_advance(destruction_case *value, const fist_destruction_environment *environment,
                          fist_destruction_step *result) {
    return value->primary.type == WRECK
               ? fist_vehicle_wreck_advance(&value->wreck, environment, result)
               : fist_destroyed_target_advance(&value->actor, environment, result);
}

static int invalid_parent(destruction_case *value) {
    uint8_t before[sizeof(*value)];
    snapshot_bytes(value, before, sizeof(before));
    fist_destruction_step out = {.has_smoke = true, .smoke.flags = MARKER};
    uint8_t marker[sizeof(out)];
    snapshot_bytes(&out, marker, sizeof(marker));
    const fist_destruction_environment environment = {&value->pool, &value->random, 1};
    fist_destruction_environment bad = environment;
    bad.random = NULL;
    int valid = parent_advance(value, NULL, &out) == -1 &&
                parent_advance(value, &bad, &out) == -1 &&
                parent_advance(value, &environment, NULL) == -1 &&
                unchanged_bytes(value, before, sizeof(before));
    fist_pool_allocation *allocation =
        value->primary.type == WRECK ? &value->wreck.allocation : &value->actor.allocation;
    const uint16_t saved = allocation->value;
    allocation->value = (uint16_t)(saved + 1);
    snapshot_bytes(value, before, sizeof(before));
    valid = valid && parent_advance(value, &environment, &out) == -1 &&
            unchanged_bytes(value, before, sizeof(before)) &&
            unchanged_bytes(&out, marker, sizeof(marker));
    allocation->value = saved;
    return valid ? 0 : -1;
}

static int invalid_creation(const destruction_case *value) {
    fist_object_pool pool = value->pool;
    fist_random random = value->random;
    fist_drifting_smoke out = {.flags = MARKER};
    uint8_t pool_before[sizeof(pool)];
    uint8_t random_before[sizeof(random)];
    uint8_t marker[sizeof(out)];
    snapshot_bytes(&pool, pool_before, sizeof(pool_before));
    snapshot_bytes(&out, marker, sizeof(marker));
    const fist_smoke_creation request = {value->strength, 1};
    int valid = fist_drifting_smoke_create(NULL, &random, &value->pose, request, &out) == -1 &&
                fist_drifting_smoke_create(&pool, NULL, &value->pose, request, &out) == -1 &&
                fist_drifting_smoke_create(&pool, &random, NULL, request, &out) == -1 &&
                fist_drifting_smoke_create(&pool, &random, &value->pose, request, NULL) == -1;
    random.next_stream = FIST_RANDOM_STREAMS;
    snapshot_bytes(&random, random_before, sizeof(random_before));
    valid = valid &&
            fist_drifting_smoke_create(&pool, &random, &value->pose, request, &out) == -1 &&
            unchanged_bytes(&random, random_before, sizeof(random_before)) &&
            unchanged_bytes(&pool, pool_before, sizeof(pool_before));
    random = value->random;
    snapshot_bytes(&random, random_before, sizeof(random_before));
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (pool.registry[index].slot == FIST_POOL_NO_SLOT) {
            pool.registry[index].value = 2;
        }
    }
    snapshot_bytes(&pool, pool_before, sizeof(pool_before));
    valid = valid &&
            fist_drifting_smoke_create(&pool, &random, &value->pose, request, &out) ==
                FIST_POOL_UNAVAILABLE &&
            unchanged_bytes(&pool, pool_before, sizeof(pool_before)) &&
            unchanged_bytes(&random, random_before, sizeof(random_before)) &&
            unchanged_bytes(&out, marker, sizeof(marker));
    return valid ? 0 : -1;
}

static int invalid_smoke(destruction_case *value, size_t slot) {
    fist_drifting_smoke *smoke = &value->smoke[slot];
    const uint16_t saved = smoke->allocation.value;
    smoke->allocation.value = (uint16_t)(saved + 1);
    uint8_t before[sizeof(*value)];
    snapshot_bytes(value, before, sizeof(before));
    const int valid = fist_drifting_smoke_advance(&value->pool, smoke, value->weather) == -1 &&
                      fist_drifting_smoke_advance(NULL, smoke, value->weather) == -1 &&
                      fist_drifting_smoke_advance(&value->pool, NULL, value->weather) == -1 &&
                      unchanged_bytes(value, before, sizeof(before));
    smoke->allocation.value = saved;
    return valid ? 0 : -1;
}

static int damage(destruction_case *value, fist_explosion effects[2], bool present[2]) {
    fist_projectile source = {.allocation = value->source,
                              .pose = value->pose,
                              .launch_parameter = PRIMARY_PARAMETER,
                              .phase = FIST_PROJECTILE_UNIT_IMPACT};
    uint8_t before[sizeof(source)];
    snapshot_bytes(&source, before, sizeof(before));
    fist_combat_state combat = {.selected_slot = FIST_POOL_NO_SLOT};
    combat.source_scale[0] = combat.source_scale[1] = DAMAGE_SCALE;
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        combat.roster[index] = FIST_POOL_NO_SLOT;
    }
    const fist_damage_environment environment = {&value->pool, &value->random, &combat};
    const fist_vehicle_damage_request request = {
        &source, {value->primary.slot, value->primary.registry_index, value->primary.value, 0}};
    fist_other_damage_result result = {0};
    if (fist_other_damage_m1(&value->actor, &environment, request, &result) != 0 ||
        !unchanged_bytes(&source, before, sizeof(before))) {
        return -1;
    }
    printf("damage %u %u %u %u\n", (unsigned)result.applied_damage, (unsigned)result.destroyed,
           (unsigned)result.sound_request, (unsigned)result.has_explosion);
    effects[0] = result.explosion;
    present[0] = result.has_explosion;
    fist_projectile_impact impact = {0};
    if (fist_projectile_finish_impact(&value->pool, &source, &impact) != 0) {
        return -1;
    }
    effects[1] = impact.explosion;
    present[1] = impact.has_explosion;
    printf("impact %u\n", (unsigned)impact.has_explosion);
    write_parent(value);
    write_shared(value);
    return 0;
}

static int smoke_advance(destruction_case *value) {
    for (size_t slot = 0; slot < FIST_POOL_SHORT_SLOTS; ++slot) {
        if (value->present[slot]) {
            if (invalid_smoke(value, slot) != 0) {
                return -1;
            }
            if (fist_drifting_smoke_advance(&value->pool, &value->smoke[slot], value->weather) !=
                0) {
                return -1;
            }
            fist_probe_write_smoke(&value->smoke[slot]);
            value->present[slot] =
                fist_object_pool_is_current(&value->pool, value->smoke[slot].allocation);
        }
    }
    return 0;
}

static int begin(destruction_case *value, fist_explosion effects[2], bool effects_present[2]) {
    if (invalid_creation(value) != 0) {
        return -1;
    }
    if (value->operation == 3 && damage(value, effects, effects_present) != 0) {
        return -1;
    }
    if (value->operation == 0) {
        const fist_smoke_creation request = {value->strength, value->weather.enabled};
        fist_drifting_smoke smoke = {.flags = MARKER};
        uint8_t before[sizeof(smoke)];
        snapshot_bytes(&smoke, before, sizeof(before));
        const int status =
            fist_drifting_smoke_create(&value->pool, &value->random, &value->pose, request, &smoke);
        if (status < 0 || (status && !unchanged_bytes(&smoke, before, sizeof(before)))) {
            return -1;
        }
        printf("creation %u\n", (unsigned)status);
        if (!status) {
            value->smoke[smoke.allocation.slot] = smoke;
            value->present[smoke.allocation.slot] = true;
            fist_probe_write_smoke(&smoke);
        }
    } else if (value->operation == 1) {
        fist_probe_write_smoke(&value->smoke[value->primary.slot]);
    } else {
        write_parent(value);
        if (invalid_parent(value) != 0) {
            return -1;
        }
    }
    write_shared(value);
    return 0;
}

static int advance_effects(destruction_case *value, fist_explosion effects[2], bool present[2]) {
    for (size_t index = 0; index < 2; ++index) {
        if (present[index]) {
            if ((effects[index].flags & 1U) == 0 &&
                fist_explosion_advance(&value->pool, &effects[index]) != 0) {
                return -1;
            }
            present[index] = fist_object_pool_is_current(&value->pool, effects[index].allocation);
        }
    }
    return 0;
}

static int advance_parent(destruction_case *value,
                          const fist_destruction_environment *environment) {
    fist_destruction_step result = {0};
    if (parent_advance(value, environment, &result) != 0) {
        return -1;
    }
    printf("emission %u\n", (unsigned)result.has_smoke);
    write_parent(value);
    if (result.has_smoke) {
        value->smoke[result.smoke.allocation.slot] = result.smoke;
        value->present[result.smoke.allocation.slot] = true;
        fist_probe_write_smoke(&result.smoke);
    }
    return 0;
}

static int observe(destruction_case *value) {
    fist_explosion effects[2] = {0};
    bool effects_present[2] = {false};
    if (begin(value, effects, effects_present) != 0) {
        return -1;
    }
    const fist_destruction_environment environment = {&value->pool, &value->random,
                                                      value->weather.enabled};
    for (size_t tick = 0; tick < value->ticks; ++tick) {
        if (value->operation >= 2 && tick < value->parent_ticks &&
            advance_parent(value, &environment) != 0) {
            return -1;
        }
        if (smoke_advance(value) != 0 || advance_effects(value, effects, effects_present) != 0) {
            return -1;
        }
        write_shared(value);
    }
    return 0;
}

static int run(uint8_t *input, size_t size) {
    if (size < sizeof(uint32_t)) {
        free(input);
        return -1;
    }
    const size_t count = fist_read_u32le(input);
    if (count == 0 || count > (size - sizeof(uint32_t)) / HEADER) {
        free(input);
        return -1;
    }
    destruction_case *values = calloc(count, sizeof(*values));
    if (values == NULL) {
        free(input);
        return -1;
    }
    size_t offset = sizeof(uint32_t);
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        if (size - offset < HEADER) {
            status = -1;
            break;
        }
        const size_t extent = HEADER + FIST_UNIT_SHORT_SIZE +
                              (fist_read_u16le(input + offset + COUNT_OFFSET) * BINDING_SIZE);
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
        status = observe(&values[index]);
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
    if (input == NULL || closed) {
        free(input);
        return EXIT_FAILURE;
    }
    return run(input, size) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
