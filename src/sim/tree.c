#include "sim/tree.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/object_pool.h"

#include <stddef.h>
#include <stdint.h>

enum {
    TREE = 21,
    EXTENT = 18,
    SCALE = 20,
    FLAGS = 22,
    SECONDARY = 23,
    GROUND = 24,
    VARIANT = 25,
    DAMAGE = 26
};

int fist_tree_restore(const fist_unit_definition *definition, fist_pool_allocation allocation,
                      fist_tree *out) {
    if (definition == NULL || out == NULL || definition->type != TREE || allocation.type != TREE ||
        allocation.slot >= FIST_POOL_SHORT_SLOTS ||
        allocation.registry_index != definition->registry_index ||
        allocation.registry_index >= FIST_UNIT_REGISTRY_COUNT ||
        allocation.value != definition->generation || definition->snapshot.data == NULL ||
        definition->snapshot.size != FIST_UNIT_SHORT_SIZE ||
        fist_read_u16le(definition->snapshot.data) != TREE) {
        return -1;
    }
    const uint8_t *raw = definition->snapshot.data;
    *out = (fist_tree){
        .allocation = allocation,
        .pose = {definition->map_x, definition->map_y, definition->altitude, definition->heading},
        .extent = fist_read_u16le(raw + EXTENT),
        .projection_scale = fist_read_u16le(raw + SCALE),
        .flags = raw[FLAGS],
        .secondary_flags = raw[SECONDARY],
        .ground_height = raw[GROUND],
        .variant = raw[VARIANT],
        .damage = raw[DAMAGE]};
    return 0;
}

int fist_tree_advance(const fist_object_pool *pool, fist_tree *tree, fist_tree_update update) {
    if (tree == NULL || tree->allocation.type != TREE ||
        !fist_object_pool_is_current(pool, tree->allocation)) {
        return -1;
    }
    if (update.changed == 1) {
        tree->variant = update.variant;
    }
    return 0;
}
