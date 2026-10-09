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
#include <string.h>

enum {
    HEADER_BYTES = 4,
    WEAPON_INPUT_OFFSET = 160,
    VIEW_INPUT_OFFSET = 164,
    MODE_OFFSET = FIST_UNIT_EXTENDED_SIZE,
    CLOCK_OFFSET = MODE_OFFSET + 2,
    PREVIOUS_OFFSET = CLOCK_OFFSET + 2,
    STEP_OFFSET = PREVIOUS_OFFSET + 2,
    HELD_OFFSET = STEP_OFFSET + 2,
    SELECTOR_OFFSET = HELD_OFFSET + 2,
    SELECTED_OFFSET = SELECTOR_OFFSET + 2,
    STEPS_OFFSET = SELECTED_OFFSET + 1,
    LIFETIME_OFFSET = STEPS_OFFSET + 2,
    SLOT_OFFSET = LIFETIME_OFFSET + 8,
    CASE_BYTES = SLOT_OFFSET + 2,
    MAP_X_OFFSET = 4,
    MAP_Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HEADING_OFFSET = 16,
    HIGH_WORD_SHIFT = 32,
    MODE_MASK = 0x7fff,
    MODE_COUNT = 6,
    VIEW_MODE = 2,
    LEFT_ACTION = 2,
    RIGHT_ACTION = 4,
    RAISE_ACTION = 6,
    LOWER_ACTION = 8,
    HIGH_SEED = 32768,
    WORD_SEED = 65535,
    SHARED_TICKS = 1024,
    ADMISSION_PERIOD = 31,
    TARGET_PERIOD = 5,
    SELECTION_PERIOD = 13,
    MODE_PERIOD = 32,
    ALIAS_FLAG = 128,
    VIEW_MULTIPLIER = 37,
    KIND_MULTIPLIER = 53
};

typedef struct {
    fist_vehicle_state vehicle;
    fist_manual_controls controls;
    fist_manual_input input;
    uint16_t steps;
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE];
} manual_case;

static fist_unit_definition definition(const uint8_t *raw) {
    return (fist_unit_definition){.type = fist_read_u16le(raw),
                                  .map_x = fist_read_i32le(raw + MAP_X_OFFSET),
                                  .map_y = fist_read_i32le(raw + MAP_Y_OFFSET),
                                  .altitude = fist_read_i32le(raw + ALTITUDE_OFFSET),
                                  .heading = fist_read_u16le(raw + HEADING_OFFSET),
                                  .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
}

static void capture_component(const fist_vehicle_state *vehicle, fist_vehicle_state *before,
                              size_t offset) {
    const size_t index = offset - fist_probe_component_offset(vehicle->type);
    fist_probe_capture(&vehicle->components[index], sizeof(before->components[index]),
                       &before->components[index]);
}

static void capture_drive(const manual_case *value, fist_vehicle_state *before) {
    static const size_t profile[FIST_UNIT_GROUND_VEHICLE_COUNT] = {214, 200, 202, 210};
    static const size_t view[FIST_UNIT_GROUND_VEHICLE_COUNT][3] = {
        {237, 239, 241}, {240, 242, 244}, {231, 233, 235}, {237, 239, 241}};
    const uint16_t mode = (uint16_t)(value->input.drive_mode & MODE_MASK);
    if ((before->control_flags & 1) != 0 || mode == 0 || mode == MODE_COUNT - 1) {
        return;
    }
    const fist_vehicle_state *vehicle = &value->vehicle;
    fist_probe_capture(&vehicle->drive.requested_heading, sizeof(before->drive.requested_heading),
                       &before->drive.requested_heading);
    fist_probe_capture(&vehicle->drive.throttle, sizeof(before->drive.throttle),
                       &before->drive.throttle);
    fist_probe_capture(&vehicle->control_mode, sizeof(before->control_mode), &before->control_mode);
    capture_component(vehicle, before, profile[vehicle->type]);
    if (mode == VIEW_MODE) {
        fist_probe_capture(&vehicle->turret_view_mode, sizeof(before->turret_view_mode),
                           &before->turret_view_mode);
        for (size_t index = 0; index < sizeof(view[vehicle->type]) / sizeof(view[0][0]); ++index) {
            capture_component(vehicle, before, view[vehicle->type][index]);
        }
    }
}

static void capture_weapon(const manual_case *value, const fist_manual_controls *controls,
                           fist_vehicle_state *before, fist_manual_controls *saved) {
    static const size_t refresh[FIST_UNIT_GROUND_VEHICLE_COUNT] = {199, 196, 230, 215};
    const fist_vehicle_state *vehicle = &value->vehicle;
    const uint8_t action = vehicle->manual_input.weapon_action;
    if (action == 0) {
        return;
    }
    fist_probe_capture(&vehicle->control_flags, sizeof(before->control_flags),
                       &before->control_flags);
    fist_probe_capture(&vehicle->turret.elevation, sizeof(before->turret.elevation),
                       &before->turret.elevation);
    fist_probe_capture(&vehicle->command.target_reference, sizeof(before->command.target_reference),
                       &before->command.target_reference);
    fist_probe_capture(&vehicle->command.target, sizeof(before->command.target),
                       &before->command.target);
    capture_component(vehicle, before, refresh[vehicle->type]);
    if (action == LEFT_ACTION || action == RIGHT_ACTION) {
        fist_probe_capture(&vehicle->turret.requested_offset,
                           sizeof(before->turret.requested_offset),
                           &before->turret.requested_offset);
        fist_probe_capture(&controls->turret, sizeof(saved->turret), &saved->turret);
    } else {
        fist_probe_capture(&controls->elevation, sizeof(saved->elevation), &saved->elevation);
    }
}

static int step_case(manual_case *value, fist_manual_controls *controls,
                     fist_manual_events *events) {
    fist_vehicle_state before;
    fist_manual_controls saved;
    fist_manual_input input;
    fist_probe_capture(&value->vehicle, sizeof(before), &before);
    fist_probe_capture(controls, sizeof(saved), &saved);
    fist_probe_capture(&value->input, sizeof(input), &input);
    if (fist_driver_apply_manual(&value->vehicle, &value->input, controls, events) != 0) {
        return -1;
    }
    if (value->input.selected) {
        capture_drive(value, &before);
        capture_weapon(value, controls, &before, &saved);
    }
    return fist_probe_unchanged(&value->vehicle, sizeof(before), &before) &&
                   fist_probe_unchanged(controls, sizeof(saved), &saved) &&
                   fist_probe_unchanged(&value->input, sizeof(input), &input)
               ? 0
               : -1;
}

static int rejected(fist_vehicle_state *vehicle, const fist_manual_input *input,
                    fist_manual_controls *controls, fist_manual_events *events) {
    fist_vehicle_state before;
    fist_manual_controls saved;
    fist_manual_events emitted;
    if (vehicle != NULL) {
        fist_probe_capture(vehicle, sizeof(before), &before);
    }
    if (controls != NULL) {
        fist_probe_capture(controls, sizeof(saved), &saved);
    }
    if (events != NULL) {
        fist_probe_capture(events, sizeof(emitted), &emitted);
    }
    return fist_driver_apply_manual(vehicle, input, controls, events) == -1 &&
                   (vehicle == NULL || fist_probe_unchanged(vehicle, sizeof(before), &before)) &&
                   (controls == NULL || fist_probe_unchanged(controls, sizeof(saved), &saved)) &&
                   (events == NULL || fist_probe_unchanged(events, sizeof(emitted), &emitted))
               ? 0
               : -1;
}

static int prepare(const uint8_t *raw, manual_case *out) {
    if (raw[SELECTED_OFFSET] > 1 || fist_read_u16le(raw + STEPS_OFFSET) == 0) {
        return -1;
    }
    fist_probe_capture(raw, sizeof(out->raw), out->raw);
    const fist_unit_definition source = definition(out->raw);
    if (fist_vehicle_restore(&source, &out->vehicle) != 0) {
        return -1;
    }
    out->input = (fist_manual_input){.drive_mode = fist_read_u16le(raw + MODE_OFFSET),
                                     .clock = fist_read_u16le(raw + CLOCK_OFFSET),
                                     .selected = raw[SELECTED_OFFSET] != 0};
    out->controls = (fist_manual_controls){
        .elevation = {fist_read_u16le(raw + PREVIOUS_OFFSET), fist_read_u16le(raw + STEP_OFFSET),
                      fist_read_u16le(raw + HELD_OFFSET)},
        .turret = {.selector = fist_read_u16le(raw + SELECTOR_OFFSET)}};
    out->steps = fist_read_u16le(raw + STEPS_OFFSET);
    out->vehicle.command.target = (fist_object_reference){
        (uint64_t)fist_read_u32le(raw + LIFETIME_OFFSET) |
            ((uint64_t)fist_read_u32le(raw + LIFETIME_OFFSET + sizeof(uint32_t))
             << HIGH_WORD_SHIFT),
        fist_read_u16le(raw + SLOT_OFFSET)};
    return 0;
}

static void observe_state(const fist_vehicle_state *vehicle) {
    fist_probe_write_vehicle_state(vehicle);
    printf("manual_inputs %u %u\n", (unsigned)vehicle->manual_input.weapon_action,
           (unsigned)vehicle->manual_input.view_selector);
}

static void observe(const manual_case *value, const fist_manual_controls *controls,
                    const fist_manual_events *events) {
    observe_state(&value->vehicle);
    printf("manual_controls %u %u %u %u\n", (unsigned)controls->elevation.previous_clock,
           (unsigned)controls->elevation.step, (unsigned)controls->elevation.held_step,
           (unsigned)controls->turret.selector);
    printf("manual_events %u %u\n", (unsigned)events->drive.refresh_drive_display,
           (unsigned)events->refresh_view_display);
    printf("manual_target %llu %u\n", (unsigned long long)value->vehicle.command.target.lifetime,
           (unsigned)value->vehicle.command.target.slot);
}

static int normal_run(manual_case *cases, size_t count, bool output) {
    for (size_t index = 0; index < count; ++index) {
        for (unsigned tick = 0; tick < cases[index].steps; ++tick) {
            fist_manual_events events = {.drive.refresh_drive_display = true,
                                         .refresh_view_display = true};
            if (step_case(&cases[index], &cases[index].controls, &events) != 0) {
                return -1;
            }
            if (output) {
                observe(&cases[index], &cases[index].controls, &events);
            }
        }
    }
    return 0;
}

static int retention_run(manual_case *cases, size_t count, bool output) {
    for (size_t index = 0; index < count; ++index) {
        manual_case *value = &cases[index];
        const fist_unit_definition source = definition(value->raw);
        fist_random random = {.words = {1, 2, HIGH_SEED, WORD_SEED}};
        if (value->input.drive_mode != 0 && value->input.drive_mode != 2) {
            return -1;
        }
        if (output) {
            observe_state(&value->vehicle);
        }
        if (fist_vehicle_initialize(&source, &random, (uint8_t)value->input.drive_mode,
                                    &value->vehicle) != 0 ||
            value->vehicle.manual_input.weapon_action != value->raw[WEAPON_INPUT_OFFSET] ||
            value->vehicle.manual_input.view_selector != value->raw[VIEW_INPUT_OFFSET]) {
            return -1;
        }
        if (output) {
            observe_state(&value->vehicle);
            printf("manual_random %u %u %u %u %u\n", (unsigned)random.next_stream,
                   (unsigned)random.words[0], (unsigned)random.words[1], (unsigned)random.words[2],
                   (unsigned)random.words[3]);
        }
        fist_random saved;
        fist_probe_capture(&random, sizeof(saved), &saved);
        if (fist_vehicle_prepare(&value->vehicle, (uint8_t)value->input.drive_mode) != 0 ||
            value->vehicle.manual_input.weapon_action != value->raw[WEAPON_INPUT_OFFSET] ||
            value->vehicle.manual_input.view_selector != value->raw[VIEW_INPUT_OFFSET] ||
            !fist_probe_unchanged(&random, sizeof(saved), &saved)) {
            return -1;
        }
        if (output) {
            observe_state(&value->vehicle);
        }
    }
    return 0;
}

typedef struct {
    manual_case *cases;
    fist_manual_controls controls;
    bool output;
} shared_manual;

static int shared_step(shared_manual *shared, size_t kind) {
    manual_case before[FIST_UNIT_GROUND_VEHICLE_COUNT];
    fist_probe_capture(shared->cases, sizeof(before), before);
    manual_case *value = &shared->cases[kind];
    fist_manual_events events = {.drive.refresh_drive_display = true, .refresh_view_display = true};
    if (step_case(value, &shared->controls, &events) != 0) {
        return -1;
    }
    for (size_t other = 0; other < FIST_UNIT_GROUND_VEHICLE_COUNT; ++other) {
        if (other != kind &&
            !fist_probe_unchanged(&shared->cases[other], sizeof(before[other]), &before[other])) {
            return -1;
        }
    }
    if (shared->output) {
        observe(value, &shared->controls, &events);
    }
    return 0;
}

static void shared_input(manual_case *value, unsigned tick, size_t kind, uint16_t seed) {
    static const uint8_t actions[] = {0, LEFT_ACTION, RIGHT_ACTION, RAISE_ACTION, LOWER_ACTION};
    static const fist_vehicle_axes axes[] = {{-128, -128}, {-24, -25}, {-23, -24}, {0, -23},
                                             {23, 0},      {24, 23},   {127, 24},  {127, 127}};
    static const int16_t speeds[] = {INT16_MIN, -1, 0, INT16_MAX};
    static const uint16_t elapsed[] = {1, 19, 20, UINT16_MAX};
    value->input.selected = (tick + kind) % SELECTION_PERIOD != 0;
    value->vehicle.manual_input.weapon_action =
        value->input.selected ? actions[(tick + kind) % (sizeof(actions) / sizeof(actions[0]))]
                              : UINT8_MAX;
    value->vehicle.manual_input.view_selector =
        (uint8_t)(((size_t)tick * VIEW_MULTIPLIER) + (kind * KIND_MULTIPLIER));
    value->vehicle.axes = axes[(tick + kind) % (sizeof(axes) / sizeof(axes[0]))];
    value->vehicle.drive.speed = speeds[tick % (sizeof(speeds) / sizeof(speeds[0]))];
    if (tick % TARGET_PERIOD == 0) {
        value->vehicle.command.target_reference =
            (uint16_t)(1 + ((kind + 1) % FIST_UNIT_GROUND_VEHICLE_COUNT));
    }
    value->input.drive_mode = (uint16_t)(((tick / MODE_PERIOD) + kind) % MODE_COUNT);
    if (tick % ADMISSION_PERIOD == 0) {
        value->vehicle.control_flags |= 1;
        value->input.drive_mode = UINT16_MAX;
    }
    if (!value->input.selected) {
        value->input.drive_mode = UINT16_MAX;
    }
    if ((tick & ALIAS_FLAG) != 0 && value->input.drive_mode != UINT16_MAX) {
        value->input.drive_mode = (uint16_t)(value->input.drive_mode + HIGH_SEED);
    }
    value->input.clock =
        (uint16_t)(seed + ((size_t)tick * elapsed[tick % (sizeof(elapsed) / sizeof(elapsed[0]))]) +
                   kind);
}

static int shared_run(manual_case *cases, size_t count, bool output) {
    if (count != FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    for (size_t kind = 0; kind < count; ++kind) {
        if (cases[kind].vehicle.type != kind) {
            return -1;
        }
    }
    shared_manual shared = {.cases = cases, .controls = cases[0].controls, .output = output};
    const uint16_t seed = cases[0].input.clock;
    for (unsigned tick = 0; tick < SHARED_TICKS; ++tick) {
        for (size_t kind = 0; kind < count; ++kind) {
            shared_input(&cases[kind], tick, kind, seed);
            if (shared_step(&shared, kind) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

typedef struct {
    unsigned drive;
    unsigned action;
    unsigned target;
    unsigned unused;
    unsigned basic;
} guard_counts;

static manual_case guard_case(uint16_t type) {
    return (manual_case){
        .vehicle = {.type = type,
                    .component_size = fist_vehicle_component_size(type),
                    .axes = {INT8_MAX, INT8_MIN},
                    .manual_input = {.view_selector = UINT8_MAX},
                    .drive = {.speed = -1, .heading = UINT16_MAX},
                    .turret = {.elevation = INT16_MIN, .requested_offset = UINT16_MAX},
                    .control_mode = UINT8_MAX},
        .input = {.drive_mode = VIEW_MODE, .clock = UINT16_MAX, .selected = true},
        .controls = {.elevation = {UINT16_MAX, WORD_SEED, HIGH_SEED},
                     .turret = {.selector = UINT16_MAX}}};
}

static int guard_drive(manual_case *value, guard_counts *counts) {
    fist_manual_events events = {.drive.refresh_drive_display = true, .refresh_view_display = true};
    for (unsigned mode = 0; mode <= UINT16_MAX; ++mode) {
        if ((mode & MODE_MASK) < MODE_COUNT) {
            continue;
        }
        value->input.drive_mode = (uint16_t)mode;
        if (rejected(&value->vehicle, &value->input, &value->controls, &events) != 0) {
            return -1;
        }
        ++counts->drive;
    }
    value->input.drive_mode = VIEW_MODE;
    return 0;
}

static int guard_actions(manual_case *value, guard_counts *counts) {
    fist_manual_events events = {.drive.refresh_drive_display = true, .refresh_view_display = true};
    for (unsigned action = 0; action <= UINT8_MAX; ++action) {
        if (action == 0 || action == LEFT_ACTION || action == RIGHT_ACTION ||
            action == RAISE_ACTION || action == LOWER_ACTION) {
            continue;
        }
        value->vehicle.manual_input.weapon_action = (uint8_t)action;
        if (rejected(&value->vehicle, &value->input, &value->controls, &events) != 0) {
            return -1;
        }
        ++counts->action;
    }
    value->vehicle.manual_input.weapon_action = 0;
    return 0;
}

static int guard_reference(manual_case *value, guard_counts *counts) {
    static const uint8_t actions[] = {LEFT_ACTION, RIGHT_ACTION, RAISE_ACTION, LOWER_ACTION};
    fist_manual_events events = {.drive.refresh_drive_display = true, .refresh_view_display = true};
    for (unsigned mode = 0; mode < MODE_COUNT; ++mode) {
        value->input.drive_mode = (uint16_t)mode;
        for (unsigned inhibit = 0; inhibit <= 1; ++inhibit) {
            value->vehicle.control_flags = (uint16_t)inhibit;
            for (size_t index = 0; index < sizeof(actions) / sizeof(actions[0]); ++index) {
                value->vehicle.manual_input.weapon_action = actions[index];
                if (rejected(&value->vehicle, &value->input, &value->controls, &events) != 0) {
                    return -1;
                }
                ++counts->target;
            }
            value->vehicle.manual_input.weapon_action = 0;
            if (step_case(value, &value->controls, &events) != 0) {
                return -1;
            }
            ++counts->unused;
        }
    }
    return 0;
}

static int guard_targets(manual_case *value, guard_counts *counts) {
    const fist_object_reference malformed[] = {
        {1, FIST_UNIT_REGISTRY_COUNT}, {0, 1}, {1, UINT16_MAX}};
    for (size_t index = 0; index < sizeof(malformed) / sizeof(malformed[0]); ++index) {
        value->vehicle.command.target = malformed[index];
        if (guard_reference(value, counts) != 0) {
            return -1;
        }
    }
    value->input.selected = false;
    value->input.drive_mode = UINT16_MAX;
    value->vehicle.manual_input.weapon_action = UINT8_MAX;
    value->vehicle.type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    value->vehicle.component_size = 0;
    fist_manual_events events = {.drive.refresh_drive_display = true, .refresh_view_display = true};
    if (step_case(value, &value->controls, &events) != 0 || events.drive.refresh_drive_display ||
        events.refresh_view_display) {
        return -1;
    }
    ++counts->unused;
    return 0;
}

static int rejected_refresh(fist_vehicle_state *vehicle) {
    fist_vehicle_state before;
    if (vehicle != NULL) {
        fist_probe_capture(vehicle, sizeof(before), &before);
    }
    return fist_vehicle_refresh_turret_view(vehicle) == -1 &&
                   (vehicle == NULL || fist_probe_unchanged(vehicle, sizeof(before), &before))
               ? 0
               : -1;
}

static int guard_basic(manual_case *value, guard_counts *counts) {
    fist_manual_events events = {.drive.refresh_drive_display = true, .refresh_view_display = true};
    if (rejected(NULL, &value->input, &value->controls, &events) != 0 ||
        rejected(&value->vehicle, NULL, &value->controls, &events) != 0 ||
        rejected(&value->vehicle, &value->input, NULL, &events) != 0 ||
        rejected(&value->vehicle, &value->input, &value->controls, NULL) != 0) {
        return -1;
    }
    counts->basic += 4;
    --value->vehicle.component_size;
    if (rejected(&value->vehicle, &value->input, &value->controls, &events) != 0 ||
        rejected_refresh(&value->vehicle) != 0) {
        return -1;
    }
    counts->basic += 2;
    ++value->vehicle.component_size;
    value->vehicle.type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    if (rejected(&value->vehicle, &value->input, &value->controls, &events) != 0 ||
        rejected_refresh(&value->vehicle) != 0) {
        return -1;
    }
    counts->basic += 2;
    return 0;
}

static int guards(void) {
    guard_counts counts = {0};
    if (rejected_refresh(NULL) != 0) {
        return -1;
    }
    ++counts.basic;
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        manual_case value = guard_case((uint16_t)type);
        if (guard_drive(&value, &counts) != 0 || guard_actions(&value, &counts) != 0 ||
            guard_basic(&value, &counts) != 0) {
            return -1;
        }
        value = guard_case((uint16_t)type);
        if (guard_targets(&value, &counts) != 0) {
            return -1;
        }
    }
    printf("guards %u %u %u %u %u\n", counts.drive, counts.action, counts.target, counts.unused,
           counts.basic);
    return ferror(stdout) == 0 ? 0 : -1;
}

static manual_case *load_cases(const char *path, size_t *count) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (data == NULL || closed != 0 || size < HEADER_BYTES) {
        free(data);
        return NULL;
    }
    *count = fist_read_u32le(data);
    if ((size - HEADER_BYTES) % CASE_BYTES != 0 || *count != (size - HEADER_BYTES) / CASE_BYTES) {
        free(data);
        return NULL;
    }
    manual_case *cases = calloc(*count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return NULL;
    }
    for (size_t index = 0; index < *count; ++index) {
        if (prepare(data + HEADER_BYTES + (index * CASE_BYTES), &cases[index]) != 0) {
            free(cases);
            cases = NULL;
            break;
        }
    }
    free(data);
    return cases;
}

typedef enum { NORMAL, SHARED, RETENTION } run_mode;

static int run(manual_case *cases, size_t count, run_mode mode, bool output) {
    if (mode == SHARED) {
        return shared_run(cases, count, output);
    }
    return mode == RETENTION ? retention_run(cases, count, output)
                             : normal_run(cases, count, output);
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--guards") == 0) {
        return guards() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    run_mode mode = NORMAL;
    if (argc == 3 && strcmp(argv[2], "--shared") == 0) {
        mode = SHARED;
    } else if (argc == 3 && strcmp(argv[2], "--retention") == 0) {
        mode = RETENTION;
    } else if (argc != 2) {
        return EXIT_FAILURE;
    }
    size_t count = 0;
    manual_case *cases = load_cases(argv[1], &count);
    if (cases == NULL) {
        return EXIT_FAILURE;
    }
    manual_case *checked = calloc(count + 1, sizeof(*checked));
    if (checked == NULL) {
        free(cases);
        return EXIT_FAILURE;
    }
    fist_probe_capture(cases, count * sizeof(*cases), checked);
    int status = run(checked, count, mode, false);
    free(checked);
    if (status == 0) {
        status = run(cases, count, mode, true);
    }
    free(cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
