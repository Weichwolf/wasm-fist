#ifndef FIST_SIM_TREE_H
#define FIST_SIM_TREE_H

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/world.h"

#include <stdint.h>

typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t extent;
    uint16_t projection_scale;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
    uint8_t variant;
} fist_tree;

typedef struct {
    uint8_t changed;
    uint8_t variant;
} fist_tree_update;

/* Restore complete modeled type-21 fields without borrowed storage. This is
 * saved-state restoration, not the separate random 9c1c tree constructor. */
int fist_tree_restore(const fist_unit_definition *definition, fist_pool_allocation allocation,
                      fist_tree *out);

/* Complete 9c4f. Only changed == 1 copies the supplied byte variant. The shared
 * global producer 9c5d and rendering are separate owners. Invalid input leaves
 * the tree unchanged. No invented variant clamp or flag-based early return. */
int fist_tree_advance(const fist_object_pool *pool, fist_tree *tree, fist_tree_update update);

#endif
