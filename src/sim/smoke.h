#ifndef FIST_SIM_SMOKE_H
#define FIST_SIM_SMOKE_H

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/world.h"

#include <stdint.h>

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t extent;
    uint16_t projection_scale;
    uint16_t animation_counter;
    uint8_t animation_frame;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
} fist_drifting_smoke;

typedef struct {
    uint16_t extent_base;
    /* Original setting byte: exactly 1 enables smoke. */
    uint8_t enabled;
} fist_smoke_creation;

typedef struct {
    int32_t wind_x;
    int32_t wind_y;
    uint8_t enabled;
} fist_smoke_weather;

/* Restore type-17 fields from an owned complete short snapshot. No borrowed
 * input survives. Returns 0 or -1 preserving output on invalid identity/data. */
int fist_drifting_smoke_restore(const fist_unit_definition *definition,
                                fist_pool_allocation allocation, fist_drifting_smoke *out);

/* Actual 19caa: low-priority admission precedes its sole RNG draw. Disabled or
 * exhausted creation returns UNAVAILABLE preserving pool/random/output. Source
 * XYZ is borrowed; heading and every constructor-only field start at zero.
 * Returns OK, UNAVAILABLE or -1 preserving owners/output on invalid input. */
int fist_drifting_smoke_create(fist_object_pool *pool, fist_random *random,
                               const fist_object_pose *pose, fist_smoke_creation request,
                               fist_drifting_smoke *out);

/* Complete 9b11 wind, low-word-only rise, animation and release. Disabled smoke
 * releases before motion. Current registered identities are required; stale or
 * invalid input returns -1 preserving state/pool. Successful updates return 0.
 * A released allocation must not be scheduled again. */
int fist_drifting_smoke_advance(fist_object_pool *pool, fist_drifting_smoke *smoke,
                                fist_smoke_weather weather);

#endif
