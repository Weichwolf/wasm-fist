#include "assets/bytes.h"
#include "assets/orders.h"
#include "probe_io.h"
#include "sim/geometry.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER = 4,
    COORDINATE_BYTES = 4,
    POINT_BYTES = COORDINATE_BYTES * 2,
    MODE = POINT_BYTES * 2,
    CASE_BYTES = MODE + 1
};

typedef struct {
    fist_order_waypoint source;
    fist_order_waypoint target;
    uint8_t coarse;
} geometry_case;

static fist_order_waypoint point(const uint8_t *raw) {
    return (fist_order_waypoint){fist_read_i32le(raw), fist_read_i32le(raw + COORDINATE_BYTES)};
}

static geometry_case *decode(const uint8_t *data, size_t size, size_t *count) {
    if (size < HEADER || fist_read_u32le(data) != (size - HEADER) / CASE_BYTES ||
        (size - HEADER) % CASE_BYTES != 0) {
        return NULL;
    }
    *count = fist_read_u32le(data);
    geometry_case *cases = calloc(*count == 0 ? 1 : *count, sizeof(*cases));
    if (cases == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < *count; ++index) {
        const uint8_t *raw = data + HEADER + (index * CASE_BYTES);
        cases[index] = (geometry_case){point(raw), point(raw + POINT_BYTES), raw[MODE]};
    }
    return cases;
}

int main(int argc, char **argv) {
    const bool proximity = argc == 3 && strcmp(argv[1], "proximity") == 0;
    if (argc != 2 && !proximity) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[proximity ? 2 : 1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    size_t count = 0;
    geometry_case *cases = data == NULL || closed != 0 ? NULL : decode(data, size, &count);
    if (data != NULL) {
        for (size_t index = 0; index < size; ++index) {
            data[index] = UINT8_MAX;
        }
    }
    free(data);
    if (cases == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t index = 0; index < count; ++index) {
        const geometry_case input = cases[index];
        if (proximity) {
            printf("proximity %u\n", (unsigned)fist_planar_proximity(input.source, input.target));
            continue;
        }
        const fist_planar_measurement measured =
            fist_planar_measure(input.source, input.target, input.coarse != 0);
        printf("geometry %u %u\n", (unsigned)measured.heading, (unsigned)measured.distance);
    }
    free(cases);
    return ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
