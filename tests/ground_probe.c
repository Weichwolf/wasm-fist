#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/ground.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_SIZE = 12,
    QUERY_BYTES = 10,
    VEHICLE_BYTES = FIST_UNIT_EXTENDED_SIZE + 4,
    QUERIES_OFFSET = 4,
    VEHICLES_OFFSET = 8,
    Y_OFFSET = 4,
    HEADING_OFFSET = 8,
    RAW_OFFSET = 4,
    GENERATION_OFFSET = 2,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    MARKER = 77
};

typedef struct {
    fist_klc_image height;
    size_t query_count;
    size_t vehicle_count;
    fist_ground_pose *poses;
    fist_ground_contact *contacts;
    fist_vehicle_state *vehicles;
} ground_request;

static void release(ground_request *request) {
    fist_klc_destroy(&request->height);
    free(request->poses);
    free(request->contacts);
    free(request->vehicles);
}

static int load_vehicle(const uint8_t *record, fist_vehicle_state *out) {
    const uint8_t *raw = record + RAW_OFFSET;
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .registry_index = fist_read_u16le(record),
                                             .generation =
                                                 fist_read_u16le(record + GENERATION_OFFSET),
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    fist_random random = {0};
    return fist_vehicle_initialize(&definition, &random, 0, out);
}

static int load_request(const uint8_t *data, size_t size, ground_request *out) {
    if (size < HEADER_SIZE) {
        return -1;
    }
    const uint32_t side = fist_read_u32le(data);
    if (side == 0 || (size_t)side > SIZE_MAX / side || (size_t)side * side > size - HEADER_SIZE) {
        return -1;
    }
    const size_t plane_size = (size_t)side * side;
    const size_t query_count = fist_read_u32le(data + QUERIES_OFFSET);
    const size_t vehicle_count = fist_read_u32le(data + VEHICLES_OFFSET);
    size_t offset = HEADER_SIZE + plane_size;
    if (query_count > (size - offset) / QUERY_BYTES) {
        return -1;
    }
    offset += query_count * QUERY_BYTES;
    if (vehicle_count > (size - offset) / VEHICLE_BYTES ||
        vehicle_count * VEHICLE_BYTES != size - offset ||
        (query_count == 0 && vehicle_count == 0)) {
        return -1;
    }
    out->height = (fist_klc_image){.width = side, .height = side};
    out->height.pixels = malloc(plane_size);
    out->poses = calloc(query_count + 1, sizeof(*out->poses));
    out->contacts = calloc(query_count + 1, sizeof(*out->contacts));
    out->vehicles = calloc(vehicle_count + 1, sizeof(*out->vehicles));
    if (out->height.pixels == NULL || out->poses == NULL || out->contacts == NULL ||
        out->vehicles == NULL) {
        return -1;
    }
    for (size_t index = 0; index < plane_size; ++index) {
        out->height.pixels[index] = data[HEADER_SIZE + index];
    }
    for (size_t index = 0; index < query_count; ++index) {
        const uint8_t *record = data + HEADER_SIZE + plane_size + (index * QUERY_BYTES);
        out->poses[index] = (fist_ground_pose){.map_x = fist_read_i32le(record),
                                               .map_y = fist_read_i32le(record + Y_OFFSET),
                                               .heading = fist_read_u16le(record + HEADING_OFFSET)};
    }
    for (size_t index = 0; index < vehicle_count; ++index) {
        if (load_vehicle(data + offset + (index * VEHICLE_BYTES), &out->vehicles[index]) != 0) {
            return -1;
        }
    }
    out->query_count = query_count;
    out->vehicle_count = vehicle_count;
    return 0;
}

static int run(ground_request *request) {
    for (size_t index = 0; index < request->query_count; ++index) {
        if (fist_ground_sample(&request->height, &request->poses[index],
                               &request->contacts[index]) != 0) {
            return -1;
        }
    }
    for (size_t index = 0; index < request->vehicle_count; ++index) {
        if (fist_vehicle_ground_update(&request->vehicles[index], &request->height) != 0) {
            return -1;
        }
    }
    fist_klc_destroy(&request->height);
    for (size_t index = 0; index < request->query_count; ++index) {
        const fist_ground_contact *contact = &request->contacts[index];
        printf("sample %u %d %d\n", (unsigned)contact->height, contact->roll, contact->pitch);
    }
    for (size_t index = 0; index < request->vehicle_count; ++index) {
        fist_probe_write_vehicle_state(&request->vehicles[index]);
    }
    return ferror(stdout) != 0 ? -1 : 0;
}

static int contact_preserved(const fist_ground_contact *contact) {
    return contact->height == MARKER && contact->roll == MARKER && contact->pitch == MARKER;
}

static int sample_contracts(void) {
    uint8_t pixel = MARKER;
    fist_klc_image height = {.width = 1, .height = 1, .pixels = &pixel};
    const fist_ground_pose pose = {0};
    fist_ground_contact contact = {MARKER, MARKER, MARKER};
    if (fist_ground_sample(NULL, &pose, &contact) != -1 ||
        fist_ground_sample(&height, NULL, &contact) != -1 ||
        fist_ground_sample(&height, &pose, NULL) != -1 || contact_preserved(&contact) == 0) {
        return -1;
    }
    const uint32_t shapes[][2] = {{0, 0}, {1, 2}, {3, 3}, {UINT32_C(131072), UINT32_C(131072)}};
    for (size_t index = 0; index < sizeof(shapes) / sizeof(shapes[0]); ++index) {
        height.width = shapes[index][0];
        height.height = shapes[index][1];
        if (fist_ground_sample(&height, &pose, &contact) != -1 ||
            contact_preserved(&contact) == 0) {
            return -1;
        }
    }
    height = (fist_klc_image){.width = 1, .height = 1};
    return fist_ground_sample(&height, &pose, &contact) == -1 && contact_preserved(&contact) != 0
               ? 0
               : -1;
}

static int same_bytes(const unsigned char *first, const unsigned char *second, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        if (first[index] != second[index]) {
            return 0;
        }
    }
    return 1;
}

static int vehicle_contracts(void) {
    fist_vehicle_state *vehicle = calloc(1, sizeof(*vehicle));
    if (vehicle == NULL) {
        return -1;
    }
    uint8_t pixel = MARKER;
    const fist_klc_image height = {.width = 1, .height = 1, .pixels = &pixel};
    unsigned char backup[sizeof(*vehicle)] = {0};
    const unsigned char *bytes = (const unsigned char *)vehicle;
    int result = fist_vehicle_ground_update(NULL, &height) == -1 ? 0 : -1;
    for (unsigned type = 0; type <= FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
            vehicle->type = (uint16_t)type;
            vehicle->component_size = size;
            for (size_t index = 0; index < sizeof(*vehicle); ++index) {
                backup[index] = bytes[index];
            }
            if (fist_vehicle_ground_update(vehicle, NULL) != -1 ||
                same_bytes(backup, bytes, sizeof(*vehicle)) == 0) {
                result = -1;
            }
            if (type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
                size != fist_vehicle_component_size((uint16_t)type)) {
                if (fist_vehicle_ground_update(vehicle, &height) != -1 ||
                    same_bytes(backup, bytes, sizeof(*vehicle)) == 0) {
                    result = -1;
                }
            }
        }
    }
    free(vehicle);
    return result;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "contracts") == 0) {
        return sample_contracts() == 0 && vehicle_contracts() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
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
    ground_request request = {0};
    int result = -1;
    if (data != NULL && closed == 0) {
        result = load_request(data, size, &request);
    }
    if (data != NULL) {
        for (size_t index = 0; index < size; ++index) {
            data[index] = 0;
        }
    }
    free(data); /* No source file/snapshot storage survives into contact queries. */
    if (result == 0) {
        result = run(&request);
    }
    release(&request);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
