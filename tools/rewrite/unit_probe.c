#include "assets/scenario.h"
#include "assets/units.h"
#include "probe_io.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { CONTRACT_FAILURE = 2, MARKER_COUNT = 137 };

static int marker_intact(const fist_units *units) {
    if (units->definitions != NULL || units->storage != NULL || units->count != MARKER_COUNT) {
        return 0;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (units->registry[index] != index + 1) {
            return 0;
        }
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        if (units->roster[index] != index + 1) {
            return 0;
        }
    }
    return 1;
}

static int destroyed(const fist_units *units) {
    if (units->definitions != NULL || units->storage != NULL || units->count != 0) {
        return 0;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (units->registry[index] != 0) {
            return 0;
        }
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        if (units->roster[index] != 0 || fist_units_roster_get(units, index) != NULL) {
            return 0;
        }
    }
    return 1;
}

static int write_units(const fist_units *units) {
    printf("units %zu\n", units->count);
    for (size_t index = 0; index < units->count; ++index) {
        const fist_unit_definition *definition = &units->definitions[index];
        printf("unit %zu %u %u %u %u %" PRId32 " %" PRId32 " %" PRId32 " %u %u %u %u %u %zu", index,
               (unsigned)definition->type, (unsigned)definition->registry_index,
               (unsigned)definition->generation, (unsigned)definition->saved_pool_index,
               definition->map_x, definition->map_y, definition->altitude,
               (unsigned)definition->heading, (unsigned)definition->flags,
               (unsigned)definition->secondary_flags, (unsigned)definition->platoon,
               (unsigned)definition->member, definition->snapshot.size);
        for (size_t offset = 0; offset < definition->snapshot.size; ++offset) {
            printf(" %02x", (unsigned)definition->snapshot.data[offset]);
        }
        printf("\n");
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        printf("registry %zu %u\n", index, (unsigned)units->registry[index]);
    }
    for (size_t slot = 0; slot < FIST_UNIT_ROSTER_COUNT; ++slot) {
        const fist_unit_definition *definition = fist_units_roster_get(units, slot);
        const size_t index = units->roster[slot];
        if ((index == FIST_UNIT_NO_DEFINITION && definition != NULL) ||
            (index != FIST_UNIT_NO_DEFINITION &&
             (index >= units->count || definition != &units->definitions[index]))) {
            return CONTRACT_FAILURE;
        }
        printf("roster %zu %u\n", slot, (unsigned)units->roster[slot]);
    }
    return ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
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
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    if (data == NULL || closed != 0 || fist_scenario_decode(data, size, &scenario) != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    fist_units units = {.count = MARKER_COUNT};
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        units.registry[index] = (uint16_t)(index + 1);
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        units.roster[index] = (uint16_t)(index + 1);
    }
    if (fist_units_decode(NULL, &units) != -1 || marker_intact(&units) == 0 ||
        fist_units_decode(&scenario, NULL) != -1 || fist_units_roster_get(NULL, 0) != NULL) {
        free(data);
        return CONTRACT_FAILURE;
    }
    if (fist_units_decode(&scenario, &units) != 0) {
        free(data);
        return marker_intact(&units) != 0 ? EXIT_FAILURE : CONTRACT_FAILURE;
    }
    /* Destroy the input before observing every copied byte and typed field. */
    for (size_t index = 0; index < size; ++index) {
        data[index] = 0;
    }
    free(data);
    int result = write_units(&units);
    if (fist_units_roster_get(&units, FIST_UNIT_ROSTER_COUNT) != NULL) {
        result = CONTRACT_FAILURE;
    }
    fist_units_destroy(&units);
    fist_units_destroy(&units);
    fist_units_destroy(NULL);
    return destroyed(&units) != 0 ? result : CONTRACT_FAILURE;
}
