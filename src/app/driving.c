#include "app/driving.h"

#include "assets/heightfield.h"
#include "assets/klc.h"
#include "assets/model.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "assets/vehicle.h"
#include "sim/driver.h"
#include "sim/ground.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>

void fist_driving_destroy(fist_driving *driving) {
    if (driving == NULL) {
        return;
    }
    fist_terrain_destroy(&driving->terrain);
    fist_units_destroy(&driving->units);
    fist_model_destroy(&driving->model);
    fist_klc_destroy(&driving->installed_height);
    *driving = (fist_driving){0};
}

int fist_driving_load(const fist_scenario *scenario, const fist_asset_source *source,
                      const fist_driving_options *options, fist_driving *out) {
    if (scenario == NULL || source == NULL || options == NULL || out == NULL) {
        return -1;
    }
    fist_driving driving = {0};
    fist_random random = options->random;
    int status = fist_units_decode(scenario, &driving.units);
    const fist_unit_definition *player = fist_units_roster_get(&driving.units, 0);
    if (status == 0) {
        status = fist_vehicle_initialize(player, &random, 0, &driving.player);
    }
    if (status == 0) {
        status = fist_vehicle_visual_decode(player, &driving.visual);
    }
    if (status == 0) {
        status = fist_terrain_load(scenario, source, &driving.terrain);
    }
    if (status == 0) {
        status = fist_heightfield_resample(&driving.terrain.heightmap, options->height_side,
                                           &driving.installed_height);
    }
    if (status == 0) {
        status = fist_vehicle_ground_update(&driving.player, &driving.installed_height);
    }
    if (status == 0) {
        status = fist_model_load(driving.visual.model_name, source, &driving.model);
    }
    if (status != 0) {
        fist_driving_destroy(&driving);
        return -1;
    }
    *out = driving;
    return 0;
}

static int direction(uint16_t keys, uint16_t positive, uint16_t negative) {
    return ((keys & positive) != 0) - ((keys & negative) != 0);
}

int fist_driving_advance(fist_driving *driving, fist_driving_interval interval) {
    const uint16_t keys = interval.keys;
    const uint64_t microseconds = UINT64_C(1000000);
    const uint64_t tick_phase = FIST_DRIVER_TICK_COUNTS * microseconds;
    if (driving == NULL || keys > FIST_DRIVE_KEYS || driving->keys > FIST_DRIVE_KEYS ||
        driving->paused > 1 || driving->clock_phase >= tick_phase ||
        driving->player.type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        driving->player.component_size != fist_vehicle_component_size(driving->player.type)) {
        return -1;
    }
    const fist_driver_controls controls = {
        .throttle_change = (int8_t)direction(driving->keys, FIST_DRIVE_FASTER, FIST_DRIVE_SLOWER),
        .steering = (int8_t)direction(driving->keys, FIST_DRIVE_RIGHT, FIST_DRIVE_LEFT),
        .turret_change =
            (int16_t)(direction(driving->keys, FIST_DRIVE_TURRET_RIGHT, FIST_DRIVE_TURRET_LEFT) *
                      FIST_DRIVER_TURRET_INCREMENT),
        .throttle_off = (uint8_t)((driving->keys & FIST_DRIVE_THROTTLE_OFF) != 0)};
    uint64_t phase = driving->clock_phase;
    uint64_t count = 0;
    if (driving->paused == 0) {
        phase += (uint64_t)interval.elapsed_us * FIST_DRIVER_PIT_RATE;
        count = phase / tick_phase;
        phase %= tick_phase;
    }
    if (driving->ticks > UINT64_MAX - count) {
        return -1;
    }
    fist_vehicle_state player = driving->player;
    for (uint64_t tick = 0; tick < count; ++tick) {
        if (fist_driver_step(&player, &driving->installed_height, &controls) != 0) {
            return -1;
        }
    }
    driving->player = player;
    driving->clock_phase = phase;
    driving->ticks += count;
    if ((keys & FIST_DRIVE_PAUSE) != 0 && (driving->keys & FIST_DRIVE_PAUSE) == 0) {
        driving->paused ^= 1;
    }
    driving->keys = keys;
    return 0;
}

uint16_t fist_driving_key(int key) {
    if (key >= 'a' && key <= 'z') {
        key += 'A' - 'a';
    }
    switch (key) {
    case 'W':
        return FIST_DRIVE_FASTER;
    case 'S':
        return FIST_DRIVE_SLOWER;
    case 'A':
        return FIST_DRIVE_LEFT;
    case 'D':
        return FIST_DRIVE_RIGHT;
    case 'Q':
        return FIST_DRIVE_TURRET_LEFT;
    case 'E':
        return FIST_DRIVE_TURRET_RIGHT;
    case ' ':
        return FIST_DRIVE_THROTTLE_OFF;
    case 'P':
        return FIST_DRIVE_PAUSE;
    default:
        return 0;
    }
}
