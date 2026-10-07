#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "probe_io.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int orders_failure(const fist_scenario *scenario, const fist_mission_orders *before) {
    fist_mission_orders out = *before;
    return fist_mission_orders_decode(scenario, &out) == -1 &&
           memcmp(&out, before, sizeof(out)) == 0;
}

static int check_order_counts(const fist_scenario *scenario, const fist_mission_orders *before) {
    uint8_t paths[FIST_ORDER_PATH_BLOCK_BYTES] = {0};
    for (size_t index = 0; index < FIST_ORDER_PATH_BLOCK_BYTES; ++index) {
        paths[index] = scenario->chunks[FIST_SCENARIO_PATHS].data[index];
    }
    fist_scenario changed = *scenario;
    changed.chunks[FIST_SCENARIO_PATHS].data = paths;
    for (size_t platoon = 0; platoon < FIST_UNIT_PLATOON_COUNT; ++platoon) {
        const size_t offset = platoon * FIST_ORDER_PATH_BYTES;
        for (unsigned count = 0; count <= UINT8_MAX; ++count) {
            paths[offset] = (uint8_t)count;
            if (count > FIST_ORDER_WAYPOINTS) {
                if (orders_failure(&changed, before) == 0) {
                    return -1;
                }
            } else {
                fist_mission_orders expected = *before;
                expected.routes[platoon].count = (uint8_t)count;
                fist_mission_orders out = {0};
                if (fist_mission_orders_decode(&changed, &out) != 0 ||
                    memcmp(&out, &expected, sizeof(out)) != 0) {
                    return -1;
                }
            }
        }
        paths[offset] = scenario->chunks[FIST_SCENARIO_PATHS].data[offset];
    }
    return 0;
}

static int check_orders(const fist_scenario *scenario, const fist_mission_orders *before) {
    if (orders_failure(NULL, before) == 0 || fist_mission_orders_decode(scenario, NULL) != -1) {
        return -1;
    }
    const fist_scenario_chunk chunks[] = {FIST_SCENARIO_PATHS, FIST_SCENARIO_PLAYER_INFO};
    for (size_t index = 0; index < sizeof(chunks) / sizeof(chunks[0]); ++index) {
        fist_scenario changed = *scenario;
        const fist_scenario_chunk chunk = chunks[index];
        for (size_t length = 0; length < scenario->chunks[chunk].size; ++length) {
            changed.chunks[chunk].size = length;
            if (orders_failure(&changed, before) == 0) {
                return -1;
            }
        }
        changed.chunks[chunk].size = scenario->chunks[chunk].size + 1;
        if (orders_failure(&changed, before) == 0) {
            return -1;
        }
        changed.chunks[chunk].size = scenario->chunks[chunk].size;
        changed.chunks[chunk].data = NULL;
        if (orders_failure(&changed, before) == 0) {
            return -1;
        }
    }
    return check_order_counts(scenario, before);
}

static int observe_orders(uint8_t *data, size_t size, const fist_scenario *scenario, int checks) {
    fist_mission_orders orders = {0};
    orders.routes[0].header[0] = UINT8_MAX;
    orders.routes[FIST_UNIT_PLATOON_COUNT - 1].points[FIST_ORDER_WAYPOINTS - 1].y = INT32_MIN;
    orders.descriptors[FIST_UNIT_PLATOON_COUNT - 1].words[FIST_ORDER_DESCRIPTOR_WORDS - 1] =
        UINT16_MAX;
    const fist_mission_orders before = orders;
    if (fist_mission_orders_decode(scenario, &orders) != 0) {
        if (memcmp(&orders, &before, sizeof(orders)) != 0) {
            free(data);
            return 2;
        }
        free(data);
        return EXIT_FAILURE;
    }
    if (check_orders(scenario, &orders) != 0) {
        free(data);
        return 2;
    }
    /* Observation happens only after all borrowed source storage is freed. */
    for (size_t index = 0; index < size; ++index) {
        data[index] = UINT8_MAX;
    }
    free(data);
    if (checks == 0) {
        fist_probe_write_orders(&orders);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    const int prefixes = argc == 3 && strcmp(argv[1], "--prefixes") == 0;
    const int orders_mode = argc == 3 && strcmp(argv[1], "--orders") == 0;
    const int orders_checks = argc == 3 && strcmp(argv[1], "--orders-checks") == 0;
    if (argc != 2 && prefixes == 0 && orders_mode == 0 && orders_checks == 0) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[argc - 1], "rb");
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
    if (prefixes != 0) {
        const int result = check_prefixes(data, size);
        free(data);
        return result;
    }
    if (orders_mode != 0 || orders_checks != 0) {
        return observe_orders(data, size, &scenario, orders_checks);
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
