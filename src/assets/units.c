#include "assets/units.h"
#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    POOL_INDEX_OFFSET = 2,
    X_OFFSET = 4,
    Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HEADING_OFFSET = 16,
    FLAGS_OFFSET = 22,
    SECONDARY_FLAGS_OFFSET = 23,
    PLATOON_OFFSET = 27,
    MEMBER_OFFSET = 28,
    PLACEHOLDER_PLATOON_OFFSET = 35,
    PLACEHOLDER_MEMBER_OFFSET = 36,
    ROSTER_FLAG = 32
};

size_t fist_unit_state_size(uint16_t type) {
    if (type >= FIST_UNIT_TYPE_COUNT) {
        return 0;
    }
    return type < FIST_UNIT_GROUND_VEHICLE_COUNT || type == FIST_UNIT_EXTENDED_RESERVED_TYPE
               ? FIST_UNIT_EXTENDED_SIZE
               : FIST_UNIT_SHORT_SIZE;
}

static int decode_definition(const fist_scenario_unit *record, fist_unit_definition *out) {
    if (record->state.size < FIST_UNIT_SHORT_SIZE ||
        record->catalog_index >= FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    const uint8_t *state = record->state.data;
    const uint16_t type = fist_read_u16le(state);
    const size_t expected_size = fist_unit_state_size(type);
    if (expected_size == 0 || record->state.size != expected_size) {
        return -1;
    }
    *out = (fist_unit_definition){
        .type = type,
        .registry_index = record->catalog_index,
        .generation = record->catalog_value,
        .saved_pool_index = fist_read_u16le(state + POOL_INDEX_OFFSET),
        .map_x = fist_read_i32le(state + X_OFFSET),
        .map_y = fist_read_i32le(state + Y_OFFSET),
        .altitude = fist_read_i32le(state + ALTITUDE_OFFSET),
        .heading = fist_read_u16le(state + HEADING_OFFSET),
        .flags = state[FLAGS_OFFSET],
        .secondary_flags = state[SECONDARY_FLAGS_OFFSET],
        .platoon = state[type == FIST_UNIT_ROSTER_PLACEHOLDER ? PLACEHOLDER_PLATOON_OFFSET
                                                              : PLATOON_OFFSET],
        .member =
            state[type == FIST_UNIT_ROSTER_PLACEHOLDER ? PLACEHOLDER_MEMBER_OFFSET : MEMBER_OFFSET],
        .snapshot = record->state};
    return 0;
}

static int assign_roster(fist_units *units, size_t index) {
    const fist_unit_definition *definition = &units->definitions[index];
    if (definition->type != FIST_UNIT_ROSTER_PLACEHOLDER &&
        (definition->flags & ROSTER_FLAG) == 0) {
        return 0;
    }
    if (definition->platoon >= FIST_UNIT_PLATOON_COUNT ||
        definition->member >= FIST_UNIT_MEMBERS_PER_PLATOON) {
        return -1;
    }
    const size_t slot =
        ((size_t)definition->platoon * FIST_UNIT_MEMBERS_PER_PLATOON) + (size_t)definition->member;
    if (definition->type != FIST_UNIT_ROSTER_PLACEHOLDER || slot != 0) {
        units->roster[slot] = (uint16_t)index;
    }
    return 0;
}

void fist_units_destroy(fist_units *units) {
    if (units != NULL) {
        free(units->definitions);
        free(units->storage);
        *units = (fist_units){0};
    }
}

int fist_units_decode(const fist_scenario *scenario, fist_units *out) {
    if (scenario == NULL || out == NULL || scenario->chunks[FIST_SCENARIO_UNITS].data == NULL ||
        scenario->chunks[FIST_SCENARIO_UNITS].size < sizeof(uint16_t)) {
        return -1;
    }
    fist_units units = {.count = scenario->unit_count};
    const fist_asset_view source = scenario->chunks[FIST_SCENARIO_UNITS];
    units.storage = malloc(source.size);
    if (units.count != 0) {
        units.definitions = calloc(units.count, sizeof(*units.definitions));
    }
    if (units.storage == NULL || (units.count != 0 && units.definitions == NULL)) {
        fist_units_destroy(&units);
        return -1;
    }
    for (size_t index = 0; index < source.size; ++index) {
        units.storage[index] = source.data[index];
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        units.registry[index] = FIST_UNIT_NO_DEFINITION;
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        units.roster[index] = FIST_UNIT_NO_DEFINITION;
    }
    fist_scenario owned = *scenario;
    owned.chunks[FIST_SCENARIO_UNITS].data = units.storage;
    fist_scenario_unit_iterator iterator = fist_scenario_units_begin(&owned);
    fist_scenario_unit record = {0};
    for (size_t index = 0; index < units.count; ++index) {
        if (fist_scenario_units_next(&iterator, &record) != 1 ||
            decode_definition(&record, &units.definitions[index]) != 0 ||
            assign_roster(&units, index) != 0) {
            fist_units_destroy(&units);
            return -1;
        }
        units.registry[record.catalog_index] = (uint16_t)index;
    }
    if (fist_scenario_units_next(&iterator, &record) != 0) {
        fist_units_destroy(&units);
        return -1;
    }
    *out = units;
    return 0;
}

const fist_unit_definition *fist_units_roster_get(const fist_units *units, size_t slot) {
    if (units == NULL || slot >= FIST_UNIT_ROSTER_COUNT || units->definitions == NULL ||
        units->roster[slot] >= units->count) {
        return NULL;
    }
    return &units->definitions[units->roster[slot]];
}
