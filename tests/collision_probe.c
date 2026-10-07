#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/collision.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    FILE_HEADER = 4,
    CASE_HEADER = 15,
    RANDOM_CURSOR = 8,
    IMPORT_COUNT = 9,
    QUERY_COUNT = 11,
    RESERVED = 13,
    BODY_BYTES = 24,
    BODY_INDEX = 2,
    BODY_VALUE = 4,
    BODY_X = 6,
    BODY_Y = 10,
    BODY_ALTITUDE = 14,
    BODY_HEADING = 18,
    BODY_SCALE = 20,
    BODY_FLAGS = 22,
    BODY_MODE = 23,
    MARKER = 123,
    PROJECTILE_TYPE = 8
};

typedef struct {
    fist_object_pool pool;
    fist_object_pose poses[FIST_UNIT_REGISTRY_COUNT];
    fist_collision_body bodies[FIST_UNIT_REGISTRY_COUNT];
    uint16_t imports[FIST_UNIT_REGISTRY_COUNT];
    uint16_t *queries;
    size_t query_count;
    fist_random random;
} collision_case;

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

static int rejected(const fist_collision_world *world, uint16_t slot, fist_random *random) {
    const fist_random saved = *random;
    const fist_collision_hit marker = {MARKER, MARKER, MARKER, MARKER};
    fist_collision_hit output = marker;
    return fist_collision_find(world, slot, random, &output) == -1 &&
           same_bytes(random, sizeof(*random), &saved) &&
           same_bytes(&output, sizeof(output), &marker);
}

static int invalid_inputs(void) {
    collision_case *value = calloc(1, sizeof(*value));
    if (value == NULL) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    fist_pool_allocation allocation = {0};
    int valid = fist_object_pool_import(&value->pool, (fist_pool_import){PROJECTILE_TYPE, 0, 1},
                                        &allocation) == 0;
    const uint16_t slot = allocation.slot;
    value->bodies[slot].pose = &value->poses[slot];
    fist_collision_world world = {&value->pool, value->bodies, FIST_UNIT_REGISTRY_COUNT};
    fist_collision_hit output = {0};
    valid = valid && rejected(NULL, slot, &value->random) &&
            fist_collision_find(&world, slot, NULL, &output) == -1 &&
            fist_collision_find(&world, slot, &value->random, NULL) == -1 &&
            rejected(&world, FIST_POOL_NO_SLOT, &value->random) &&
            rejected(&world, FIST_UNIT_REGISTRY_COUNT, &value->random) &&
            rejected(&world, 1, &value->random);
    world.body_count = FIST_UNIT_REGISTRY_COUNT - 1;
    valid = valid && rejected(&world, slot, &value->random);
    world.body_count = FIST_UNIT_REGISTRY_COUNT;
    value->bodies[slot].pose = NULL;
    valid = valid && rejected(&world, slot, &value->random);
    value->bodies[slot].pose = &value->poses[slot];
    value->random.next_stream = FIST_RANDOM_STREAMS;
    valid = valid && rejected(&world, slot, &value->random);
    value->random.next_stream = 0;
    value->pool.short_count = 0;
    valid = valid && rejected(&world, slot, &value->random);
    value->pool.short_count = 1;
    value->pool.registry[1] = value->pool.registry[0];
    valid = valid && rejected(&world, slot, &value->random);
    free(value);
    return valid ? 0 : -1;
}

static int import_body(collision_case *value, const uint8_t *input, size_t ordinal) {
    fist_pool_allocation allocation = {0};
    const fist_pool_import request = {fist_read_u16le(input), fist_read_u16le(input + BODY_INDEX),
                                      fist_read_u16le(input + BODY_VALUE)};
    if (fist_object_pool_import(&value->pool, request, &allocation) != 0) {
        return -1;
    }
    const size_t slot = allocation.slot;
    value->poses[slot] = (fist_object_pose){
        fist_read_i32le(input + BODY_X), fist_read_i32le(input + BODY_Y),
        fist_read_i32le(input + BODY_ALTITUDE), fist_read_u16le(input + BODY_HEADING)};
    value->bodies[slot] =
        (fist_collision_body){&value->poses[slot], fist_read_u16le(input + BODY_SCALE),
                              input[BODY_FLAGS], input[BODY_MODE]};
    value->imports[ordinal] = allocation.slot;
    return 0;
}

static int prepare(collision_case *value, const uint8_t *input, size_t available,
                   size_t *consumed) {
    if (available < CASE_HEADER) {
        return -1;
    }
    const size_t imports = fist_read_u16le(input + IMPORT_COUNT);
    const size_t queries = fist_read_u16le(input + QUERY_COUNT);
    const size_t size = CASE_HEADER + (imports * BODY_BYTES) + (queries * sizeof(uint16_t));
    if (imports > FIST_UNIT_REGISTRY_COUNT || queries == 0 || size > available ||
        input[RANDOM_CURSOR] >= FIST_RANDOM_STREAMS || fist_read_u16le(input + RESERVED) != 0) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        value->random.words[index] = fist_read_u16le(input + (index * sizeof(uint16_t)));
    }
    value->random.next_stream = input[RANDOM_CURSOR];
    for (size_t index = 0; index < imports; ++index) {
        if (import_body(value, input + CASE_HEADER + (index * BODY_BYTES), index) != 0) {
            return -1;
        }
    }
    value->queries = calloc(queries, sizeof(*value->queries));
    if (value->queries == NULL) {
        return -1;
    }
    value->query_count = queries;
    const fist_collision_world world = {&value->pool, value->bodies, FIST_UNIT_REGISTRY_COUNT};
    for (size_t index = 0; index < queries; ++index) {
        const size_t ordinal = fist_read_u16le(input + CASE_HEADER + (imports * BODY_BYTES) +
                                               (index * sizeof(uint16_t)));
        if (ordinal >= imports) {
            return -1;
        }
        value->queries[index] = value->imports[ordinal];
        fist_random random = value->random;
        fist_collision_hit hit = {0};
        if (fist_collision_find(&world, value->queries[index], &random, &hit) != 0) {
            return -1;
        }
    }
    *consumed = size;
    return 0;
}

static int observe_case(collision_case *value) {
    const fist_collision_world world = {&value->pool, value->bodies, FIST_UNIT_REGISTRY_COUNT};
    const fist_object_pool saved_pool = value->pool;
    fist_object_pose saved_poses[FIST_UNIT_REGISTRY_COUNT] = {0};
    fist_collision_body saved_bodies[FIST_UNIT_REGISTRY_COUNT] = {0};
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        saved_poses[index] = value->poses[index];
        saved_bodies[index] = value->bodies[index];
    }
    for (size_t index = 0; index < value->query_count; ++index) {
        fist_collision_hit hit = {0};
        if (fist_collision_find(&world, value->queries[index], &value->random, &hit) != 0) {
            return -1;
        }
        printf("hit %u %u %u %u\nrandom %u", (unsigned)hit.slot, (unsigned)hit.registry_index,
               (unsigned)hit.value, (unsigned)hit.aspect, (unsigned)value->random.next_stream);
        for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
            printf(" %u", (unsigned)value->random.words[stream]);
        }
        printf("\n");
    }
    return same_bytes(&value->pool, sizeof(value->pool), &saved_pool) &&
                   same_bytes(value->poses, sizeof(saved_poses), saved_poses) &&
                   same_bytes(value->bodies, sizeof(saved_bodies), saved_bodies)
               ? 0
               : -1;
}

static int run_cases(uint8_t *input, size_t size) {
    const size_t count = fist_read_u32le(input);
    if (count > (size - FILE_HEADER) / CASE_HEADER) {
        free(input);
        return -1;
    }
    collision_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(input);
        return -1;
    }
    size_t offset = FILE_HEADER;
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        size_t consumed = 0;
        if (prepare(&cases[index], input + offset, size - offset, &consumed) != 0) {
            status = -1;
            break;
        }
        offset += consumed;
    }
    if (offset != size) {
        status = -1;
    }
    free(input);
    for (size_t index = 0; index < count && status == 0; ++index) {
        status = observe_case(&cases[index]);
    }
    for (size_t index = 0; index < count; ++index) {
        free(cases[index].queries);
    }
    free(cases);
    return status;
}

int main(int argc, char **argv) {
    if (argc != 2 || invalid_inputs() != 0) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *input = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (input == NULL || size < FILE_HEADER || closed != 0) {
        free(input);
        return EXIT_FAILURE;
    }
    return run_cases(input, size) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
