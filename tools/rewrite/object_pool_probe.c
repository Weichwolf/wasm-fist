#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/object_pool.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { ARGUMENT_COUNT = 2, HEADER_BYTES = 4, COMMAND_BYTES = 8, CONTRACT_FAILURE = 2 };

static int same_pool(const fist_object_pool *left, const fist_object_pool *right) {
    if (left->short_count != right->short_count || left->extended_count != right->extended_count) {
        return 0;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (left->slots[index].type != right->slots[index].type ||
            left->slots[index].used != right->slots[index].used ||
            left->registry[index].slot != right->registry[index].slot ||
            left->registry[index].value != right->registry[index].value) {
            return 0;
        }
    }
    return 1;
}

static int same_allocation(const fist_pool_allocation *left, const fist_pool_allocation *right) {
    return left->type == right->type && left->slot == right->slot &&
           left->registry_index == right->registry_index && left->value == right->value;
}

static int invalid_state(fist_object_pool pool) {
    const fist_object_pool saved = pool;
    const fist_pool_allocation marker = {FIST_UNIT_TYPE_COUNT, FIST_POOL_NO_SLOT,
                                         FIST_UNIT_REGISTRY_COUNT, UINT16_MAX};
    fist_pool_allocation output = marker;
    const fist_pool_request request = {8, 0};
    const fist_pool_import binding = {8, 0, 1};
    return fist_object_pool_allocate(&pool, request, &output) == -1 &&
           fist_object_pool_import(&pool, binding, &output) == -1 &&
           fist_object_pool_release(&pool, 0, &output) == -1 && same_pool(&pool, &saved) != 0 &&
           same_allocation(&output, &marker) != 0;
}

static int invalid_inputs(void) {
    fist_object_pool empty = {0};
    fist_object_pool_reset(&empty);
    fist_object_pool_reset(NULL);
    fist_pool_allocation output = {0};
    const fist_pool_request request = {8, 0};
    const fist_pool_import binding = {8, 0, 1};
    if (fist_object_pool_allocate(NULL, request, &output) != -1 ||
        fist_object_pool_import(NULL, binding, &output) != -1 ||
        fist_object_pool_release(NULL, 0, &output) != -1 ||
        fist_object_pool_allocate(&empty, request, NULL) != -1 ||
        fist_object_pool_import(&empty, binding, NULL) != -1 ||
        fist_object_pool_release(&empty, 0, NULL) != -1) {
        return 0;
    }
    fist_object_pool bad = empty;
    bad.short_count = 1;
    if (invalid_state(bad) == 0) {
        return 0;
    }
    bad = empty;
    bad.registry[0].slot = FIST_UNIT_REGISTRY_COUNT;
    if (invalid_state(bad) == 0) {
        return 0;
    }
    bad = empty;
    bad.slots[0] = (fist_pool_slot){0, 1};
    bad.short_count = 1;
    if (invalid_state(bad) == 0) {
        return 0;
    }
    bad = empty;
    if (fist_object_pool_allocate(&bad, request, &output) != 0) {
        return 0;
    }
    bad.registry[1] = bad.registry[0];
    return invalid_state(bad);
}

static void write_pool(const fist_object_pool *pool) {
    printf("counts %u %u\n", (unsigned)pool->short_count, (unsigned)pool->extended_count);
    printf("slots");
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        printf(" %u:%u", (unsigned)pool->slots[index].used, (unsigned)pool->slots[index].type);
    }
    printf("\nregistry");
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        printf(" %u:%u", (unsigned)pool->registry[index].slot,
               (unsigned)pool->registry[index].value);
    }
    printf("\n");
}

static int command(fist_object_pool *pool, const uint8_t *input, fist_pool_allocation *output) {
    enum {
        RESET = 0,
        ALLOCATE = 1,
        IMPORT = 2,
        RELEASE = 3,
        TYPE_OFFSET = 2,
        INDEX_OFFSET = 4,
        VALUE_OFFSET = 6
    };
    const uint16_t type = fist_read_u16le(input + TYPE_OFFSET);
    const uint16_t index = fist_read_u16le(input + INDEX_OFFSET);
    const uint16_t value = fist_read_u16le(input + VALUE_OFFSET);
    switch (input[0]) {
    case RESET:
        fist_object_pool_reset(pool);
        return 0;
    case ALLOCATE:
        return fist_object_pool_allocate(pool, (fist_pool_request){type, input[1]}, output);
    case IMPORT:
        return fist_object_pool_import(pool, (fist_pool_import){type, index, value}, output);
    case RELEASE:
        return fist_object_pool_release(pool, index, output);
    default:
        return -1;
    }
}

int main(int argc, char **argv) {
    if (argc != ARGUMENT_COUNT) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *input = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (input == NULL || closed != 0 || size < HEADER_BYTES ||
        (size - HEADER_BYTES) % COMMAND_BYTES != 0 ||
        fist_read_u32le(input) != (size - HEADER_BYTES) / COMMAND_BYTES) {
        free(input);
        return EXIT_FAILURE;
    }
    if (invalid_inputs() == 0) {
        free(input);
        return CONTRACT_FAILURE;
    }
    fist_object_pool pool = {0};
    fist_object_pool_reset(&pool);
    fist_pool_allocation output = {FIST_UNIT_TYPE_COUNT, FIST_POOL_NO_SLOT,
                                   FIST_UNIT_REGISTRY_COUNT, UINT16_MAX};
    write_pool(&pool);
    for (size_t offset = HEADER_BYTES; offset < size; offset += COMMAND_BYTES) {
        const int result = command(&pool, input + offset, &output);
        printf("result %d %u %u %u %u\n", result, (unsigned)output.type, (unsigned)output.slot,
               (unsigned)output.registry_index, (unsigned)output.value);
        write_pool(&pool);
    }
    free(input);
    return ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
