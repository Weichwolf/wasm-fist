#ifndef FIST_SIM_MISSION_WORLD_H
#define FIST_SIM_MISSION_WORLD_H

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_flight.h"
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
 * Primary launch installs dynamic shell 8 and muzzle 18. Canonical combat visits
 * publish explosion 4 and consume delivered death/effect/retirement classes.
 * Saved restoration of dynamic classes and living dispatch remain separate. */
typedef union {
    fist_vehicle_state vehicle;
    fist_other_actor other;
    fist_drifting_smoke smoke;
    fist_tree tree;
    fist_vehicle_wreck wreck;
    fist_projectile projectile;
    fist_muzzle_smoke muzzle;
    fist_explosion explosion;
} fist_mission_object;

typedef struct {
    fist_object_pool pool;
    fist_random random;
    /* Canonical mission order owner. Saved-object-only installation leaves this
     * explicitly unloaded; command dispatch requires complete mission input. */
    fist_mission_orders orders;
    uint8_t orders_loaded;
    fist_mission_object objects[FIST_UNIT_REGISTRY_COUNT];
    /* Owns the sole physical roster, including overwritten registry orphans.
     * Reset/import do not configure mission combat factors/census/player UI. */
    fist_combat_state combat;
    /* Scheduler handoff after selected fatal damage; not an original record. */
    uint16_t pending_player_impact;
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

/* Complete original ab82 mode selection. Uses the installed platoon descriptor
 * and canonical RNG; phase_random is the caller's already consumed ab03 value.
 * Does not run the parent phase, advance counters, resolve targets or dispatch
 * subsequent commands. Reject a used descriptor selector outside the original
 * UI's four choices; unused retained words do not restrict earlier branches.
 * Returns 0 or -1; every failure preserves the complete world. */
typedef struct {
    uint16_t slot;
    uint16_t phase_random;
} fist_command_selection;

int fist_mission_world_select_command(fist_mission_world *world, fist_command_selection request);

/* Complete ac75 goal assignment for all eight mode entries: first waypoint or
 * automatic formation offset from the physical platoon leader; the other six
 * original entries are genuine returns. Uses fine shared rotation and retained
 * leader heading average. Does not sample headings, advance routes, resolve
 * targets or consume RNG. Invalid used selectors/roster metadata fail without
 * changing any world state. */
int fist_mission_world_assign_command_goal(fist_mission_world *world, uint16_t slot);

#endif
