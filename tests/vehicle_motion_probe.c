#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/random.h"
#include "sim/rotation.h"
#include "sim/vehicle_motion.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_BYTES = 4,
    ROTATION_BYTES = 5,
    SPATIAL_BYTES = 7,
    MOTION_BYTES = FIST_UNIT_EXTENDED_SIZE + 2,
    PHASE_STEP = 2,
    MARKER = 123
};

typedef enum { MODE_DRIVE, MODE_HISTORY, MODE_MAINTENANCE } motion_mode;

typedef struct {
    fist_vehicle_state state;
    uint16_t steps;
} motion_case;

static int same_bytes(const unsigned char *expected, const unsigned char *observed, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        if (expected[index] != observed[index]) {
            return 0;
        }
    }
    return 1;
}

static int invalid_cases(fist_vehicle_state *state) {
    state->type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    fist_vehicle_motion_events events = {MARKER, MARKER, MARKER};
    unsigned char saved[sizeof(*state)] = {0};
    const unsigned char *bytes = (const unsigned char *)state;
    for (size_t index = 0; index < sizeof(*state); ++index) {
        saved[index] = bytes[index];
    }
    if (fist_vehicle_motion_step(NULL, &events) != -1 ||
        fist_vehicle_motion_step(state, NULL) != -1 ||
        fist_vehicle_motion_step(state, &events) != -1 || fist_vehicle_history_phase(NULL) != -1 ||
        fist_vehicle_maintenance_phase(NULL) != -1 || fist_vehicle_history_phase(state) != -1 ||
        fist_vehicle_maintenance_phase(state) != -1 || !same_bytes(saved, bytes, sizeof(*state))) {
        return -1;
    }
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        state->type = (uint16_t)type;
        for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
            if (size == fist_vehicle_component_size((uint16_t)type)) {
                continue;
            }
            state->component_size = size;
            for (size_t index = 0; index < sizeof(*state); ++index) {
                saved[index] = bytes[index];
            }
            if (fist_vehicle_motion_step(state, &events) != -1 ||
                fist_vehicle_history_phase(state) != -1 ||
                fist_vehicle_maintenance_phase(state) != -1 ||
                !same_bytes(saved, bytes, sizeof(*state))) {
                return -1;
            }
        }
    }
    return events.speed_changed == MARKER && events.hull_refreshed == MARKER &&
                   events.turret_changed == MARKER && fist_vehicle_component_size(UINT16_MAX) == 0
               ? 0
               : -1;
}

static int invalid_inputs(void) {
    /* Initialize padding too: byte preservation is the failure contract. */
    fist_vehicle_state *state = calloc(1, sizeof(*state));
    if (state == NULL) {
        return -1;
    }
    const int result = invalid_cases(state);
    free(state);
    return result;
}

static int rotations(const uint8_t *data, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = data + (index * ROTATION_BYTES);
        if (record[ROTATION_BYTES - 1] > 1) {
            return -1;
        }
    }
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = data + (index * ROTATION_BYTES);
        const fist_velocity velocity =
            fist_rotate((fist_rotation){.heading = fist_read_u16le(record),
                                        .magnitude = fist_read_i16le(record + 2),
                                        .coarse = record[ROTATION_BYTES - 1] != 0});
        printf("velocity %d %d\n", velocity.x, velocity.y);
    }
    return 0;
}

static int spatial_rotations(const uint8_t *data, size_t count) {
    enum { ELEVATION = 2, MAGNITUDE = 4 };
    for (size_t index = 0; index < count; ++index) {
        if (data[(index * SPATIAL_BYTES) + SPATIAL_BYTES - 1] > 1) {
            return -1;
        }
    }
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *record = data + (index * SPATIAL_BYTES);
        const fist_spatial_velocity velocity = fist_rotate_spatial(
            (fist_spatial_rotation){.heading = fist_read_u16le(record),
                                    .elevation = fist_read_u16le(record + ELEVATION),
                                    .magnitude = fist_read_i16le(record + MAGNITUDE),
                                    .coarse = record[SPATIAL_BYTES - 1] != 0});
        printf("velocity %d %d %d\n", velocity.x, velocity.y, velocity.z);
    }
    return 0;
}

static int prepare(const uint8_t *raw, motion_case *out, motion_mode mode) {
    enum {
        MAP_X = 4,
        MAP_Y = 8,
        ALTITUDE = 12,
        OPERATING_FLAGS = 26,
        CONTROL_FLAGS = 64,
        MOVEMENT_GATE = 93,
        TURRET_REQUEST = 139,
        CONTROL_MODE = 144
    };
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    fist_random random = {0};
    if (mode != MODE_DRIVE) {
        if (fist_vehicle_restore(&definition, &out->state) != 0) {
            return -1;
        }
        out->steps = fist_read_u16le(raw + FIST_UNIT_EXTENDED_SIZE);
        return out->steps != 0 ? 0 : -1;
    }
    if (fist_vehicle_initialize(&definition, &random, 0, &out->state) != 0) {
        return -1;
    }
    fist_vehicle_state *state = &out->state;
    state->operating_flags = raw[OPERATING_FLAGS];
    state->control_flags = fist_read_u16le(raw + CONTROL_FLAGS);
    state->drive.movement_gate = fist_read_u16le(raw + MOVEMENT_GATE);
    state->turret.requested_offset = fist_read_u16le(raw + TURRET_REQUEST);
    state->control_mode = raw[CONTROL_MODE];
    for (size_t index = 0; index < state->component_size; ++index) {
        state->components[index] = raw[fist_probe_component_offset(state->type) + index];
    }
    out->steps = fist_read_u16le(raw + FIST_UNIT_EXTENDED_SIZE);
    return out->steps != 0 ? 0 : -1;
}

static int phase_step(fist_vehicle_state *state, motion_mode mode) {
    unsigned char saved[sizeof(*state)] = {0};
    const unsigned char *bytes = (const unsigned char *)state;
    for (size_t index = 0; index < sizeof(*state); ++index) {
        saved[index] = bytes[index];
    }
    const int result = mode == MODE_HISTORY ? fist_vehicle_history_phase(state)
                                            : fist_vehicle_maintenance_phase(state);
    if (result != 0) {
        return -1;
    }
    const size_t history_start = offsetof(fist_vehicle_state, position_history);
    const size_t history_end = history_start + sizeof(state->position_history);
    for (size_t index = 0; index < sizeof(*state); ++index) {
        int owned = index == offsetof(fist_vehicle_state, random_phases) ||
                    (index >= history_start && index < history_end);
        if (mode == MODE_MAINTENANCE) {
            const size_t budget =
                offsetof(fist_vehicle_state, drive) + offsetof(fist_vehicle_drive, movement_gate);
            const size_t counter =
                offsetof(fist_vehicle_state, drive) + offsetof(fist_vehicle_drive, speed_counter);
            const size_t components = offsetof(fist_vehicle_state, components);
            owned = (index >= budget && index < budget + sizeof(state->drive.movement_gate)) ||
                    index == counter ||
                    (index >= components && index < components + state->component_size);
        }
        if (owned == 0 && bytes[index] != saved[index]) {
            return -1;
        }
    }
    return 0;
}

static int drive_cases(motion_mode mode, uint8_t *data, size_t count) {
    motion_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return -1;
    }
    int result = 0;
    for (size_t index = 0; index < count; ++index) {
        if (prepare(data + HEADER_BYTES + (index * MOTION_BYTES), &cases[index], mode) != 0) {
            result = -1;
            break;
        }
    }
    free(data); /* Every snapshot/request byte is gone before updates/observations. */
    for (size_t index = 0; index < count && result == 0; ++index) {
        fist_vehicle_state *state = &cases[index].state;
        for (unsigned step = 0; step < cases[index].steps; ++step) {
            if (mode != MODE_DRIVE) {
                if (phase_step(state, mode) != 0) {
                    result = -1;
                    break;
                }
                fist_probe_write_vehicle_state(state);
                continue;
            }
            fist_vehicle_motion_events events = {0};
            if (fist_vehicle_motion_step(state, &events) != 0) {
                result = -1;
                break;
            }
            fist_probe_write_vehicle_state(state);
            printf("events %u %u %u\n", (unsigned)events.speed_changed,
                   (unsigned)events.hull_refreshed, (unsigned)events.turret_changed);
            state->drive.update_phase = (uint8_t)(state->drive.update_phase + PHASE_STEP);
        }
    }
    free(cases);
    return result;
}

int main(int argc, char **argv) {
    if (argc != 3 || invalid_inputs() != 0) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[2], "rb");
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
    const int rotation = strcmp(argv[1], "rotation") == 0;
    const int spatial = strcmp(argv[1], "spatial") == 0;
    const int motion = strcmp(argv[1], "motion") == 0;
    const int history = strcmp(argv[1], "history") == 0;
    const int maintenance = strcmp(argv[1], "maintenance") == 0;
    motion_mode mode = history != 0 ? MODE_HISTORY : MODE_DRIVE;
    if (maintenance != 0) {
        mode = MODE_MAINTENANCE;
    }
    size_t record_bytes = MOTION_BYTES;
    if (rotation != 0) {
        record_bytes = ROTATION_BYTES;
    }
    if (spatial != 0) {
        record_bytes = SPATIAL_BYTES;
    }
    if ((rotation == 0 && spatial == 0 && motion == 0 && history == 0 && maintenance == 0) ||
        count != (size - HEADER_BYTES) / record_bytes ||
        (size - HEADER_BYTES) % record_bytes != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    int result = -1;
    if (rotation != 0 || spatial != 0) {
        result = spatial != 0 ? spatial_rotations(data + HEADER_BYTES, count)
                              : rotations(data + HEADER_BYTES, count);
        free(data);
    } else {
        result = drive_cases(mode, data, count);
    }
    return result == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
