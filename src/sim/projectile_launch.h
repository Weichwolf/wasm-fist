#ifndef FIST_SIM_PROJECTILE_LAUNCH_H
#define FIST_SIM_PROJECTILE_LAUNCH_H

#include "sim/object_pool.h"
#include "sim/rotation.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    FIST_LAUNCH_FIRED = 0,
    FIST_LAUNCH_EMPTY = 1,
    FIST_LAUNCH_CAPACITY = 2,
    FIST_LAUNCH_NO_SOUND = UINT8_MAX
};

enum {
    FIST_PROJECTILE_FLYING = 0,
    FIST_PROJECTILE_GROUND_IMPACT = 1,
    FIST_PROJECTILE_UNIT_IMPACT = 2,
    FIST_PROJECTILE_RETIRED = 3
};

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    fist_spatial_velocity velocity;
    int16_t speed;
    uint16_t collision_grace;
    uint16_t age;
    /* Physical actor slot, independent of overwritten registry bindings. */
    uint16_t origin_slot;
    uint16_t target_slot;
    int32_t target_height_offset;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
    uint8_t mode;
    /* Owned update protocol; no additional original record field. */
    uint8_t phase;
    uint8_t collision_profile;
    /* Original +2a is initialized to 5. Its flight meaning is still open. */
    uint8_t launch_parameter;
} fist_projectile;

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t projection_scale;
    uint16_t animation_counter;
    uint8_t animation_frame;
    uint8_t flags;
} fist_muzzle_smoke;

typedef struct {
    uint16_t origin_slot;
    bool coarse;
} fist_launch_request;

typedef struct {
    fist_projectile projectile;
    fist_muzzle_smoke muzzle;
    uint8_t outcome;
    bool has_muzzle;
    /* Original sound-dispatch request, not an audible playback decision. */
    uint8_t sound_request;
} fist_launch_result;

/* Complete already-eligible, untargeted M1 station-0 launch transaction.
 * The caller owns eligibility/targets and transfers returned initialized
 * payloads to its world at their allocated slots. No input binding or flight
 * is implied. Empty ammo preserves vehicle/pool; capacity consumes one round
 * and marks its component, as the original does. Missing low-priority smoke
 * still succeeds. Returns 0 for a valid transaction, -1 for invalid arguments
 * or metadata, preserving vehicle, pool and output on invalid input. */
int fist_m1_launch_untargeted(fist_object_pool *pool, fist_vehicle_state *vehicle,
                              fist_launch_request request, fist_launch_result *out);

#endif
