#include "assets/bytes.h"
#include "assets/units.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/object_pool.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER_BYTES = 4,
    CASE_HEADER = 15,
    IMPORT_BYTES = 6,
    ORIGIN_ORDINAL = 2,
    STEPS = 4,
    COARSE = 6,
    FLAGS = 7,
    AMMO = 8,
    RELOAD = 10,
    RECOIL = 11,
    TRIGGER = 12,
    RELEASE_COUNT = 13,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    TARGET = 151,
    MARKER_INDEX = 9,
    MARKER_VALUE = 12
};

typedef struct {
    fist_object_pool pool;
    fist_vehicle_state vehicle;
    fist_launch_request request;
    uint16_t steps;
} launch_case;

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

static int rejected(fist_object_pool *pool, fist_vehicle_state *vehicle,
                    fist_launch_request request) {
    const fist_object_pool saved_pool = *pool;
    const fist_vehicle_state saved_vehicle = *vehicle;
    const fist_launch_result marker = {.outcome = UINT8_MAX, .sound_request = UINT8_MAX};
    fist_launch_result output = marker;
    return fist_m1_launch_untargeted(pool, vehicle, request, &output) == -1 &&
           same_bytes(pool, sizeof(*pool), &saved_pool) &&
           same_bytes(vehicle, sizeof(*vehicle), &saved_vehicle) &&
           same_bytes(&output, sizeof(output), &marker);
}

static int invalid_inputs(void) {
    fist_object_pool pool = {0};
    fist_object_pool_reset(&pool);
    fist_pool_allocation actor = {0};
    if (fist_object_pool_import(&pool, (fist_pool_import){0, MARKER_INDEX, MARKER_VALUE}, &actor) !=
        0) {
        return -1;
    }
    fist_vehicle_state vehicle = {.component_size = fist_vehicle_component_size(0)};
    fist_launch_result output = {0};
    const fist_launch_request request = {actor.slot, false};
    if (fist_m1_launch_untargeted(NULL, &vehicle, request, &output) != -1 ||
        fist_m1_launch_untargeted(&pool, NULL, request, &output) != -1 ||
        fist_m1_launch_untargeted(&pool, &vehicle, request, NULL) != -1) {
        return -1;
    }
    for (unsigned slot = 0; slot <= FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (slot != actor.slot &&
            !rejected(&pool, &vehicle, (fist_launch_request){(uint16_t)slot, false})) {
            return -1;
        }
    }
    for (unsigned type = 1; type <= FIST_UNIT_TYPE_COUNT; ++type) {
        vehicle.type = (uint16_t)type;
        if (!rejected(&pool, &vehicle, request)) {
            return -1;
        }
    }
    vehicle.type = 0;
    for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
        if (size == fist_vehicle_component_size(0)) {
            continue;
        }
        vehicle.component_size = size;
        if (!rejected(&pool, &vehicle, request)) {
            return -1;
        }
    }
    vehicle.component_size = fist_vehicle_component_size(0);
    pool.short_count = 1;
    if (!rejected(&pool, &vehicle, request)) {
        return -1;
    }
    pool.short_count = 0;
    pool.registry[0] = pool.registry[MARKER_INDEX];
    return rejected(&pool, &vehicle, request) ? 0 : -1;
}

static int prepare(const uint8_t *data, size_t available, launch_case *out, size_t *consumed) {
    if (available < CASE_HEADER + FIST_UNIT_EXTENDED_SIZE) {
        return -1;
    }
    const size_t count = fist_read_u16le(data);
    const size_t ordinal = fist_read_u16le(data + ORIGIN_ORDINAL);
    const size_t releases = fist_read_u16le(data + RELEASE_COUNT);
    const size_t size = CASE_HEADER + FIST_UNIT_EXTENDED_SIZE + (count * IMPORT_BYTES) +
                        (releases * sizeof(uint16_t));
    const uint8_t *raw = data + CASE_HEADER;
    if (count > FIST_UNIT_REGISTRY_COUNT || ordinal >= count || size > available ||
        data[COARSE] > 1 || releases > FIST_UNIT_REGISTRY_COUNT || fist_read_u16le(raw) != 0 ||
        fist_read_u16le(raw + TARGET) != 0 || fist_read_u16le(data + STEPS) == 0) {
        return -1;
    }
    fist_object_pool_reset(&out->pool);
    fist_pool_allocation origin = {0};
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *binding = raw + FIST_UNIT_EXTENDED_SIZE + (index * IMPORT_BYTES);
        fist_pool_allocation allocation = {0};
        const fist_pool_import request = {fist_read_u16le(binding), fist_read_u16le(binding + 2),
                                          fist_read_u16le(binding + 4)};
        if (fist_object_pool_import(&out->pool, request, &allocation) != 0) {
            return -1;
        }
        if (index == ordinal) {
            origin = allocation;
        }
    }
    for (size_t index = 0; index < releases; ++index) {
        const uint8_t *entry =
            raw + FIST_UNIT_EXTENDED_SIZE + (count * IMPORT_BYTES) + (index * sizeof(uint16_t));
        fist_pool_allocation allocation = {0};
        if (fist_object_pool_release(&out->pool, fist_read_u16le(entry), &allocation) != 0 ||
            allocation.slot == origin.slot) {
            return -1;
        }
    }
    if (origin.type != 0) {
        return -1;
    }
    const fist_unit_definition definition = {.type = 0,
                                             .registry_index = origin.registry_index,
                                             .generation = origin.value,
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    fist_random random = {0};
    if (fist_vehicle_initialize(&definition, &random, 0, &out->vehicle) != 0) {
        return -1;
    }
    out->vehicle.weapons.rounds[0] = fist_read_u16le(data + AMMO);
    out->vehicle.reload_countdown = data[RELOAD];
    out->vehicle.weapons.recoil = data[RECOIL];
    out->vehicle.weapons.trigger = data[TRIGGER];
    out->vehicle.object_flags = data[FLAGS];
    out->request = (fist_launch_request){origin.slot, data[COARSE] != 0};
    out->steps = fist_read_u16le(data + STEPS);
    *consumed = size;
    return 0;
}

static void write_payloads(const fist_launch_result *result) {
    printf("launch %u %u %u\n", (unsigned)result->outcome, (unsigned)result->has_muzzle,
           (unsigned)result->sound_request);
    if (result->outcome == FIST_LAUNCH_FIRED) {
        const fist_projectile *shot = &result->projectile;
        printf("projectile %u %u %u %u %d %d %d %u %d %d %d %d %u %u %u %d %u %u %u\n",
               (unsigned)shot->allocation.type, (unsigned)shot->allocation.slot,
               (unsigned)shot->allocation.registry_index, (unsigned)shot->allocation.value,
               shot->pose.x, shot->pose.y, shot->pose.altitude, (unsigned)shot->pose.heading,
               shot->velocity.x, shot->velocity.y, shot->velocity.z, shot->speed,
               (unsigned)shot->collision_grace, (unsigned)shot->origin_slot,
               (unsigned)shot->target_slot, shot->target_height_offset, (unsigned)shot->flags,
               (unsigned)shot->collision_profile, (unsigned)shot->launch_parameter);
    }
    if (result->has_muzzle) {
        const fist_muzzle_smoke *smoke = &result->muzzle;
        printf("muzzle %u %u %u %u %d %d %d %u %u %u %u %u\n", (unsigned)smoke->allocation.type,
               (unsigned)smoke->allocation.slot, (unsigned)smoke->allocation.registry_index,
               (unsigned)smoke->allocation.value, smoke->pose.x, smoke->pose.y,
               smoke->pose.altitude, (unsigned)smoke->pose.heading,
               (unsigned)smoke->projection_scale, (unsigned)smoke->animation_counter,
               (unsigned)smoke->animation_frame, (unsigned)smoke->flags);
    }
}

static int run_cases(uint8_t *data, size_t size) {
    const size_t count = fist_read_u32le(data);
    if (count > (size - HEADER_BYTES) / (CASE_HEADER + FIST_UNIT_EXTENDED_SIZE)) {
        free(data);
        return -1;
    }
    launch_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return -1;
    }
    size_t offset = HEADER_BYTES;
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        size_t consumed = 0;
        if (prepare(data + offset, size - offset, &cases[index], &consumed) != 0) {
            status = -1;
            break;
        }
        offset += consumed;
    }
    if (offset != size) {
        status = -1;
    }
    free(data); /* No borrowed source/request bytes survive simulation. */
    for (size_t index = 0; index < count && status == 0; ++index) {
        launch_case *value = &cases[index];
        for (unsigned step = 0; step < value->steps; ++step) {
            fist_launch_result result = {0};
            status =
                fist_m1_launch_untargeted(&value->pool, &value->vehicle, value->request, &result);
            if (status != 0) {
                break;
            }
            write_payloads(&result);
            fist_probe_write_vehicle_state(&value->vehicle);
            fist_probe_write_object_pool(&value->pool);
        }
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
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (data == NULL || size < HEADER_BYTES || closed != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    return run_cases(data, size) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
