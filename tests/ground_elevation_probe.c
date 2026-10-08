#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER_BYTES = 4,
    CASE_BYTES = FIST_UNIT_EXTENDED_SIZE + 23,
    CLOCK_OFFSET = FIST_UNIT_EXTENDED_SIZE,
    CONTROLS_OFFSET = CLOCK_OFFSET + 2,
    ACTION_OFFSET = CONTROLS_OFFSET + 6,
    STEPS_OFFSET = ACTION_OFFSET + 1,
    ADVANCE_OFFSET = STEPS_OFFSET + 2,
    LIFETIME_OFFSET = ADVANCE_OFFSET + 2,
    SLOT_OFFSET = LIFETIME_OFFSET + 8,
    MAP_X_OFFSET = 4,
    MAP_Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HEADING_OFFSET = 16,
    MARKER = 123,
    HIGH_WORD_SHIFT = 32
};

typedef struct {
    fist_vehicle_state vehicle;
    fist_weapon_elevation_controls controls;
    fist_weapon_elevation_action action;
    uint16_t clock;
    uint16_t advance;
    uint16_t steps;
} elevation_case;

static int step_case(elevation_case *value) {
    fist_vehicle_state before;
    fist_probe_capture(&value->vehicle, sizeof(before), &before);
    if (fist_weapon_adjust_elevation(&value->vehicle, value->action, &value->controls,
                                     value->clock) != 0) {
        return -1;
    }
    /* Independently restrict the complete typed write footprint. */
    fist_probe_capture(&value->vehicle.turret.elevation, sizeof(before.turret.elevation),
                       &before.turret.elevation);
    fist_probe_capture(&value->vehicle.turret.requested_offset,
                       sizeof(before.turret.requested_offset), &before.turret.requested_offset);
    fist_probe_capture(&value->vehicle.command.target_reference,
                       sizeof(before.command.target_reference), &before.command.target_reference);
    fist_probe_capture(&value->vehicle.command.target, sizeof(before.command.target),
                       &before.command.target);
    return fist_probe_unchanged(&value->vehicle, sizeof(before), &before) ? 0 : -1;
}

static int rejected(fist_vehicle_state *vehicle, fist_weapon_elevation_controls *controls,
                    fist_weapon_elevation_action action) {
    fist_vehicle_state before;
    fist_weapon_elevation_controls saved;
    if (vehicle != NULL) {
        fist_probe_capture(vehicle, sizeof(before), &before);
    }
    if (controls != NULL) {
        fist_probe_capture(controls, sizeof(saved), &saved);
    }
    return fist_weapon_adjust_elevation(vehicle, action, controls, MARKER) == -1 &&
                   (vehicle == NULL || fist_probe_unchanged(vehicle, sizeof(before), &before)) &&
                   (controls == NULL || fist_probe_unchanged(controls, sizeof(saved), &saved))
               ? 0
               : -1;
}

static int invalid_inputs(void) {
    fist_vehicle_state vehicle = {0};
    fist_weapon_elevation_controls controls = {MARKER, MARKER, MARKER};
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        vehicle.type = (uint16_t)type;
        vehicle.component_size = fist_vehicle_component_size(vehicle.type);
        for (int action = FIST_WEAPON_ELEVATION_PHASE; action <= FIST_WEAPON_ELEVATION_CENTER;
             ++action) {
            const fist_weapon_elevation_action operation = (fist_weapon_elevation_action)action;
            if (rejected(NULL, &controls, operation) != 0 ||
                rejected(&vehicle, NULL, operation) != 0) {
                return -1;
            }
            --vehicle.component_size;
            if (rejected(&vehicle, &controls, operation) != 0) {
                return -1;
            }
            ++vehicle.component_size;
            vehicle.drive.motion_flags = MARKER;
            vehicle.command.target = (fist_object_reference){1, FIST_UNIT_REGISTRY_COUNT};
            if (rejected(&vehicle, &controls, operation) != 0) {
                return -1;
            }
            vehicle.command.target = (fist_object_reference){0};
        }
        if (rejected(&vehicle, &controls, (fist_weapon_elevation_action)-1) != 0 ||
            rejected(&vehicle, &controls,
                     (fist_weapon_elevation_action)(FIST_WEAPON_ELEVATION_CENTER + 1)) != 0) {
            return -1;
        }
    }
    vehicle.type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    return rejected(&vehicle, &controls, FIST_WEAPON_ELEVATION_PHASE);
}

static int shared_controls(void) {
    enum {
        SHARED_STEP = 123,
        HELD_STEP = 18,
        FIRST_CLOCK = 10,
        FIRST_ELEVATION = 19,
        DIRECTION_FLAGS = 96,
        MANUAL_CLOCK = 14,
        MANUAL_STEP = 124,
        RAISED_ELEVATION = 143,
        QUICK_CLOCK = 999,
        QUICK_STEP = 728,
        QUICK_ELEVATION = -709,
        WRAPPED_ELEVATION = 37,
        CENTER_CLOCK = 20,
        LOWERED_ELEVATION = -585
    };
    fist_vehicle_state vehicles[FIST_UNIT_GROUND_VEHICLE_COUNT] = {0};
    fist_weapon_elevation_controls controls = {0, SHARED_STEP, HELD_STEP};
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        vehicles[type].type = (uint16_t)type;
        vehicles[type].component_size = fist_vehicle_component_size((uint16_t)type);
        vehicles[type].drive.motion_flags = DIRECTION_FLAGS;
        const uint16_t clock = (uint16_t)(FIRST_CLOCK + type);
        if (fist_weapon_adjust_elevation(&vehicles[type], FIST_WEAPON_ELEVATION_PHASE, &controls,
                                         clock) != 0 ||
            controls.previous_clock != clock || controls.step != SHARED_STEP ||
            controls.held_step != HELD_STEP || vehicles[type].turret.elevation != FIRST_ELEVATION) {
            return -1;
        }
    }
    /* Exact nine-call shared-owner sequence independently proved by the
     * versioned original elevation_shared.py fixture. */
    if (fist_weapon_adjust_elevation(&vehicles[2], FIST_WEAPON_ELEVATION_RAISE, &controls,
                                     MANUAL_CLOCK) != 0 ||
        controls.previous_clock != MANUAL_CLOCK || controls.step != MANUAL_STEP ||
        vehicles[2].turret.elevation != RAISED_ELEVATION ||
        fist_weapon_adjust_elevation(&vehicles[3], FIST_WEAPON_ELEVATION_QUICK_LOWER, &controls,
                                     QUICK_CLOCK) != 0 ||
        controls.previous_clock != MANUAL_CLOCK || controls.step != QUICK_STEP ||
        vehicles[3].turret.elevation != QUICK_ELEVATION ||
        fist_weapon_adjust_elevation(&vehicles[0], FIST_WEAPON_ELEVATION_PHASE, &controls,
                                     UINT16_MAX) != 0 ||
        controls.previous_clock != UINT16_MAX || controls.step != QUICK_STEP ||
        vehicles[0].turret.elevation != WRAPPED_ELEVATION) {
        return -1;
    }
    vehicles[1].command.target_reference = 1;
    vehicles[1].command.target = (fist_object_reference){1, FIST_POOL_SHORT_SLOTS};
    if (fist_weapon_adjust_elevation(&vehicles[1], FIST_WEAPON_ELEVATION_CENTER, &controls,
                                     CENTER_CLOCK) != 0 ||
        controls.previous_clock != UINT16_MAX || controls.step != QUICK_STEP ||
        vehicles[1].turret.elevation != 0 || vehicles[1].command.target_reference != 0 ||
        vehicles[1].command.target.lifetime != 0 || vehicles[1].command.target.slot != 0 ||
        fist_weapon_adjust_elevation(&vehicles[2], FIST_WEAPON_ELEVATION_LOWER, &controls, 0) !=
            0 ||
        controls.previous_clock != 0 || controls.step != QUICK_STEP ||
        vehicles[2].turret.elevation != LOWERED_ELEVATION) {
        return -1;
    }
    return controls.held_step == HELD_STEP ? 0 : -1;
}

static int prepare(const uint8_t *raw, elevation_case *out) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .map_x = fist_read_i32le(raw + MAP_X_OFFSET),
                                             .map_y = fist_read_i32le(raw + MAP_Y_OFFSET),
                                             .altitude = fist_read_i32le(raw + ALTITUDE_OFFSET),
                                             .heading = fist_read_u16le(raw + HEADING_OFFSET),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    if (fist_vehicle_restore(&definition, &out->vehicle) != 0) {
        return -1;
    }
    out->controls = (fist_weapon_elevation_controls){
        fist_read_u16le(raw + CONTROLS_OFFSET),
        fist_read_u16le(raw + CONTROLS_OFFSET + sizeof(uint16_t)),
        fist_read_u16le(raw + CONTROLS_OFFSET + (2 * sizeof(uint16_t)))};
    out->clock = fist_read_u16le(raw + CLOCK_OFFSET);
    out->advance = fist_read_u16le(raw + ADVANCE_OFFSET);
    out->steps = fist_read_u16le(raw + STEPS_OFFSET);
    out->action = (fist_weapon_elevation_action)raw[ACTION_OFFSET];
    out->vehicle.command.target = (fist_object_reference){
        (uint64_t)fist_read_u32le(raw + LIFETIME_OFFSET) |
            ((uint64_t)fist_read_u32le(raw + LIFETIME_OFFSET + sizeof(uint32_t))
             << HIGH_WORD_SHIFT),
        fist_read_u16le(raw + SLOT_OFFSET)};
    return out->steps == 0 ? -1 : 0;
}

static void observe(const elevation_case *value) {
    fist_probe_write_vehicle_state(&value->vehicle);
    printf("elevation_controls %u %u %u\n", (unsigned)value->controls.previous_clock,
           (unsigned)value->controls.step, (unsigned)value->controls.held_step);
    printf("elevation_target %llu %u\n", (unsigned long long)value->vehicle.command.target.lifetime,
           (unsigned)value->vehicle.command.target.slot);
}

static int run_cases(const uint8_t *data, size_t count, elevation_case *cases) {
    for (size_t index = 0; index < count; ++index) {
        if (prepare(data + HEADER_BYTES + (index * CASE_BYTES), &cases[index]) != 0) {
            return -1;
        }
        elevation_case checked = cases[index];
        for (unsigned step = 0; step < checked.steps; ++step) {
            if (step_case(&checked) != 0) {
                return -1;
            }
            checked.clock = (uint16_t)(checked.clock + checked.advance);
        }
    }
    return 0;
}

static int observations(elevation_case *cases, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        for (unsigned step = 0; step < cases[index].steps; ++step) {
            if (step_case(&cases[index]) != 0) {
                return -1;
            }
            observe(&cases[index]);
            cases[index].clock = (uint16_t)(cases[index].clock + cases[index].advance);
        }
    }
    return ferror(stdout) == 0 ? 0 : -1;
}

int main(int argc, char **argv) {
    if (argc != 2 || invalid_inputs() != 0 || shared_controls() != 0) {
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
    elevation_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return EXIT_FAILURE;
    }
    int status = run_cases(data, count, cases);
    free(data); /* The complete restored state and controls own their values. */
    if (status == 0) {
        status = observations(cases, count);
    }
    free(cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
