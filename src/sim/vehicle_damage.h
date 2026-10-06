#ifndef FIST_SIM_VEHICLE_DAMAGE_H
#define FIST_SIM_VEHICLE_DAMAGE_H

#include "assets/units.h"
#include "sim/collision.h"
#include "sim/object_pool.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    FIST_DAMAGE_SIDES = 2,
    FIST_DAMAGE_COUNTED_PLATOONS = 4,
    FIST_DAMAGE_VOICE_CAPACITY = 3,
    FIST_DAMAGE_EXPLOSIONS = 2,
    FIST_DAMAGE_NO_REQUEST = UINT8_MAX
};

typedef struct {
    /* Original e3ae/e3b0 factors, selected by the PROJECTILE side bit. */
    uint16_t source_scale[FIST_DAMAGE_SIDES];
    uint16_t selected_slot;
    uint16_t roster[FIST_UNIT_ROSTER_COUNT];
    uint16_t platoon_sizes[FIST_DAMAGE_COUNTED_PLATOONS];
    /* Side-bit-clear count is 799e; side-bit-set count is 799a. */
    uint16_t destroyed_by_side[FIST_DAMAGE_SIDES];
    uint16_t clear_side_destroyed_by_clear_source;
    /* Type-5/6 critical census: clear side 79a0, set side 799c. */
    uint16_t pair_destroyed_by_side[FIST_DAMAGE_SIDES];
    uint8_t damage_flash;
} fist_combat_state;

typedef struct {
    fist_object_pool *pool;
    fist_random *random;
    fist_combat_state *state;
} fist_damage_environment;

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t model_code;
    uint16_t original_type;
    uint16_t projection_scale;
    /* Original +21; its later update semantics are separate. */
    uint16_t parameter;
    uint8_t platoon;
    uint8_t member;
    uint8_t flags;
    uint8_t secondary_flags;
} fist_vehicle_wreck;

typedef struct {
    const fist_projectile *projectile;
    fist_collision_hit hit;
} fist_vehicle_damage_request;

typedef struct {
    fist_explosion explosions[FIST_DAMAGE_EXPLOSIONS];
    fist_vehicle_wreck wreck;
    fist_pool_allocation retiring;
    uint8_t explosion_count;
    uint8_t voice_requests[FIST_DAMAGE_VOICE_CAPACITY];
    uint8_t voice_count;
    uint8_t sound_request;
    uint8_t destruction_sound_request;
    uint8_t applied_damage;
    bool destroyed;
    bool has_wreck;
    /* Caller must complete original selected-player loss/UI/takeover flow. */
    bool selected_destroyed;
    bool refresh_damage_display;
} fist_vehicle_damage_result;

/* Complete M1 primary type-8/profile-0/parameter-5 damage to classes 0..3:
 * recovered word-width damage/aspect/source scaling, ordered random reactions,
 * selected-player feedback and critical transition. Destruction creates optional
 * effects/wreck, replaces every roster reference and retypes the retained actor
 * as deleted type 19; original census and four-update release ordering are kept.
 * Voice/sound/UI are explicit requests, not mixed/device output. Caller handles
 * selected-player takeover/UI before resuming its world schedule. The shell is
 * unchanged and allocated until its separate post-damage impact continuation.
 * Returns 0, or -1 preserving every owner/output on invalid or stale input. */
int fist_vehicle_damage_m1(fist_vehicle_state *vehicle, const fist_damage_environment *environment,
                           fist_vehicle_damage_request request, fist_vehicle_damage_result *out);

/* Actual type-19 c0ba countdown and release; the world must stop dispatching
 * after out becomes true. Deleted vehicle storage remains owned until release.
 * Returns 0, or -1 preserving all state/output on invalid/stale input. */
int fist_vehicle_retirement_advance(fist_object_pool *pool, fist_pool_allocation allocation,
                                    fist_vehicle_state *vehicle, bool *out);

#endif
