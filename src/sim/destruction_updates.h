#ifndef FIST_SIM_DESTRUCTION_UPDATES_H
#define FIST_SIM_DESTRUCTION_UPDATES_H

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/vehicle_damage.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    fist_object_pool *pool;
    fist_random *random;
    uint8_t smoke_enabled;
} fist_destruction_environment;

typedef struct {
    fist_drifting_smoke smoke;
    bool has_smoke;
} fist_destruction_step;

int fist_vehicle_wreck_restore(const fist_unit_definition *definition,
                               fist_pool_allocation allocation, fist_vehicle_wreck *out);

/* Complete bc0c: persistent/collidable wreck, modulo-word counter and periodic
 * smoke. No parameter decrement or fabricated wreck retirement. */
int fist_vehicle_wreck_advance(fist_vehicle_wreck *wreck,
                               const fist_destruction_environment *environment,
                               fist_destruction_step *out);

/* Complete b355 for type 27 and bc46 for destroyed type-26 modes 4..7 only.
 * Alive type-26 firing and aircraft 5/6 updates are separate required methods.
 * Smoke allocation failure/disable never suppresses the original parameter
 * decrements. Returns 0 or -1 preserving all owners/output on invalid input. */
int fist_destroyed_target_advance(fist_other_actor *actor,
                                  const fist_destruction_environment *environment,
                                  fist_destruction_step *out);

#endif
