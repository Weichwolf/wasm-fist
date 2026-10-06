#ifndef FIST_SIM_AIRCRAFT_DEATH_H
#define FIST_SIM_AIRCRAFT_DEATH_H

#include "assets/klc.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/random.h"
#include "sim/smoke.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    fist_object_pool *pool;
    fist_random *random;
    const fist_klc_image *height;
    /* Original 6cde tick and independent 6d14 rotor animation phase. */
    uint16_t tick;
    uint16_t animation_phase;
    uint8_t smoke_enabled;
    bool coarse;
} fist_aircraft_environment;

typedef struct {
    fist_explosion explosion;
    fist_drifting_smoke smoke;
    bool has_explosion;
    bool has_smoke;
    bool released;
    uint8_t sound_request;
} fist_aircraft_death_step;

/* Complete 9e2b for retained type-5/6 behavior 12, including a03f and its
 * post-release emission/movement tail. Other aircraft behaviors/AI remain
 * separate required methods. Captured emitter pose survives slot reuse,
 * repairing the original's constructor-before-source-read lifetime defect.
 * Caller discards a released actor before installing returned effects/smoke.
 * If new smoke reuses its slot, actor retains its last state before replacement;
 * the tail reads the new constructor's zero motion fields instead.
 * Returns 0, or -1 preserving all owners/output on invalid/stale input. */
int fist_aircraft_death_advance(fist_other_actor *actor,
                                const fist_aircraft_environment *environment,
                                fist_aircraft_death_step *out);

#endif
