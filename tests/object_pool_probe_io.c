#include "object_pool_probe_io.h"

#include "assets/units.h"
#include "sim/object_pool.h"

#include <stddef.h>
#include <stdio.h>

void fist_probe_write_object_pool(const fist_object_pool *pool) {
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
