#ifndef FIST_SIM_COLLISION_H
#define FIST_SIM_COLLISION_H

#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    /* Borrow the owning live payload's pose; this view owns no simulation state. */
    const fist_object_pose *pose;
    uint16_t projection_scale;
    uint8_t flags;
    /* Original +19: motion flags, animation or subtype according to class. */
    uint8_t mode;
} fist_collision_body;

typedef struct {
    const fist_object_pool *pool;
    /* Flat physical-slot views, including orphaned imports. */
    const fist_collision_body *bodies;
    size_t body_count;
} fist_collision_world;

typedef struct {
    uint16_t slot;
    uint16_t registry_index;
    uint16_t value;
    uint8_t aspect;
} fist_collision_hit;

/* Complete original bb1b unit-interaction query. Walk current registry bindings
 * in order, excluding self and bodies without flag 40; test inclusive wrapped
 * XY bounds (scale+256 modulo a word), then the actual per-type vertical or
 * probabilistic rule. Types 5/6 consume shared random streams in encounter
 * order even on a failed vertical test. First hit wins, including the origin
 * actor; the flight owner decides whether that hit is its origin and ignores it.
 * Return 0 with NO_SLOT/NO_SLOT/value zero on a valid miss, -1 on invalid input
 * or world, preserving random and output. Body/pool state is never modified. */
int fist_collision_find(const fist_collision_world *world, uint16_t source_slot,
                        fist_random *random, fist_collision_hit *out);

#endif
