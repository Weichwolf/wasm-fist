#include "sim/weapon_control.h"

#include "assets/units.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>

enum {
    STATION_STEP = 2,
    LAST_CYCLE_STATION = 6,
    STATION_COUNT = FIST_WEAPON_MAX_STATIONS,
    REFRESH_COMPONENTS = 6,
    COMPONENT_REFRESH = 3,
    FIRE_REQUEST_TICKS = 48,
    ELEVATION_SHIFT = 8,
    EMPTY_CUE = 13,
    RACK_NOTICE = 25,
    RACK_NOTICE_TICKS = 90
};

typedef struct {
    uint8_t station_count;
    uint8_t continuous_station;
    uint8_t stock_station;
    uint8_t refresh_count;
    uint8_t refresh[REFRESH_COMPONENTS];
    uint8_t reload_component;
    uint8_t reload_times[STATION_COUNT];
    uint8_t selection_cues[STATION_COUNT];
    uint8_t completion_cues[STATION_COUNT];
} weapon_profile;

/* Actual DGROUP tables and complete 7963/17fbc/18b7c/1964e setters.
 * Historical reconstruction labels misidentify T80 and BMP as other classes. */
static const weapon_profile profiles[FIST_UNIT_GROUND_VEHICLE_COUNT] = {
    {.station_count = 4,
     .continuous_station = 4,
     .stock_station = FIST_WEAPON_NO_REQUEST,
     .refresh_count = 5,
     .refresh = {28, 29, 31, 33, 35},
     .reload_component = 28,
     .reload_times = {20, 20, 0, 20},
     .selection_cues = {0, 1, 255, 2},
     .completion_cues = {9, 10, 255, 11}},
    {.station_count = 4,
     .continuous_station = 6,
     .stock_station = 0,
     .refresh_count = 5,
     .refresh = {17, 18, 20, 22, 24},
     .reload_component = 17,
     .reload_times = {2, 2, 2, 0},
     .selection_cues = {5, 7, 6, 255}},
    {.station_count = 5,
     .continuous_station = 6,
     .stock_station = FIST_WEAPON_NO_REQUEST,
     .refresh_count = 6,
     .refresh = {20, 22, 24, 26, 28, 17},
     .reload_component = 17,
     .reload_times = {20, 20, 40, 0, 8},
     .selection_cues = {2, 0, 1, 255, 255},
     .completion_cues = {11, 9, 10, 255, 255}},
    {.station_count = 4,
     .continuous_station = 6,
     .stock_station = 4,
     .refresh_count = 5,
     .refresh = {28, 30, 32, 34, 27},
     .reload_component = 27,
     .reload_times = {2, 2, 2, 0},
     .selection_cues = {6, 7, 5, 255}}};

static int valid_vehicle(const fist_vehicle_state *vehicle) {
    return vehicle != NULL && vehicle->type < FIST_UNIT_GROUND_VEHICLE_COUNT &&
           vehicle->component_size == fist_vehicle_component_size(vehicle->type);
}

static fist_weapon_events no_events(void) {
    return (fist_weapon_events){.voice_request = FIST_WEAPON_NO_REQUEST,
                                .notice_request = FIST_WEAPON_NO_REQUEST};
}

static uint16_t ammunition(const fist_vehicle_state *vehicle, size_t slot) {
    /* Only the fifth T80 station uses the separate initialized word +b4. */
    return slot < FIST_VEHICLE_WEAPON_SLOTS ? vehicle->weapons.rounds[slot]
                                            : vehicle->weapons.class_parameter;
}

int fist_weapon_inspect(const fist_vehicle_state *vehicle, fist_weapon_status *out) {
    if (!valid_vehicle(vehicle) || out == NULL) {
        return -1;
    }
    const weapon_profile *profile = &profiles[vehicle->type];
    const uint8_t selected = vehicle->weapons.selected;
    if (selected % STATION_STEP != 0 || selected / STATION_STEP >= profile->station_count) {
        return -1;
    }
    *out = (fist_weapon_status){.ammunition = ammunition(vehicle, selected / STATION_STEP),
                                .station_count = profile->station_count,
                                .selected = selected,
                                .countdown = vehicle->reload_countdown,
                                .continuous = selected == profile->continuous_station,
                                .reserve = vehicle->weapons.ready_stock,
                                .has_reserve = profile->stock_station != FIST_WEAPON_NO_REQUEST};
    return 0;
}

static void start_reload(fist_vehicle_state *vehicle, uint8_t station, fist_weapon_events *events) {
    const weapon_profile *profile = &profiles[vehicle->type];
    const size_t slot = station / STATION_STEP;
    vehicle->weapons.loaded = station;
    vehicle->reload_countdown = profile->reload_times[slot];
    events->voice_request = profile->selection_cues[slot];
    if (ammunition(vehicle, slot) != 0) {
        return;
    }
    if (vehicle->type == 0) {
        vehicle->reload_countdown = UINT8_MAX;
        events->voice_request = EMPTY_CUE;
    } else if (station == profile->stock_station) {
        if (vehicle->weapons.ready_stock == 0) {
            events->voice_request = EMPTY_CUE;
        } else {
            events->notice_request = RACK_NOTICE;
            events->notice_ticks = RACK_NOTICE_TICKS;
        }
    }
}

int fist_weapon_select(fist_vehicle_state *vehicle, uint8_t station, fist_weapon_events *events) {
    if (!valid_vehicle(vehicle) || events == NULL || station % STATION_STEP != 0 ||
        station / STATION_STEP >= profiles[vehicle->type].station_count) {
        return -1;
    }
    fist_weapon_events emitted = no_events();
    if (vehicle->weapons.selected != station) {
        const weapon_profile *profile = &profiles[vehicle->type];
        vehicle->weapons.selected = station;
        if (station != profile->continuous_station && station != vehicle->weapons.loaded) {
            start_reload(vehicle, station, &emitted);
        }
        for (size_t index = 0; index < profile->refresh_count; ++index) {
            vehicle->components[profile->refresh[index]] = COMPONENT_REFRESH;
        }
        emitted.station_changed = 1;
    }
    *events = emitted;
    return 0;
}

int fist_weapon_cycle(fist_vehicle_state *vehicle, fist_weapon_events *events) {
    if (!valid_vehicle(vehicle) || events == NULL) {
        return -1;
    }
    const uint8_t advanced = (uint8_t)(vehicle->weapons.selected + STATION_STEP);
    const uint8_t station = advanced <= LAST_CYCLE_STATION ? advanced : 0;
    return fist_weapon_select(vehicle, station, events);
}

int fist_weapon_request_fire(fist_vehicle_state *vehicle) {
    if (!valid_vehicle(vehicle)) {
        return -1;
    }
    vehicle->weapons.trigger = FIRE_REQUEST_TICKS;
    return 0;
}

int fist_weapon_begin_tick(fist_vehicle_state *vehicle) {
    if (!valid_vehicle(vehicle)) {
        return -1;
    }
    vehicle->turret.elevation_frame =
        (uint8_t)((uint16_t)vehicle->turret.elevation >> ELEVATION_SHIFT);
    if (vehicle->weapons.recoil != 0) {
        --vehicle->weapons.recoil;
    }
    return 0;
}

int fist_weapon_reload_phase(fist_vehicle_state *vehicle, fist_weapon_events *events) {
    if (!valid_vehicle(vehicle) || events == NULL) {
        return -1;
    }
    const weapon_profile *profile = &profiles[vehicle->type];
    const uint8_t selected = vehicle->weapons.selected;
    const int checks_ammunition = vehicle->type == 0 || vehicle->type == 2;
    if (checks_ammunition != 0 &&
        (selected % STATION_STEP != 0 || selected / STATION_STEP >= profile->station_count)) {
        return -1;
    }
    fist_weapon_events emitted = no_events();
    if ((vehicle->drive.update_phase & FIST_VEHICLE_PHASE_MASK) == 0 &&
        vehicle->reload_countdown != 0) {
        --vehicle->reload_countdown;
        if (vehicle->reload_countdown == 0) {
            emitted.timer_expired = 1;
            if (checks_ammunition == 0 || ammunition(vehicle, selected / STATION_STEP) != 0) {
                vehicle->components[profile->reload_component] = COMPONENT_REFRESH;
                if (checks_ammunition != 0) {
                    emitted.voice_request = profile->completion_cues[selected / STATION_STEP];
                }
            }
        }
    }
    *events = emitted;
    return 0;
}
