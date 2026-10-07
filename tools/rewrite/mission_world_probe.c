#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "mission_probe_io.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/tree.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 15,
    STREAM = 8,
    LINK = 9,
    RELOAD = 10,
    COUNT = 11,
    COMMAND = 4,
    TREE = 21,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    TARGET = 26,
    ARTILLERY = 27,
    SMOKE = 17,
    WRECK = 23,
    MARKER = 123
};

static void capture(const void *object, uint8_t *out, size_t size) {
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

static uint8_t *read_file(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    uint8_t *data = fist_probe_read_file(file, size);
    if (fclose(file) != 0) {
        free(data);
        return NULL;
    }
    return data;
}

static int invalid(const fist_units *units, const fist_random *random, fist_mission_world *world) {
    uint8_t before[sizeof(*world)];
    capture(world, before, sizeof(before));
    fist_random bad = *random;
    bad.next_stream = FIST_RANDOM_STREAMS;
    return fist_mission_world_initialize(NULL, random, 0, world) == -1 &&
                   fist_mission_world_initialize(units, NULL, 0, world) == -1 &&
                   fist_mission_world_initialize(units, random, 0, NULL) == -1 &&
                   fist_mission_world_initialize(units, &bad, 0, world) == -1 &&
                   fist_mission_world_object(NULL, 0) == NULL &&
                   fist_mission_world_object(world, FIST_UNIT_REGISTRY_COUNT) == NULL &&
                   unchanged(world, before, sizeof(before))
               ? 0
               : -1;
}

static int tree_commands(fist_mission_world *world, const uint8_t *input, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *request = input + (index * COMMAND);
        const uint16_t slot = fist_read_u16le(request);
        int status = -1;
        if (slot < FIST_UNIT_REGISTRY_COUNT && world->pool.slots[slot].used != 0 &&
            world->pool.slots[slot].type == TREE) {
            fist_tree *tree = &world->objects[slot].tree;
            uint8_t before[sizeof(*world)];
            capture(world, before, sizeof(before));
            const fist_tree_update update = {request[2], request[3]};
            if (fist_tree_advance(NULL, tree, update) != -1 ||
                fist_tree_advance(&world->pool, NULL, update) != -1 ||
                !unchanged(world, before, sizeof(before))) {
                return -1;
            }
            const uint8_t old_variant = tree->variant;
            status = fist_tree_advance(&world->pool, tree, update);
            if (status != 0 && !unchanged(world, before, sizeof(before))) {
                return -1;
            }
            if (status == 0) {
                const uint8_t new_variant = tree->variant;
                tree->variant = old_variant;
                const int preserved = unchanged(world, before, sizeof(before));
                tree->variant = new_variant;
                if (!preserved) {
                    return -1;
                }
            }
            printf("update %u %d\n", (unsigned)slot, status);
            fist_probe_write_tree(tree);
        } else {
            printf("update %u %d\n", (unsigned)slot, status);
        }
    }
    return fist_probe_write_mission_world(world);
}

static int reset_checked(fist_mission_world *world) {
    fist_mission_world_reset(world);
    fist_mission_world_reset(world);
    if (!fist_object_pool_is_valid(&world->pool)) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (fist_mission_world_object(world, (uint16_t)slot) != NULL) {
            return -1;
        }
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        if (world->combat.roster[index] != FIST_POOL_NO_SLOT) {
            return -1;
        }
    }
    fist_mission_world_reset(NULL);
    return 0;
}

static int run(fist_units *units, const uint8_t *request, size_t request_size) {
    if (request_size < HEADER || request[STREAM] >= FIST_RANDOM_STREAMS || request[RELOAD] > 1 ||
        fist_read_u32le(request + COUNT) != (request_size - HEADER) / COMMAND ||
        (request_size - HEADER) % COMMAND != 0) {
        return -1;
    }
    const size_t count = fist_read_u32le(request + COUNT);
    fist_random random = {.next_stream = request[STREAM]};
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        random.words[index] = fist_read_u16le(request + (index * sizeof(uint16_t)));
    }
    fist_mission_world *world = calloc(1, sizeof(*world));
    if (world == NULL) {
        return -1;
    }
    fist_mission_world_reset(world);
    world->random.words[0] = MARKER;
    uint8_t before[sizeof(*world)];
    capture(world, before, sizeof(before));
    if (invalid(units, &random, world) != 0) {
        free(world);
        return -1;
    }
    const int status = fist_mission_world_initialize(units, &random, request[LINK], world);
    if (status != 0 && !unchanged(world, before, sizeof(before))) {
        free(world);
        return -1;
    }
    int result = 0;
    if (status == 0 && invalid(units, &random, world) != 0) {
        free(world);
        return -1;
    }
    if (status == 0 && request[RELOAD] != 0) {
        printf("status 0\n");
        result = fist_probe_write_mission_world(world);
        if (result != 0 ||
            fist_mission_world_initialize(units, &world->random, request[LINK], world) != 0) {
            free(world);
            return -1;
        }
        printf("reload 0\n");
    }
    /* Runtime payloads must survive release of every immutable source view. */
    for (size_t index = 0; index < units->count; ++index) {
        units->definitions[index] = (fist_unit_definition){0};
    }
    fist_units_destroy(units);
    if (status != 0 || request[RELOAD] == 0) {
        printf("status %d\n", status);
    }
    result = status == 0 ? fist_probe_write_mission_world(world) : 0;
    if (result == 0 && status == 0) {
        result = tree_commands(world, request + HEADER, count);
    }
    if (reset_checked(world) != 0) {
        result = -1;
    }
    free(world);
    return result;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_file(argv[2], &size);
    fist_scenario scenario = {0};
    fist_units units = {0};
    int status = data == NULL ? -1 : fist_scenario_decode(data, size, &scenario);
    if (status == 0) {
        status = fist_units_decode(&scenario, &units);
    }
    free(data);
    if (status != 0) {
        return EXIT_FAILURE;
    }
    size_t request_size = 0;
    uint8_t *request = read_file(argv[1], &request_size);
    status = request == NULL ? -1 : run(&units, request, request_size);
    free(request);
    fist_units_destroy(&units);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
