#ifndef FIST_SIM_PRIMARY_FIRE_H
#define FIST_SIM_PRIMARY_FIRE_H

#include "sim/object_pool.h"
#include "sim/projectile_launch.h"
#include "sim/vehicle_state.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    /* Shared original 9fce; not a per-actor cooldown. */
    uint16_t failed_at;
} fist_fire_history;

typedef struct {
    fist_launch_request launch;
    uint16_t tick;
} fist_fire_request;

typedef struct {
    fist_launch_result launch;
    bool requested;
    bool dispatched;
    /* All four weapon plates: original globals 8e62/66/6a/6e become 3. */
    bool weapon_panel_refresh;
    /* Candidate at bf3c, before selected-player/side/device/timing gates. */
    uint8_t voice_request;
} fist_fire_result;

/* Original 7c65..7c7b and complete untargeted station-zero 7e29. The caller
 * supplies the already-updated class state and current world tick. This stage
 * advances neither recoil, reload, motion nor class phase. Decrement pending
 * before dispatch; automatic secondary bit 80 also requests it. Reload blocks
 * after the actual weapon-panel request and component mark. Empty/capacity attempts
 * retain original consumption and shared failure cooldown semantics. Returned
 * launch payloads must be installed by the world owner before further visits.
 * Only untargeted M1 selected station zero is delivered by this API. Returns
 * 0 or -1 preserving all owners/output on invalid input. No device/PCM call. */
int fist_m1_fire_untargeted(fist_object_pool *pool, fist_vehicle_state *vehicle,
                            fist_fire_history *history, fist_fire_request request,
                            fist_fire_result *out);

#endif
