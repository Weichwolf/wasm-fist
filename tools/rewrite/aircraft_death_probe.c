#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/units.h"
#include "combat_probe_io.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/aircraft_death.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/vehicle_damage.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 36,
    BINDING_SIZE = 6,
    STREAM = 8,
    SMOKE_SETTING = 9,
    COARSE = 10,
    OPERATION = 11,
    TICK = 12,
    ANIMATION = 14,
    TICKS = 16,
    SIDE = 18,
    COUNT = 20,
    TARGET = 22,
    RELEASE_FIRST = 24,
    RELEASE_SECOND = 26,
    WIND_X = 28,
    WIND_Y = 32,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    HEADING = 16,
    APACHE = 5,
    HIND = 6,
    SOURCE = 8,
    PRIMARY_PARAMETER = 5,
    DAMAGE_SCALE = 256,
    DEATH_BEHAVIOR = 12,
    MAX_SIDE = 4096,
    MARKER = 123
};

typedef struct {
    fist_object_pool pool;
    fist_random random;
    fist_other_actor actor;
    fist_pool_allocation source;
    fist_klc_image height;
    fist_aircraft_environment environment;
    fist_smoke_weather weather;
    fist_drifting_smoke smokes[FIST_POOL_SHORT_SLOTS];
    fist_explosion effects[FIST_POOL_SHORT_SLOTS];
    bool smoke_present[FIST_POOL_SHORT_SLOTS];
    bool effect_present[FIST_POOL_SHORT_SLOTS];
    uint16_t ticks;
    uint8_t operation;
    bool live;
} aircraft_case;

static void snapshot(const void *object, uint8_t *out, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        out[index] = bytes[index];
    }
}

static int unchanged(const void *object, const uint8_t *before, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        if (bytes[index] != before[index]) {
            return 0;
        }
    }
    return 1;
}

static int import_bindings(aircraft_case *value, const uint8_t *input, size_t pixels) {
    const size_t count = fist_read_u16le(input + COUNT);
    const size_t primary = fist_read_u16le(input + TARGET);
    const uint16_t releases[2] = {fist_read_u16le(input + RELEASE_FIRST),
                                  fist_read_u16le(input + RELEASE_SECOND)};
    fist_pool_allocation allocations[FIST_UNIT_REGISTRY_COUNT] = {0};
    fist_object_pool_reset(&value->pool);
    const uint8_t *bindings = input + HEADER + FIST_UNIT_SHORT_SIZE + pixels;
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = bindings + (index * BINDING_SIZE);
        const fist_pool_import request = {fist_read_u16le(record), fist_read_u16le(record + 2),
                                          fist_read_u16le(record + 4)};
        if (fist_object_pool_import(&value->pool, request, &allocations[index]) != FIST_POOL_OK) {
            return -1;
        }
    }
    for (size_t index = 0; index < 2; ++index) {
        if (releases[index] != FIST_POOL_NO_SLOT) {
            if (releases[index] >= count || releases[index] == primary || value->operation ||
                fist_object_pool_release(&value->pool, allocations[releases[index]].registry_index,
                                         &allocations[releases[index]]) != FIST_POOL_OK) {
                return -1;
            }
        }
    }
    value->actor.allocation = allocations[primary];
    value->source = allocations[0];
    return fist_object_pool_is_current(&value->pool, value->actor.allocation) ? 0 : -1;
}

static int restore(aircraft_case *value, const uint8_t *raw) {
    const fist_pool_allocation allocation = value->actor.allocation;
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .registry_index = allocation.registry_index,
                                             .generation = allocation.value,
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .heading = fist_read_u16le(raw + HEADING),
                                             .snapshot = {raw, FIST_UNIT_SHORT_SIZE}};
    if ((allocation.type != APACHE && allocation.type != HIND) ||
        fist_other_actor_restore(&definition, allocation, &value->actor) != 0 ||
        (!value->operation && value->actor.state.pair.behavior != DEATH_BEHAVIOR) ||
        (value->operation && (value->source.type != SOURCE ||
                              !fist_object_pool_is_current(&value->pool, value->source)))) {
        return -1;
    }
    return 0;
}

static int prepare(aircraft_case *value, const uint8_t *input, size_t size) {
    const size_t side = fist_read_u16le(input + SIDE);
    const size_t count = fist_read_u16le(input + COUNT);
    if (side == 0 || side > MAX_SIDE || (side & (side - 1)) != 0 || count == 0 ||
        count > FIST_UNIT_REGISTRY_COUNT || fist_read_u16le(input + TARGET) >= count ||
        input[STREAM] >= FIST_RANDOM_STREAMS || input[COARSE] > 1 || input[OPERATION] > 1 ||
        size != HEADER + FIST_UNIT_SHORT_SIZE + (side * side) + (count * BINDING_SIZE)) {
        return -1;
    }
    value->operation = input[OPERATION];
    value->ticks = fist_read_u16le(input + TICKS);
    value->live = true;
    value->random.next_stream = input[STREAM];
    for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
        value->random.words[stream] = fist_read_u16le(input + (stream * sizeof(uint16_t)));
    }
    value->height = (fist_klc_image){.width = (uint32_t)side, .height = (uint32_t)side};
    value->height.pixels = malloc(side * side);
    if (value->height.pixels == NULL) {
        return -1;
    }
    for (size_t index = 0; index < side * side; ++index) {
        value->height.pixels[index] = input[HEADER + FIST_UNIT_SHORT_SIZE + index];
    }
    value->environment = (fist_aircraft_environment){&value->pool,
                                                     &value->random,
                                                     &value->height,
                                                     fist_read_u16le(input + TICK),
                                                     fist_read_u16le(input + ANIMATION),
                                                     input[SMOKE_SETTING],
                                                     input[COARSE] != 0};
    value->weather = (fist_smoke_weather){fist_read_i32le(input + WIND_X),
                                          fist_read_i32le(input + WIND_Y), input[SMOKE_SETTING]};
    if (import_bindings(value, input, side * side) != 0 || restore(value, input + HEADER) != 0) {
        return -1;
    }
    return 0;
}

static void write_shared(const aircraft_case *value) {
    printf("random %u", (unsigned)value->random.next_stream);
    for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
        printf(" %u", (unsigned)value->random.words[stream]);
    }
    printf("\n");
    fist_probe_write_object_pool(&value->pool);
}

static void accept_effect(aircraft_case *value, const fist_explosion *effect) {
    value->effects[effect->allocation.slot] = *effect;
    value->effect_present[effect->allocation.slot] = true;
    fist_probe_write_explosion(effect);
}

static int damage(aircraft_case *value) {
    fist_projectile source = {.allocation = value->source,
                              .pose = value->actor.pose,
                              .phase = FIST_PROJECTILE_UNIT_IMPACT,
                              .launch_parameter = PRIMARY_PARAMETER};
    uint8_t before[sizeof(source)];
    snapshot(&source, before, sizeof(before));
    fist_combat_state combat = {.selected_slot = FIST_POOL_NO_SLOT};
    combat.source_scale[0] = combat.source_scale[1] = DAMAGE_SCALE;
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        combat.roster[index] = FIST_POOL_NO_SLOT;
    }
    const fist_damage_environment environment = {&value->pool, &value->random, &combat};
    const fist_vehicle_damage_request request = {&source,
                                                 {value->actor.allocation.slot,
                                                  value->actor.allocation.registry_index,
                                                  value->actor.allocation.value, 0}};
    fist_other_damage_result result = {0};
    if (fist_other_damage_m1(&value->actor, &environment, request, &result) != 0 ||
        !unchanged(&source, before, sizeof(before))) {
        return -1;
    }
    value->live = !result.released;
    printf("damage %u %u %u %u\n", (unsigned)result.applied_damage, (unsigned)result.destroyed,
           (unsigned)result.released, (unsigned)result.sound_request);
    if (result.has_explosion) {
        accept_effect(value, &result.explosion);
    }
    fist_projectile_impact impact = {0};
    if (fist_projectile_finish_impact(&value->pool, &source, &impact) != 0) {
        return -1;
    }
    if (impact.has_explosion) {
        accept_effect(value, &impact.explosion);
    }
    printf("impact %u\n", (unsigned)impact.has_explosion);
    return 0;
}

static int invalid(aircraft_case *value) {
    uint8_t before[sizeof(*value)];
    snapshot(value, before, sizeof(before));
    fist_aircraft_death_step out = {.sound_request = MARKER};
    uint8_t marker[sizeof(out)];
    snapshot(&out, marker, sizeof(marker));
    fist_aircraft_environment bad = value->environment;
    bad.random = NULL;
    int valid = fist_aircraft_death_advance(NULL, &value->environment, &out) == -1 &&
                fist_aircraft_death_advance(&value->actor, NULL, &out) == -1 &&
                fist_aircraft_death_advance(&value->actor, &bad, &out) == -1 &&
                fist_aircraft_death_advance(&value->actor, &value->environment, NULL) == -1 &&
                unchanged(value, before, sizeof(before));
    bad = value->environment;
    bad.height = NULL;
    valid = valid && fist_aircraft_death_advance(&value->actor, &bad, &out) == -1 &&
            unchanged(value, before, sizeof(before));
    const uint8_t saved_cursor = value->random.next_stream;
    value->random.next_stream = FIST_RANDOM_STREAMS;
    snapshot(value, before, sizeof(before));
    valid = valid && fist_aircraft_death_advance(&value->actor, &value->environment, &out) == -1 &&
            unchanged(value, before, sizeof(before));
    value->random.next_stream = saved_cursor;
    const uint8_t saved_behavior = value->actor.state.pair.behavior;
    value->actor.state.pair.behavior = 0;
    snapshot(value, before, sizeof(before));
    valid = valid && fist_aircraft_death_advance(&value->actor, &value->environment, &out) == -1 &&
            unchanged(value, before, sizeof(before));
    value->actor.state.pair.behavior = saved_behavior;
    const uint16_t saved = value->actor.allocation.value;
    value->actor.allocation.value = (uint16_t)(saved + 1);
    snapshot(value, before, sizeof(before));
    valid = valid && fist_aircraft_death_advance(&value->actor, &value->environment, &out) == -1 &&
            unchanged(value, before, sizeof(before)) && unchanged(&out, marker, sizeof(marker));
    value->actor.allocation.value = saved;
    return valid ? 0 : -1;
}

static int advance_effects(aircraft_case *value) {
    for (size_t slot = 0; slot < FIST_POOL_SHORT_SLOTS; ++slot) {
        if (value->smoke_present[slot]) {
            if (fist_drifting_smoke_advance(&value->pool, &value->smokes[slot], value->weather) !=
                0) {
                return -1;
            }
            fist_probe_write_smoke(&value->smokes[slot]);
            value->smoke_present[slot] =
                fist_object_pool_is_current(&value->pool, value->smokes[slot].allocation);
        }
    }
    for (size_t slot = 0; slot < FIST_POOL_SHORT_SLOTS; ++slot) {
        if (value->effect_present[slot]) {
            if (fist_explosion_advance(&value->pool, &value->effects[slot]) != 0) {
                return -1;
            }
            fist_probe_write_explosion(&value->effects[slot]);
            value->effect_present[slot] =
                fist_object_pool_is_current(&value->pool, value->effects[slot].allocation);
        }
    }
    return 0;
}

static int observe(aircraft_case *value) {
    if (value->operation && damage(value) != 0) {
        return -1;
    }
    fist_probe_write_other_actor(&value->actor);
    write_shared(value);
    for (size_t tick = 0; tick < value->ticks; ++tick) {
        const bool updated = value->live;
        fist_aircraft_death_step result = {.sound_request = FIST_DAMAGE_NO_REQUEST};
        if (updated) {
            if (invalid(value) != 0 ||
                fist_aircraft_death_advance(&value->actor, &value->environment, &result) != 0) {
                return -1;
            }
            value->live = !result.released;
        }
        printf("tick %u %u %u %u %u %u %u\n", (unsigned)value->environment.tick,
               (unsigned)value->environment.animation_phase, (unsigned)updated,
               (unsigned)result.released, (unsigned)result.sound_request,
               (unsigned)result.has_explosion, (unsigned)result.has_smoke);
        if (updated) {
            fist_probe_write_other_actor(&value->actor);
        }
        if (result.has_explosion) {
            accept_effect(value, &result.explosion);
        }
        if (result.has_smoke) {
            const size_t slot = result.smoke.allocation.slot;
            value->smokes[slot] = result.smoke;
            value->smoke_present[slot] = true;
            fist_probe_write_smoke(&result.smoke);
        }
        if (advance_effects(value) != 0) {
            return -1;
        }
        write_shared(value);
        value->environment.tick = (uint16_t)(value->environment.tick + 1);
        value->environment.animation_phase = (uint16_t)(value->environment.animation_phase + 1);
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
    aircraft_case *values = calloc(count, sizeof(*values));
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
        const size_t side = fist_read_u16le(input + offset + SIDE);
        const size_t extent = HEADER + FIST_UNIT_SHORT_SIZE + (side * side) +
                              ((size_t)fist_read_u16le(input + offset + COUNT) * BINDING_SIZE);
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
    for (size_t index = 0; index < count; ++index) {
        free(values[index].height.pixels);
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
