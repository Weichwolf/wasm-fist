#ifndef FIST_SIM_MISSION_UPDATE_H
#define FIST_SIM_MISSION_UPDATE_H

#include "assets/klc.h"
#include "sim/aircraft_death.h"
#include "sim/destruction_updates.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const fist_klc_image *height;
    fist_smoke_weather weather;
    fist_tree_update trees;
    uint16_t tick;
    uint16_t animation_phase;
    bool coarse;
} fist_mission_update_environment;

typedef struct {
    fist_projectile_step flight;
    fist_projectile_impact impact;
    fist_vehicle_damage_result ground_damage;
    fist_other_damage_result other_damage;
    fist_destruction_step destruction;
    fist_aircraft_death_step aircraft;
    uint16_t target_type;
    bool damaged;
    bool has_impact;
    bool player_loss;
} fist_mission_update_result;

/* Consume one current registry visit using canonical world payloads. Rebuild
 * collision views on arrival; ground pose adapters are temporary read-only
 * projections, never retained state. Damage precedes impact allocation/release.
 * Selected fatal damage publishes its effects/roster and suspends all further
 * visits until the caller completes actual player loss/UI/takeover and resumes.
 * Delivered classes: shell 8, effects 4/17/18, retirement 19, trees 21, wreck 23,
 * destroyed 26, all 27 and aircraft 5/6 behavior 12. Living ground/other aircraft/
 * alive 26 return UNSUPPORTED, never an empty successful method. Caller owns
 * current-entry traversal, clock, combat configuration, tree producer and PCM.
 * Returns 0, UNSUPPORTED, or -1; failures preserve complete world/output. */
int fist_mission_world_visit(fist_mission_world *world, fist_pool_allocation allocation,
                             const fist_mission_update_environment *environment,
                             fist_mission_update_result *out);

/* Explicit continuation AFTER the selected-player loss consumer. Never repeat
 * target damage. Finish the suspended canonical shell and publish its effect
 * before clearing the handoff. Returns 0 or -1 preserving world/output on
 * invalid input. This acknowledgement does not implement UI or takeover. */
int fist_mission_world_resume_impact(fist_mission_world *world, fist_projectile_impact *out);

#endif
