#ifndef FIST_APP_DRIVING_H
#define FIST_APP_DRIVING_H

#include "assets/klc.h"
#include "assets/model.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "assets/vehicle.h"
#include "sim/mission_world.h"
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
    FIST_DRIVE_WEAPON_1 = 256,
    FIST_DRIVE_WEAPON_2 = 512,
    FIST_DRIVE_WEAPON_3 = 1024,
    FIST_DRIVE_WEAPON_4 = 2048,
    FIST_DRIVE_WEAPON_5 = 4096,
    FIST_DRIVE_NEXT_WEAPON = 8192,
    FIST_DRIVE_KEYS = 16383
};

typedef struct {
    uint64_t selections;
    uint64_t reloads;
    uint64_t voice_requests;
    uint64_t notice_deadline;
    /* Feedback/verification of requests. This is not an audio playback queue. */
    uint8_t voice_request;
    uint8_t notice;
} fist_driving_feedback;

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
    /* Exactly one owned heap state: diagnostic player or complete mission.
     * Mission selection belongs to world->combat.selected_slot. */
    fist_vehicle_state *preview_player;
    fist_mission_world *world;
    fist_driving_feedback feedback;
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
/* Explicit complete supported-world load. Reuse the existing controller stages
 * against the canonical selected actor; no standalone fallback or full class
 * tick/AI/fire/device claim. Returns 0, UNAVAILABLE, UNSUPPORTED or -1; failures
 * preserve out. Source/options stay unchanged. */
int fist_driving_load_mission(const fist_scenario *scenario, const fist_asset_source *source,
                              const fist_driving_options *options, fist_driving *out);
/* Borrow the sole active player state, or NULL for missing/invalid selection. */
const fist_vehicle_state *fist_driving_player(const fist_driving *driving);
void fist_driving_destroy(fist_driving *driving);

/* Advance the prior input through the full elapsed interval, then install new
 * keys at its boundary. Integer rational PIT cadence is independent of drawing.
 * Rising pause edges freeze simulation time; paused intervals do not accumulate.
 * Weapon press edges select/cycle at the boundary only while unpaused.
 * Invalid state/input preserves player, feedback, clock, keys and pause. */
int fist_driving_advance(fist_driving *driving, fist_driving_interval interval);
/* Canonical ASCII keyboard bindings are shared, including case folding. */
uint16_t fist_driving_key(int key);

#endif
