#include "assets/bytes.h"
#include "assets/klc.h"
#include "probe_io.h"
#include "sim/ground.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { HEADER = 8, COUNT_OFFSET = 4, POINT_BYTES = 12, CASE_BYTES = POINT_BYTES * 2 };

typedef struct {
    fist_object_pose source;
    fist_object_pose target;
} visibility_case;

typedef struct {
    fist_klc_image height;
    visibility_case *cases;
    size_t count;
} visibility_request;

static fist_object_pose point(const uint8_t *raw) {
    enum { Y_OFFSET = 4, ALTITUDE_OFFSET = 8 };
    return (fist_object_pose){.x = fist_read_i32le(raw),
                              .y = fist_read_i32le(raw + Y_OFFSET),
                              .altitude = fist_read_i32le(raw + ALTITUDE_OFFSET)};
}

static int decode(const uint8_t *data, size_t size, visibility_request *out) {
    if (size < HEADER) {
        return -1;
    }
    const uint32_t side = fist_read_u32le(data);
    if (side == 0 || (size_t)side > SIZE_MAX / side || (size_t)side * side > size - HEADER) {
        return -1;
    }
    const size_t plane_size = (size_t)side * side;
    const size_t offset = HEADER + plane_size;
    const size_t count = fist_read_u32le(data + COUNT_OFFSET);
    if ((size - offset) % CASE_BYTES != 0 || count != (size - offset) / CASE_BYTES) {
        return -1;
    }
    uint8_t *pixels = malloc(plane_size);
    visibility_case *cases = calloc(count == 0 ? 1 : count, sizeof(*cases));
    if (pixels == NULL || cases == NULL) {
        free(pixels);
        free(cases);
        return -1;
    }
    for (size_t index = 0; index < plane_size; ++index) {
        pixels[index] = data[HEADER + index];
    }
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *raw = data + offset + (index * CASE_BYTES);
        cases[index] = (visibility_case){point(raw), point(raw + POINT_BYTES)};
    }
    *out = (visibility_request){.height = {.width = side, .height = side, .pixels = pixels},
                                .cases = cases,
                                .count = count};
    return 0;
}

static int contracts(void) {
    enum { TOO_MANY_INDEX_BITS_SIDE = 1 << 17 };
    uint8_t pixel = 0;
    const fist_klc_image valid = {.width = 1, .height = 1, .pixels = &pixel};
    const fist_object_pose pose = {.altitude = 1};
    fist_klc_image bad[] = {
        {0},
        {.width = 1, .height = 1},
        {.width = 0, .height = 1, .pixels = &pixel},
        {.width = 1, .height = 2, .pixels = &pixel},
        {.width = 3, .height = 3, .pixels = &pixel},
        {.width = UINT32_MAX, .height = UINT32_MAX, .pixels = &pixel},
        {.width = TOO_MANY_INDEX_BITS_SIDE, .height = TOO_MANY_INDEX_BITS_SIDE, .pixels = &pixel},
    };
    bool visible = true;
    if (fist_ground_visible(NULL, &pose, &pose, &visible) != -1 || !visible ||
        fist_ground_visible(&valid, NULL, &pose, &visible) != -1 || !visible ||
        fist_ground_visible(&valid, &pose, NULL, &visible) != -1 || !visible ||
        fist_ground_visible(&valid, &pose, &pose, NULL) != -1) {
        return EXIT_FAILURE;
    }
    for (size_t index = 0; index < sizeof(bad) / sizeof(bad[0]); ++index) {
        if (fist_ground_visible(&bad[index], &pose, &pose, &visible) != -1 || !visible) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "contracts") == 0) {
        return contracts();
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    visibility_request request = {0};
    const int decoded = data == NULL || closed != 0 ? -1 : decode(data, size, &request);
    if (data != NULL) {
        for (size_t index = 0; index < size; ++index) {
            data[index] = UINT8_MAX;
        }
    }
    free(data);
    if (decoded != 0) {
        return EXIT_FAILURE;
    }
    // Validate the plane even for an empty query batch before emitting output.
    const fist_object_pose origin = {0};
    bool visible = false;
    int status = fist_ground_visible(&request.height, &origin, &origin, &visible);
    for (size_t index = 0; index < request.count && status == 0; ++index) {
        const visibility_case input = request.cases[index];
        status = fist_ground_visible(&request.height, &input.source, &input.target, &visible);
        if (status == 0) {
            printf("visibility %d\n", (int)visible);
        }
    }
    fist_klc_destroy(&request.height);
    free(request.cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
