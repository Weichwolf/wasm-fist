#include "assets/scenario.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Read through EOF rather than guessing an asset's size or truncating a prefix. */
static uint8_t *read_file(FILE *file, size_t *size) {
    enum { INITIAL_CAPACITY = 1024 };
    size_t capacity = INITIAL_CAPACITY;
    uint8_t *data = malloc(capacity);
    *size = 0;
    if (data == NULL) {
        return NULL;
    }
    for (;;) {
        *size += fread(data + *size, 1, capacity - *size, file);
        if (ferror(file) != 0) {
            free(data);
            return NULL;
        }
        if (feof(file) != 0) {
            return data;
        }
        if (capacity > SIZE_MAX / 2) {
            free(data);
            return NULL;
        }
        capacity *= 2;
        uint8_t *grown = realloc(data, capacity);
        if (grown == NULL) {
            free(data);
            return NULL;
        }
        data = grown;
    }
}

static int is_empty(const fist_scenario *scenario) {
    if (scenario->version != 0 || scenario->mode != 0 || scenario->limit != 0 ||
        scenario->unit_count != 0) {
        return 0;
    }
    for (size_t index = 0; index < FIST_SCENARIO_POSITION_COUNT; ++index) {
        if (scenario->map_positions[index] != 0) {
            return 0;
        }
    }
    for (size_t index = 0; index < FIST_SCENARIO_ASSET_COUNT; ++index) {
        for (size_t offset = 0; offset <= FIST_SCENARIO_NAME_SIZE; ++offset) {
            if (scenario->asset_names[index][offset] != 0) {
                return 0;
            }
        }
    }
    for (size_t index = 0; index < FIST_SCENARIO_CHUNK_COUNT; ++index) {
        if (scenario->chunks[index].data != NULL || scenario->chunks[index].size != 0) {
            return 0;
        }
    }
    return 1;
}

static int check_prefixes(const uint8_t *data, size_t size) {
    fist_scenario out = {0};
    for (size_t length = 0; length < size; ++length) {
        if (fist_scenario_decode(data, length, &out) != -1 || is_empty(&out) == 0) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    const int prefixes = argc == 3 && strcmp(argv[1], "--prefixes") == 0;
    if (argc != 2 && prefixes == 0) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[argc - 1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    if (data == NULL || closed != 0 || fist_scenario_decode(data, size, &scenario) != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    if (prefixes != 0) {
        const int result = check_prefixes(data, size);
        free(data);
        return result;
    }
    printf("header %u %u %u\n", (unsigned)scenario.version, (unsigned)scenario.mode,
           (unsigned)scenario.limit);
    for (size_t index = 0; index < FIST_SCENARIO_POSITION_COUNT; ++index) {
        printf("position %zu %" PRId32 "\n", index, scenario.map_positions[index]);
    }
    for (size_t index = 0; index < FIST_SCENARIO_ASSET_COUNT; ++index) {
        printf("asset %zu %s\n", index, scenario.asset_names[index]);
    }
    printf("units %u\n", (unsigned)scenario.unit_count);
    fist_scenario_unit_iterator iterator = fist_scenario_units_begin(&scenario);
    fist_scenario_unit unit = {0};
    while (fist_scenario_units_next(&iterator, &unit) == 1) {
        printf("unit %u %u %zu", (unsigned)unit.catalog_index, (unsigned)unit.catalog_value,
               unit.state.size);
        for (size_t index = 0; index < unit.state.size; ++index) {
            printf(" %02x", (unsigned)unit.state.data[index]);
        }
        printf("\n");
    }
    for (size_t index = 0; index < FIST_SCENARIO_CHUNK_COUNT; ++index) {
        printf("chunk %zu %zu", index, scenario.chunks[index].size);
        for (size_t offset = 0; offset < scenario.chunks[index].size; ++offset) {
            printf(" %02x", (unsigned)scenario.chunks[index].data[offset]);
        }
        printf("\n");
    }
    free(data);
    return EXIT_SUCCESS;
}
