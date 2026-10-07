#include "probe_io.h"

#include "assets/orders.h"
#include "assets/units.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Read through EOF rather than guessing an asset's size or truncating a prefix. */
uint8_t *fist_probe_read_file(FILE *file, size_t *size) {
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

void fist_probe_write_orders(const fist_mission_orders *orders) {
    printf("orders %u\n", (unsigned)(orders != NULL));
    if (orders == NULL) {
        return;
    }
    for (size_t platoon = 0; platoon < FIST_UNIT_PLATOON_COUNT; ++platoon) {
        const fist_order_route *route = &orders->routes[platoon];
        printf("route %zu %u", platoon, (unsigned)route->count);
        for (size_t index = 0; index < FIST_ORDER_HEADER_BYTES; ++index) {
            printf(" %02x", (unsigned)route->header[index]);
        }
        for (size_t index = 0; index < FIST_ORDER_WAYPOINTS; ++index) {
            printf(" %" PRId32 " %" PRId32, route->points[index].x, route->points[index].y);
        }
        printf("\n");
    }
    for (size_t platoon = 0; platoon < FIST_UNIT_PLATOON_COUNT; ++platoon) {
        printf("descriptor %zu", platoon);
        for (size_t index = 0; index < FIST_ORDER_DESCRIPTOR_WORDS; ++index) {
            printf(" %u", (unsigned)orders->descriptors[platoon].words[index]);
        }
        printf("\n");
    }
}

void fist_probe_capture(const void *object, size_t size, void *out) {
    const unsigned char *bytes = object;
    unsigned char *snapshot = out;
    for (size_t index = 0; index < size; ++index) {
        snapshot[index] = bytes[index];
    }
}

int fist_probe_unchanged(const void *object, size_t size, const void *before) {
    const unsigned char *bytes = object;
    const unsigned char *snapshot = before;
    for (size_t index = 0; index < size; ++index) {
        if (bytes[index] != snapshot[index]) {
            return 0;
        }
    }
    return 1;
}
