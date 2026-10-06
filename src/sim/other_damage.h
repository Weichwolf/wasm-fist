#ifndef FIST_SIM_OTHER_DAMAGE_H
#define FIST_SIM_OTHER_DAMAGE_H

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/projectile_flight.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t animation_parameter;
    uint8_t behavior;
    uint8_t animation_frame;
    uint8_t damage;
} fist_pair_actor_state;

typedef struct {
    uint8_t damage;
    uint8_t limit;
    /* Original unsigned +1c destruction word; later updates are separate. */
    uint16_t destruction_parameter;
    uint8_t emission_counter;
} fist_type26_state;

typedef struct {
    uint8_t damage;
    uint16_t debris_parameter;
    uint16_t animation_counter;
    uint16_t emission_counter;
} fist_type27_state;

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t projection_extent;
    uint16_t projection_scale;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
    uint8_t mode;
    /* The allocation type is the tag: 5/6, 26 or 27. */
    union {
        fist_pair_actor_state pair;
        fist_type26_state type26;
        fist_type27_state type27;
    } state;
} fist_other_actor;

typedef struct {
    fist_explosion explosion;
    uint8_t applied_damage;
    uint8_t sound_request;
    uint8_t voice_request;
    bool has_explosion;
    bool destroyed;
    bool released;
    bool refresh_damage_display;
} fist_other_damage_result;

/* Restore modeled fields from a complete original short snapshot. Borrowed
 * bytes do not survive the call. This does not run class initialization or AI.
 * Returns 0, or -1 preserving out on invalid definition/allocation/subtype. */
int fist_other_actor_restore(const fist_unit_definition *definition,
                             fist_pool_allocation allocation, fist_other_actor *out);

/* Complete M1 primary damage to collision-reachable 5/6/26/27. Source remains
 * unchanged/allocated until impact continuation. Critical 5/6 updates census
 * then chooses immediate normal effect/release or retained death animation;
 * 26/27 retain bindings in their actual destroyed state for later class updates.
 * These later updates, AI, devices, drawing and PCM are separate consumers.
 * Voice/sound/display are producer requests. Returns 0, or -1 preserving all
 * owners/output on invalid/stale/profile/type/owner input. */
int fist_other_damage_m1(fist_other_actor *actor, const fist_damage_environment *environment,
                         fist_vehicle_damage_request request, fist_other_damage_result *out);

/* Actual type-23 c335 no-op followed by c31e/60f4 display refresh. Does not
 * copy or modify the independently owned wreck payload. Same source/identity
 * validation and invalid-input atomicity as the other damage entry. */
int fist_wreck_damage_m1(const fist_damage_environment *environment,
                         fist_vehicle_damage_request request, fist_other_damage_result *out);

#endif
