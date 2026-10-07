#ifndef FIST_SIM_MISSION_WORLD_H
#define FIST_SIM_MISSION_WORLD_H

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"

#include <stdint.h>

enum { FIST_MISSION_UNSUPPORTED = 2 };

/* The existing pool's physical type is the payload tag. Ground 0..3, other
 * 5/6/26/27, smoke 17, tree 21 and wreck 23 have delivered typed restoration.
 * Primary launch also installs dynamic shell 8 and muzzle 18. Their flight,
 * damage/effect/retirement dispatcher remains a later consumer. */
typedef union {
    fist_vehicle_state vehicle;
    fist_other_actor other;
    fist_drifting_smoke smoke;
    fist_tree tree;
    fist_vehicle_wreck wreck;
    fist_projectile projectile;
    fist_muzzle_smoke muzzle;
} fist_mission_object;

typedef struct {
    fist_object_pool pool;
    fist_random random;
    fist_mission_object objects[FIST_UNIT_REGISTRY_COUNT];
    /* Physical slots, including possible overwritten registry orphans. */
    uint16_t roster[FIST_UNIT_ROSTER_COUNT];
} fist_mission_world;

void fist_mission_world_reset(fist_mission_world *world);

/* Complete normal-side d84a/43c1 saved-object installation for delivered
 * classes, using decoded immutable definitions. Initialize only participating
 * ground actors, in input order; retain final RNG and all physical orphans.
 * Input units/random are borrowed; no input views survive. Units stay unchanged.
 * Random stays unchanged unless it aliases out->random; that supported reload
 * replaces the old world using its current RNG as the new initial state.
 * Returns OK, UNAVAILABLE for physical exhaustion, UNSUPPORTED for an undelivered
 * type, or -1 for invalid data/allocation. Every failure preserves out. This
 * owns neither terrain/contact installation nor living class dispatch/devices. */
int fist_mission_world_initialize(const fist_units *units, const fist_random *random,
                                  uint8_t link_mode, fist_mission_world *out);

/* Borrow a physical payload, including an orphan, or NULL for invalid/unused
 * slots. The allocation owner remains the authority for current bindings. */
const fist_mission_object *fist_mission_world_object(const fist_mission_world *world,
                                                     uint16_t slot);

/* Consume the canonical physical M1 and publish complete returned shell/muzzle
 * payloads before the next world visit. Shared history belongs to the mission
 * caller; the pool remains the identity/occupancy authority. Returns 0 or -1
 * preserving world/history/output on invalid input. Does not dispatch flight,
 * damage, class updates, input devices or audible requests. */
int fist_mission_world_fire_untargeted(fist_mission_world *world, fist_fire_history *history,
                                       fist_fire_request request, fist_fire_result *out);

#endif
