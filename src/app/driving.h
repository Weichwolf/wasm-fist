#ifndef FIST_APP_DRIVING_H
#define FIST_APP_DRIVING_H

#include "assets/klc.h"
#include "assets/model.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "assets/vehicle.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"

#include <stdint.h>

enum {
    FIST_DRIVE_FASTER = 1,
    FIST_DRIVE_SLOWER = 2,
    FIST_DRIVE_LEFT = 4,
    FIST_DRIVE_RIGHT = 8,
    FIST_DRIVE_TURRET_LEFT = 16,
    FIST_DRIVE_TURRET_RIGHT = 32,
    FIST_DRIVE_THROTTLE_OFF = 64,
    FIST_DRIVE_PAUSE = 128,
    FIST_DRIVE_KEYS = 255
};

typedef struct {
    uint32_t height_side;
    fist_random random;
} fist_driving_options;

typedef struct {
    fist_terrain terrain;
    fist_units units;
    fist_model model;
    fist_vehicle_visual visual;
    fist_klc_image installed_height;
    fist_vehicle_state player;
    uint64_t clock_phase;
    uint64_t ticks;
    uint16_t keys;
    uint8_t paused;
} fist_driving;

typedef struct {
    uint32_t elapsed_us;
    uint16_t keys;
} fist_driving_interval;

/* Load/initialize the roster-zero ground player, models and installed field.
 * Source/scenario are borrowed only during the call. Failure preserves out;
 * success owns all data. Explicit random/detail options choose the start. */
int fist_driving_load(const fist_scenario *scenario, const fist_asset_source *source,
                      const fist_driving_options *options, fist_driving *out);
void fist_driving_destroy(fist_driving *driving);

/* Advance the prior input through the full elapsed interval, then install new
 * keys at its boundary. Integer rational PIT cadence is independent of drawing.
 * Rising pause edges freeze simulation time; paused intervals do not accumulate.
 * Return -1 on invalid state/input, preserving player/clock/keys/pause. */
int fist_driving_advance(fist_driving *driving, fist_driving_interval interval);
/* Canonical ASCII keyboard bindings are shared, including case folding. */
uint16_t fist_driving_key(int key);

#endif
