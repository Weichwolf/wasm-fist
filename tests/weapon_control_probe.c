#include "assets/bytes.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER_BYTES = 4,
    CASE_BYTES = FIST_UNIT_EXTENDED_SIZE + 4,
    SELECT = 0,
    CYCLE = 1,
    REQUEST = 2,
    BEGIN = 3,
    RELOAD = 4,
    TICK = 5,
    PHASE_STEP = 2,
    MARKER = 123,
    LAST_STATION = 6,
    T80_LAST_STATION = 8
};

typedef struct {
    fist_vehicle_state state;
    uint16_t steps;
    uint8_t operation;
    uint8_t argument;
} weapon_case;

static int prepare(const uint8_t *raw, weapon_case *out) {
    enum {
        MAP_X = 4,
        MAP_Y = 8,
        ALTITUDE = 12,
        RELOAD_COUNT = 168,
        READY_STOCK = 187,
        AMMO_OFFSET = 173,
        T80_AMMO_OFFSET = 172
    };
    static const size_t parameters[] = {181, 250, 180, 249};
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    fist_random random = {0};
    if (fist_vehicle_initialize(&definition, &random, 0, &out->state) != 0) {
        return -1;
    }
    fist_vehicle_state *state = &out->state;
    const size_t ammo_offset = state->type == 2 ? T80_AMMO_OFFSET : AMMO_OFFSET;
    for (size_t slot = 0; slot < FIST_VEHICLE_WEAPON_SLOTS; ++slot) {
        state->weapons.rounds[slot] =
            fist_read_u16le(raw + ammo_offset + (slot * sizeof(uint16_t)));
    }
    state->weapons.class_parameter = state->type == 2
                                         ? fist_read_u16le(raw + parameters[state->type])
                                         : raw[parameters[state->type]];
    if (state->type == 1 || state->type == 3) {
        state->weapons.ready_stock = raw[READY_STOCK];
    }
    state->reload_countdown = raw[RELOAD_COUNT];
    for (size_t index = 0; index < state->component_size; ++index) {
        state->components[index] = raw[fist_probe_component_offset(state->type) + index];
    }
    out->steps = fist_read_u16le(raw + FIST_UNIT_EXTENDED_SIZE);
    out->operation = raw[FIST_UNIT_EXTENDED_SIZE + 2];
    out->argument = raw[FIST_UNIT_EXTENDED_SIZE + 3];
    return out->steps != 0 && out->operation <= TICK &&
                   (out->operation == SELECT || out->argument == 0)
               ? 0
               : -1;
}

static int step_case(weapon_case *value, fist_weapon_events *events) {
    fist_vehicle_state *state = &value->state;
    *events = (fist_weapon_events){.voice_request = FIST_WEAPON_NO_REQUEST,
                                   .notice_request = FIST_WEAPON_NO_REQUEST};
    switch (value->operation) {
    case SELECT:
        return fist_weapon_select(state, value->argument, events);
    case CYCLE:
        return fist_weapon_cycle(state, events);
    case REQUEST:
        return fist_weapon_request_fire(state);
    case BEGIN:
        return fist_weapon_begin_tick(state);
    case RELOAD:
        return fist_weapon_reload_phase(state, events);
    case TICK:
        if (fist_weapon_begin_tick(state) != 0) {
            return -1;
        }
        state->drive.update_phase = (uint8_t)(state->drive.update_phase + PHASE_STEP);
        return fist_weapon_reload_phase(state, events);
    default:
        return -1;
    }
}

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

static int reject_invalid(fist_vehicle_state *state, const fist_vehicle_state *saved) {
    const fist_weapon_events saved_events = {MARKER, MARKER, MARKER, MARKER, MARKER};
    fist_weapon_events events = saved_events;
    return fist_weapon_select(state, 0, &events) == -1 && fist_weapon_cycle(state, &events) == -1 &&
                   fist_weapon_request_fire(state) == -1 && fist_weapon_begin_tick(state) == -1 &&
                   fist_weapon_reload_phase(state, &events) == -1 &&
                   same_bytes(&events, sizeof(events), &saved_events) &&
                   (state == NULL || same_bytes(state, sizeof(*state), saved))
               ? 0
               : -1;
}

static int invalid_station_codes(fist_vehicle_state *state, const fist_vehicle_state *saved) {
    const unsigned last = state->type == 2 ? T80_LAST_STATION : LAST_STATION;
    const fist_weapon_events saved_events = {MARKER, MARKER, MARKER, MARKER, MARKER};
    fist_weapon_events events = saved_events;
    for (unsigned station = 0; station <= UINT8_MAX; ++station) {
        if (station % 2 == 0 && station <= last) {
            continue;
        }
        if (fist_weapon_select(state, (uint8_t)station, &events) != -1 ||
            !same_bytes(state, sizeof(*state), saved) ||
            !same_bytes(&events, sizeof(events), &saved_events)) {
            return -1;
        }
    }
    return 0;
}

static int invalid_inputs(void) {
    fist_vehicle_state *state = calloc(1, sizeof(*state));
    fist_vehicle_state *saved = calloc(1, sizeof(*saved));
    if (state == NULL || saved == NULL) {
        free(state);
        free(saved);
        return -1;
    }
    int result = reject_invalid(NULL, saved);
    state->type = FIST_UNIT_GROUND_VEHICLE_COUNT;
    *saved = *state;
    if (reject_invalid(state, saved) != 0) {
        result = -1;
    }
    for (unsigned type = 0; type < FIST_UNIT_GROUND_VEHICLE_COUNT; ++type) {
        state->type = (uint16_t)type;
        state->component_size = 0;
        *saved = *state;
        if (reject_invalid(state, saved) != 0) {
            result = -1;
        }
        state->component_size = fist_vehicle_component_size(state->type);
        *saved = *state;
        if (fist_weapon_select(state, 0, NULL) != -1 || fist_weapon_cycle(state, NULL) != -1 ||
            fist_weapon_reload_phase(state, NULL) != -1 ||
            !same_bytes(state, sizeof(*state), saved) || invalid_station_codes(state, saved) != 0) {
            result = -1;
        }
    }
    free(state);
    free(saved);
    return result;
}

static int run_cases(const uint8_t *data, size_t count, weapon_case *cases) {
    for (size_t index = 0; index < count; ++index) {
        if (prepare(data + HEADER_BYTES + (index * CASE_BYTES), &cases[index]) != 0) {
            return -1;
        }
    }
    /* Validate every complete operation sequence before publishing observations. */
    for (size_t index = 0; index < count; ++index) {
        weapon_case checked = cases[index];
        for (unsigned step = 0; step < checked.steps; ++step) {
            fist_weapon_events events = {0};
            if (step_case(&checked, &events) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int observations(weapon_case *cases, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        for (unsigned step = 0; step < cases[index].steps; ++step) {
            fist_weapon_events events = {0};
            if (step_case(&cases[index], &events) != 0) {
                return -1;
            }
            fist_probe_write_vehicle_state(&cases[index].state);
            printf("weapon_events %u %u %u %u %u\n", (unsigned)events.station_changed,
                   (unsigned)events.timer_expired, (unsigned)events.voice_request,
                   (unsigned)events.notice_request, (unsigned)events.notice_ticks);
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
    weapon_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return EXIT_FAILURE;
    }
    int result = run_cases(data, count, cases);
    free(data); /* No original snapshot/request buffer survives observations. */
    if (result == 0) {
        result = observations(cases, count);
    }
    free(cases);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
