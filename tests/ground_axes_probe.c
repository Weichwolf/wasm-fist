#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/driver.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER_BYTES = 4,
    CASE_BYTES = FIST_UNIT_EXTENDED_SIZE + 4,
    STEPS_OFFSET = FIST_UNIT_EXTENDED_SIZE,
    STAGE_OFFSET = STEPS_OFFSET + 2,
    LINK_OFFSET = STAGE_OFFSET + 1,
    MAP_X_OFFSET = 4,
    MAP_Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HEADING_OFFSET = 16,
    RESTORE = 0,
    INITIALIZE = 1,
    PREPARE = 2,
    APPLY = 3,
    HIGH_SEED = 32768,
    WORD_SEED = 65535
};

typedef struct {
    fist_vehicle_state vehicle;
    fist_random random;
    uint16_t steps;
    uint8_t stage;
} axis_case;

static int step_case(axis_case *value, fist_drive_control_events *events) {
    static const size_t original_components[FIST_UNIT_GROUND_VEHICLE_COUNT] = {214, 200, 202, 210};
    fist_vehicle_state before;
    fist_probe_capture(&value->vehicle, sizeof(before), &before);
    if (fist_driver_apply_axes(&value->vehicle, events) != 0) {
        return -1;
    }
    fist_probe_capture(&value->vehicle.drive.requested_heading,
                       sizeof(before.drive.requested_heading), &before.drive.requested_heading);
    fist_probe_capture(&value->vehicle.drive.throttle, sizeof(before.drive.throttle),
                       &before.drive.throttle);
    fist_probe_capture(&value->vehicle.control_mode, sizeof(before.control_mode),
                       &before.control_mode);
    const size_t component =
        original_components[value->vehicle.type] - fist_probe_component_offset(value->vehicle.type);
    fist_probe_capture(&value->vehicle.components[component], sizeof(before.components[component]),
                       &before.components[component]);
    return fist_probe_unchanged(&value->vehicle, sizeof(before), &before) ? 0 : -1;
}

static int rejected(fist_vehicle_state *vehicle, fist_drive_control_events *events) {
    fist_vehicle_state before;
    fist_drive_control_events saved;
    if (vehicle != NULL) {
        fist_probe_capture(vehicle, sizeof(before), &before);
    }
    if (events != NULL) {
        fist_probe_capture(events, sizeof(saved), &saved);
    }
    return fist_driver_apply_axes(vehicle, events) == -1 &&
                   (vehicle == NULL || fist_probe_unchanged(vehicle, sizeof(before), &before)) &&
                   (events == NULL || fist_probe_unchanged(events, sizeof(saved), &saved))
               ? 0
               : -1;
}

static int invalid_inputs(void) {
    axis_case value = {.vehicle = {.axes = {.steering = INT8_MAX, .throttle = INT8_MIN},
                                   .drive = {.speed = -1, .heading = UINT16_MAX}}};
    fist_drive_control_events events = {.refresh_drive_display = true};
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        value.vehicle.type = (uint16_t)type;
        value.vehicle.component_size = fist_vehicle_component_size(value.vehicle.type);
        if (rejected(NULL, &events) != 0 || rejected(&value.vehicle, NULL) != 0) {
            return -1;
        }
        --value.vehicle.component_size;
        if (rejected(&value.vehicle, &events) != 0) {
            return -1;
        }
        ++value.vehicle.component_size;
        /* The complete consumer never reads or validates an unrelated target. */
        value.vehicle.command.target = (fist_object_reference){0, FIST_UNIT_REGISTRY_COUNT};
        if (step_case(&value, &events) != 0) {
            return -1;
        }
    }
    value.vehicle.type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    return rejected(&value.vehicle, &events);
}

static int prepare_case(const uint8_t *raw, axis_case *out) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .map_x = fist_read_i32le(raw + MAP_X_OFFSET),
                                             .map_y = fist_read_i32le(raw + MAP_Y_OFFSET),
                                             .altitude = fist_read_i32le(raw + ALTITUDE_OFFSET),
                                             .heading = fist_read_u16le(raw + HEADING_OFFSET),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    out->random = (fist_random){.words = {1, 2, HIGH_SEED, WORD_SEED}};
    out->steps = fist_read_u16le(raw + STEPS_OFFSET);
    out->stage = raw[STAGE_OFFSET];
    if (out->steps == 0 || out->stage > APPLY) {
        return -1;
    }
    if (out->stage == INITIALIZE) {
        return fist_vehicle_initialize(&definition, &out->random, raw[LINK_OFFSET], &out->vehicle);
    }
    if (fist_vehicle_restore(&definition, &out->vehicle) != 0) {
        return -1;
    }
    return out->stage == PREPARE ? fist_vehicle_prepare(&out->vehicle, raw[LINK_OFFSET]) : 0;
}

static void observe(const axis_case *value, const fist_drive_control_events *events) {
    fist_probe_write_vehicle_state(&value->vehicle);
    printf("axes %d %d %u\n", (int)value->vehicle.axes.steering, (int)value->vehicle.axes.throttle,
           (unsigned)events->refresh_drive_display);
    printf("axis_random %u %u %u %u %u\n", (unsigned)value->random.words[0],
           (unsigned)value->random.words[1], (unsigned)value->random.words[2],
           (unsigned)value->random.words[3], (unsigned)value->random.next_stream);
}

static int validate_cases(const uint8_t *data, size_t count, axis_case *cases) {
    for (size_t index = 0; index < count; ++index) {
        if (prepare_case(data + HEADER_BYTES + (index * CASE_BYTES), &cases[index]) != 0) {
            return -1;
        }
        axis_case checked = cases[index];
        if (checked.stage == APPLY) {
            for (unsigned step = 0; step < checked.steps; ++step) {
                fist_drive_control_events events = {.refresh_drive_display = true};
                if (step_case(&checked, &events) != 0) {
                    return -1;
                }
            }
        }
    }
    return 0;
}

static int observations(axis_case *cases, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        for (unsigned step = 0; step < cases[index].steps; ++step) {
            fist_drive_control_events events = {.refresh_drive_display =
                                                    cases[index].stage == APPLY};
            if (cases[index].stage == APPLY && step_case(&cases[index], &events) != 0) {
                return -1;
            }
            observe(&cases[index], &events);
        }
    }
    return ferror(stdout) == 0 ? 0 : -1;
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
    if (data == NULL || closed != 0 || size < HEADER_BYTES) {
        free(data);
        return EXIT_FAILURE;
    }
    const size_t count = fist_read_u32le(data);
    if ((size - HEADER_BYTES) % CASE_BYTES != 0 || count != (size - HEADER_BYTES) / CASE_BYTES) {
        free(data);
        return EXIT_FAILURE;
    }
    axis_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return EXIT_FAILURE;
    }
    int status = validate_cases(data, count, cases);
    free(data);
    if (status == 0) {
        status = observations(cases, count);
    }
    free(cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
