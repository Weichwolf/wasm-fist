#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "vehicle_probe_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_BYTES = 4,
    CASE_BYTES = FIST_UNIT_EXTENDED_SIZE + 16,
    SELECTOR_OFFSET = FIST_UNIT_EXTENDED_SIZE,
    DIRECTION_OFFSET = SELECTOR_OFFSET + 2,
    CALLER_OFFSET = DIRECTION_OFFSET + 1,
    STEPS_OFFSET = CALLER_OFFSET + 1,
    LIFETIME_OFFSET = STEPS_OFFSET + 2,
    SLOT_OFFSET = LIFETIME_OFFSET + 8,
    MAP_X_OFFSET = 4,
    MAP_Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HEADING_OFFSET = 16,
    HIGH_WORD_SHIFT = 32,
    SHARED_TICKS = 512,
    VIEW_PERIOD = 256,
    TARGET_PERIOD = 3,
    MIXED_ROUNDS = 8
};

typedef struct {
    fist_vehicle_state vehicle;
    fist_weapon_turret_controls controls;
    fist_weapon_turret_direction direction;
    uint16_t steps;
    uint8_t caller;
} turret_case;

static int apply(fist_vehicle_state *vehicle, fist_weapon_turret_controls *controls,
                 fist_weapon_turret_direction direction, unsigned caller) {
    return caller != 0 ? fist_weapon_turn_turret_input(vehicle, direction, controls)
                       : fist_weapon_turn_turret(vehicle, direction, controls);
}

static int step_with_controls(turret_case *value, fist_weapon_turret_controls *controls) {
    static const size_t original_refresh[FIST_UNIT_GROUND_VEHICLE_COUNT] = {199, 196, 230, 215};
    fist_vehicle_state before;
    fist_weapon_turret_controls saved;
    fist_probe_capture(&value->vehicle, sizeof(before), &before);
    fist_probe_capture(controls, sizeof(saved), &saved);
    if (apply(&value->vehicle, controls, value->direction, value->caller) != 0) {
        return -1;
    }
    fist_probe_capture(&value->vehicle.control_flags, sizeof(before.control_flags),
                       &before.control_flags);
    fist_probe_capture(&value->vehicle.turret.requested_offset,
                       sizeof(before.turret.requested_offset), &before.turret.requested_offset);
    fist_probe_capture(&value->vehicle.turret.elevation, sizeof(before.turret.elevation),
                       &before.turret.elevation);
    fist_probe_capture(&value->vehicle.command.target_reference,
                       sizeof(before.command.target_reference), &before.command.target_reference);
    fist_probe_capture(&value->vehicle.command.target, sizeof(before.command.target),
                       &before.command.target);
    const size_t component =
        original_refresh[value->vehicle.type] - fist_probe_component_offset(value->vehicle.type);
    fist_probe_capture(&value->vehicle.components[component], sizeof(before.components[component]),
                       &before.components[component]);
    return fist_probe_unchanged(&value->vehicle, sizeof(before), &before) &&
                   (value->caller != 0 || fist_probe_unchanged(controls, sizeof(saved), &saved))
               ? 0
               : -1;
}

static int step_case(turret_case *value) {
    return step_with_controls(value, &value->controls);
}

static int rejected(fist_vehicle_state *vehicle, fist_weapon_turret_controls *controls,
                    fist_weapon_turret_direction direction, unsigned caller) {
    fist_vehicle_state before;
    fist_weapon_turret_controls saved;
    if (vehicle != NULL) {
        fist_probe_capture(vehicle, sizeof(before), &before);
    }
    if (controls != NULL) {
        fist_probe_capture(controls, sizeof(saved), &saved);
    }
    return apply(vehicle, controls, direction, caller) == -1 &&
                   (vehicle == NULL || fist_probe_unchanged(vehicle, sizeof(before), &before)) &&
                   (controls == NULL || fist_probe_unchanged(controls, sizeof(saved), &saved))
               ? 0
               : -1;
}

static int invalid_operation(turret_case *value) {
    const fist_object_reference malformed[] = {
        {1, FIST_UNIT_REGISTRY_COUNT}, {0, 1}, {1, UINT16_MAX}};
    if (rejected(NULL, &value->controls, value->direction, value->caller) != 0 ||
        rejected(&value->vehicle, NULL, value->direction, value->caller) != 0) {
        return -1;
    }
    --value->vehicle.component_size;
    if (rejected(&value->vehicle, &value->controls, value->direction, value->caller) != 0) {
        return -1;
    }
    ++value->vehicle.component_size;
    for (size_t index = 0; index < sizeof(malformed) / sizeof(malformed[0]); ++index) {
        value->vehicle.command.target = malformed[index];
        if (rejected(&value->vehicle, &value->controls, value->direction, value->caller) != 0) {
            return -1;
        }
    }
    value->vehicle.command.target = (fist_object_reference){0};
    return 0;
}

static int invalid_inputs(void) {
    turret_case value = {
        .vehicle = {.control_flags = UINT16_MAX,
                    .turret = {.elevation = INT16_MIN, .requested_offset = UINT16_MAX}},
        .controls = {.selector = UINT16_MAX}};
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        value.vehicle.type = (uint16_t)type;
        value.vehicle.component_size = fist_vehicle_component_size(value.vehicle.type);
        for (unsigned caller = 0; caller <= 1; ++caller) {
            value.caller = (uint8_t)caller;
            for (int direction = FIST_WEAPON_TURRET_LEFT; direction <= FIST_WEAPON_TURRET_RIGHT;
                 ++direction) {
                value.direction = (fist_weapon_turret_direction)direction;
                if (invalid_operation(&value) != 0) {
                    return -1;
                }
            }
            if (rejected(&value.vehicle, &value.controls, FIST_WEAPON_TURRET_INVALID, caller) !=
                    0 ||
                rejected(&value.vehicle, &value.controls, FIST_WEAPON_TURRET_DIRECTION_COUNT,
                         caller) != 0) {
                return -1;
            }
        }
    }
    value.vehicle.type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    return rejected(&value.vehicle, &value.controls, FIST_WEAPON_TURRET_LEFT, 0) == 0 &&
                   rejected(&value.vehicle, &value.controls, FIST_WEAPON_TURRET_RIGHT, 1) == 0
               ? 0
               : -1;
}

static int prepare(const uint8_t *raw, turret_case *out) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .map_x = fist_read_i32le(raw + MAP_X_OFFSET),
                                             .map_y = fist_read_i32le(raw + MAP_Y_OFFSET),
                                             .altitude = fist_read_i32le(raw + ALTITUDE_OFFSET),
                                             .heading = fist_read_u16le(raw + HEADING_OFFSET),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    if (fist_vehicle_restore(&definition, &out->vehicle) != 0) {
        return -1;
    }
    if (raw[DIRECTION_OFFSET] > FIST_WEAPON_TURRET_RIGHT || raw[CALLER_OFFSET] > 1) {
        return -1;
    }
    out->controls.selector = fist_read_u16le(raw + SELECTOR_OFFSET);
    out->direction = (fist_weapon_turret_direction)raw[DIRECTION_OFFSET];
    out->caller = raw[CALLER_OFFSET];
    out->steps = fist_read_u16le(raw + STEPS_OFFSET);
    out->vehicle.command.target = (fist_object_reference){
        (uint64_t)fist_read_u32le(raw + LIFETIME_OFFSET) |
            ((uint64_t)fist_read_u32le(raw + LIFETIME_OFFSET + sizeof(uint32_t))
             << HIGH_WORD_SHIFT),
        fist_read_u16le(raw + SLOT_OFFSET)};
    return out->steps != 0 && out->caller <= 1 ? 0 : -1;
}

static void observe(const turret_case *value) {
    fist_probe_write_vehicle_state(&value->vehicle);
    printf("turret_controls %u\n", (unsigned)value->controls.selector);
    printf("turret_target %llu %u\n", (unsigned long long)value->vehicle.command.target.lifetime,
           (unsigned)value->vehicle.command.target.slot);
}

static int validate_cases(const uint8_t *data, size_t count, turret_case *cases) {
    for (size_t index = 0; index < count; ++index) {
        if (prepare(data + HEADER_BYTES + (index * CASE_BYTES), &cases[index]) != 0) {
            return -1;
        }
        turret_case checked = cases[index];
        for (unsigned step = 0; step < checked.steps; ++step) {
            if (step_case(&checked) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int observations(turret_case *cases, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        for (unsigned step = 0; step < cases[index].steps; ++step) {
            if (step_case(&cases[index]) != 0) {
                return -1;
            }
            observe(&cases[index]);
        }
    }
    return 0;
}

typedef struct {
    turret_case *cases;
    fist_weapon_turret_controls controls;
    bool output;
} shared_turret;

static int shared_step(shared_turret *shared, size_t kind) {
    turret_case before[FIST_UNIT_GROUND_VEHICLE_COUNT];
    fist_probe_capture(shared->cases, sizeof(before), before);
    turret_case *value = &shared->cases[kind];
    if (step_with_controls(value, &shared->controls) != 0) {
        return -1;
    }
    for (size_t other = 0; other < FIST_UNIT_GROUND_VEHICLE_COUNT; ++other) {
        if (other != kind &&
            !fist_probe_unchanged(&shared->cases[other], sizeof(before[other]), &before[other])) {
            return -1;
        }
    }
    value->controls = shared->controls;
    if (shared->output) {
        observe(value);
    }
    return 0;
}

static int shared_callers(shared_turret *shared) {
    /* Exact caller/view/control/target sequence from manual_turret_shared.py,
     * on owned input actors. Only these caller boundaries are rewritten. */
    for (unsigned tick = 0; tick < SHARED_TICKS; ++tick) {
        for (size_t kind = 0; kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
            turret_case *value = &shared->cases[kind];
            value->direction = (tick / VIEW_PERIOD + kind) % 2 != 0 ? FIST_WEAPON_TURRET_RIGHT
                                                                    : FIST_WEAPON_TURRET_LEFT;
            value->caller = 1;
            value->vehicle.turret_view_mode = (uint8_t)(tick % VIEW_PERIOD);
            value->vehicle.control_flags = tick % 2 != 0 ? UINT16_MAX : 0;
            value->vehicle.command.target_reference =
                tick % TARGET_PERIOD == 0
                    ? (uint16_t)(1 + ((kind + 1) % FIST_UNIT_GROUND_VEHICLE_COUNT))
                    : 0;
            value->vehicle.command.target = (fist_object_reference){0};
            if (shared_step(shared, kind) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int shared_helpers(shared_turret *shared) {
    /* Direct helpers reuse the same shared object after fixed callers. */
    for (unsigned round = 0; round < MIXED_ROUNDS; ++round) {
        for (size_t kind = 0; kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
            turret_case *value = &shared->cases[kind];
            value->direction =
                (round + kind) % 2 != 0 ? FIST_WEAPON_TURRET_RIGHT : FIST_WEAPON_TURRET_LEFT;
            value->caller = 0;
            if (shared_step(shared, kind) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int shared_run(size_t count, turret_case *cases, bool output) {
    if (count != FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    for (size_t kind = 0; kind < count; ++kind) {
        if (cases[kind].vehicle.type != kind) {
            return -1;
        }
    }
    shared_turret shared = {.cases = cases, .controls = cases[0].controls, .output = output};
    return shared_callers(&shared) == 0 ? shared_helpers(&shared) : -1;
}

int main(int argc, char **argv) {
    const int shared = argc == 3 && strcmp(argv[2], "--shared") == 0;
    if ((argc != 2 && !shared) || invalid_inputs() != 0) {
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
    turret_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return EXIT_FAILURE;
    }
    int status = validate_cases(data, count, cases);
    free(data);
    if (status == 0 && shared != 0) {
        turret_case checked[FIST_UNIT_GROUND_VEHICLE_COUNT];
        if (count != FIST_UNIT_GROUND_VEHICLE_COUNT) {
            status = -1;
        } else {
            fist_probe_capture(cases, sizeof(checked), checked);
            status = shared_run(count, checked, false);
        }
    }
    if (status == 0) {
        status = shared != 0 ? shared_run(count, cases, true) : observations(cases, count);
    }
    free(cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
