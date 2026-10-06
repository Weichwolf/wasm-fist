#ifndef FIST_SIM_PROJECTILE_FLIGHT_H
#define FIST_SIM_PROJECTILE_FLIGHT_H

#include "assets/klc.h"
#include "sim/collision.h"
#include "sim/object_pool.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    fist_object_pool *pool;
    fist_collision_world world;
    const fist_klc_image *height;
    fist_random *random;
} fist_projectile_environment;

typedef struct {
    fist_collision_hit hit;
    uint8_t phase;
} fist_projectile_step;

enum {
    FIST_EXPLOSION_SHELL_GROUND = 0,
    FIST_EXPLOSION_SHELL_UNIT = 1,
    FIST_EXPLOSION_VEHICLE_SHOCK = 2,
    FIST_EXPLOSION_VEHICLE_FIRE = 3,
    FIST_EXPLOSION_PAIR_DESTRUCTION = 4,
    FIST_EXPLOSION_TYPE26 = 5,
    FIST_EXPLOSION_TYPE26_WIDE = 6,
    FIST_EXPLOSION_TEMPLATE_COUNT = 7
};

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    /* Type 4 uses the common heading word as its model code. */
    uint16_t model_code;
    uint16_t extent;
    uint16_t projection_scale;
    uint16_t callback_selector;
    uint16_t height_offset;
    uint8_t frame;
    uint8_t last_frame;
    uint8_t period;
    uint8_t countdown;
    uint8_t flags;
} fist_explosion;

typedef struct {
    fist_explosion explosion;
    bool has_explosion;
    /* Original requests; notification/player/device gates are caller-owned. */
    uint8_t notice;
    uint8_t sound_request;
    bool hit_voice;
} fist_projectile_impact;

/* Untargeted M1 type-8 update: age, wrapped XYZ integration, original terrain
 * gate/subtraction, grace, then shared ordered unit query. Miss/origin hit
 * remains FLYING. Expiry releases the current binding and marks deletion.
 * An impact leaves the shell allocated and pending. UNIT_IMPACT hands the
 * actual hit/profile/launch parameter to the damage owner BEFORE finishing.
 * The world must borrow this shell's actual pose/flags/mode, never a copy.
 * Returns 0, or -1 preserving all state/output on invalid input. */
int fist_projectile_advance(fist_projectile *projectile,
                            const fist_projectile_environment *environment,
                            fist_projectile_step *out);

/* Normal-priority creation from the exact original fixed templates. Returns
 * OK, UNAVAILABLE or -1; failure preserves pool/output. No deletion occurs. */
int fist_explosion_create(fist_object_pool *pool, const fist_object_pose *pose, uint8_t template_id,
                          fist_explosion *out);

/* Continue a pending impact AFTER the caller completes unit damage. Allocate
 * the original type-4 template at normal priority BEFORE releasing the shell.
 * Capacity may suppress the explosion, never deletion or the sound request.
 * Notice is the pending phase; hit_voice asks the caller's original rate/player
 * gates to dispatch hit voice. This does not mix audio or implement damage.
 * Returns 0, or -1 preserving pool/projectile/output on invalid input. */
int fist_projectile_finish_impact(fist_object_pool *pool, fist_projectile *projectile,
                                  fist_projectile_impact *out);

/* Original complete type-4/type-18 update and deletion. Return 0 on a valid
 * update, -1 preserving all state on invalid input or an absent live binding.
 * The world scheduler must stop dispatching deleted payloads. */
int fist_explosion_advance(fist_object_pool *pool, fist_explosion *explosion);
int fist_muzzle_smoke_advance(fist_object_pool *pool, fist_muzzle_smoke *muzzle);

#endif
