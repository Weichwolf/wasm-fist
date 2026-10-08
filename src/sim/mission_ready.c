#include "sim/mission_world.h"

#include "sim/ground_support.h"

#include "assets/klc.h"
#include "assets/units.h"
#include "sim/ground.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/random.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    EXPLOSION = 4,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    SHELL = 8,
    TEMPORARY_FIRST = 11,
    TEMPORARY_SECOND = 13,
    COUNTED_STATIC = 16,
    SMOKE = 17,
    MUZZLE = 18,
    RETIRING = 19,
    TREE = 21,
    WRECK = 23,
    HEIGHT_STATIC = 25,
    TARGET = 26,
    ARTILLERY = 27,
    TREE_VARIANTS = 4,
    TARGET_VARIANTS = 8,
    ARTILLERY_VARIANTS = 2,
    DELETED = 1,
    SIDE = 8,
    TREE_FLAGS = 64,
    TREE_SECONDARY = 4,
    TREE_SCALE = 512,
    TARGET_FLAGS = 78,
    TARGET_SECONDARY = 20,
    DESTROYED_TARGET = 4,
    CLEAR_MOTION = 6,
    TARGET_SECONDARY_CLEAR = 8,
    ARTILLERY_SECONDARY_CLEAR = 24,
    ARTILLERY_FLAGS = 70,
    ARTILLERY_ROUNDS = 5
};

static int sample_pose(const fist_klc_image *height, fist_object_pose *pose) {
    uint8_t value = 0;
    if (fist_ground_height_sample(height, pose->x, pose->y, &value) != 0) {
        return -1;
    }
    pose->altitude = fist_altitude_set_height(pose->altitude, value);
    return 0;
}

static int prepare_tree(fist_mission_world *world, fist_tree *tree, const fist_klc_image *height) {
    /* Actual DS:9322/932a four-word extent tables used by 9bef. */
    static const uint16_t masks[TREE_VARIANTS] = {63, 63, 31, 31};
    static const uint16_t extents[TREE_VARIANTS] = {256, 240, 160, 144};
    if (tree->variant >= TREE_VARIANTS || sample_pose(height, &tree->pose) != 0) {
        return -1;
    }
    uint16_t value = 0;
    if (fist_random_next(&world->random, &tree->pose.heading) != 0 ||
        fist_random_next(&world->random, &value) != 0) {
        return -1;
    }
    tree->extent = (uint16_t)((value & masks[tree->variant]) + extents[tree->variant]);
    tree->projection_scale = TREE_SCALE;
    tree->flags |= TREE_FLAGS;
    tree->secondary_flags |= TREE_SECONDARY;
    ++world->preparation.trees;
    return 0;
}

static int prepare_target(fist_other_actor *actor, const fist_klc_image *height) {
    /* Original DS:9ecf/9ef7 and byte table 9edf, for the eight authored modes. */
    static const uint16_t extents[TARGET_VARIANTS] = {640, 512, 512, 512, 640, 512, 512, 512};
    static const uint16_t scales[TARGET_VARIANTS] = {768, 768, 1024, 1280, 640, 640, 896, 1152};
    static const uint8_t limits[TARGET_VARIANTS] = {4, 8, 4, 80, 0, 0, 0, 0};
    if (actor->mode >= TARGET_VARIANTS || sample_pose(height, &actor->pose) != 0) {
        return -1;
    }
    actor->projection_extent = extents[actor->mode];
    actor->projection_scale = scales[actor->mode];
    actor->state.type26.limit = limits[actor->mode];
    actor->flags |= TARGET_FLAGS;
    actor->secondary_flags |= TARGET_SECONDARY;
    if ((actor->mode & DESTROYED_TARGET) != 0) {
        actor->flags = (uint8_t)((actor->flags | DELETED) & ~CLEAR_MOTION);
        actor->secondary_flags &= (uint8_t)~TARGET_SECONDARY_CLEAR;
    }
    return 0;
}

static int prepare_artillery(fist_mission_world *world, fist_other_actor *actor,
                             const fist_klc_image *height) {
    static const uint16_t extents[ARTILLERY_VARIANTS] = {256, 320};
    const size_t side = (actor->flags & SIDE) != 0;
    const uint16_t count = world->preparation.artillery_count[side];
    if (actor->mode >= ARTILLERY_VARIANTS || count >= FIST_MISSION_ARTILLERY_SIDE_SLOTS ||
        sample_pose(height, &actor->pose) != 0) {
        return -1;
    }
    actor->projection_extent = extents[actor->mode];
    actor->state.type27.rounds = ARTILLERY_ROUNDS;
    fist_object_reference reference = {0};
    if (fist_object_pool_reference(&world->pool, actor->allocation.slot, &reference) != 0) {
        return -1;
    }
    world->preparation.artillery[side][count] =
        (fist_artillery_resource){actor->allocation, reference};
    world->preparation.artillery_count[side] = (uint16_t)(count + 1);
    if (side != 0) {
        if (actor->mode != 1) {
            actor->flags |= ARTILLERY_FLAGS;
        } else {
            actor->flags &= (uint8_t)~CLEAR_MOTION;
            actor->secondary_flags &= (uint8_t)~ARTILLERY_SECONDARY_CLEAR;
        }
    }
    return 0;
}

static int prepare_binding(fist_mission_world *world, fist_pool_allocation allocation,
                           const fist_klc_image *height, uint8_t link_mode) {
    fist_mission_object *object = &world->objects[allocation.slot];
    if (allocation.type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        fist_vehicle_state *actor = &object->vehicle;
        if (actor->type != allocation.type || actor->registry_index != allocation.registry_index ||
            actor->generation != allocation.value) {
            return -1;
        }
        return fist_vehicle_prepare(actor, link_mode);
    }
    fist_pool_allocation identity = {0};
    uint8_t *flags = NULL;
    int status = 0;
    switch (allocation.type) {
    case COUNTED_STATIC:
    case HEIGHT_STATIC:
    case TEMPORARY_FIRST:
    case TEMPORARY_SECOND:
        identity = object->saved_base.allocation;
        flags = &object->saved_base.flags;
        break;
    case TREE:
        identity = object->tree.allocation;
        break;
    case WRECK:
        identity = object->wreck.allocation;
        break;
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT:
    case TARGET:
    case ARTILLERY:
        identity = object->other.allocation;
        flags = &object->other.flags;
        break;
    case SMOKE:
        identity = object->smoke.allocation;
        flags = &object->smoke.flags;
        break;
    case MUZZLE:
        identity = object->muzzle.allocation;
        flags = &object->muzzle.flags;
        break;
    case EXPLOSION:
        identity = object->explosion.allocation;
        flags = &object->explosion.flags;
        break;
    case SHELL:
        identity = object->projectile.allocation;
        flags = &object->projectile.flags;
        break;
    case RETIRING:
        identity =
            (fist_pool_allocation){object->vehicle.type, allocation.slot,
                                   object->vehicle.registry_index, object->vehicle.generation};
        flags = &object->vehicle.object_flags;
        break;
    default:
        return FIST_MISSION_UNSUPPORTED;
    }
    if (identity.slot != allocation.slot || !fist_object_pool_is_current(&world->pool, identity)) {
        return -1;
    }
    switch (allocation.type) {
    case COUNTED_STATIC:
        ++world->preparation.counted_static;
        return 0;
    case HEIGHT_STATIC:
        return sample_pose(height, &object->saved_base.pose);
    case TREE:
        return prepare_tree(world, &object->tree, height);
    case WRECK:
        return sample_pose(height, &object->wreck.pose);
    case TARGET:
        return prepare_target(&object->other, height);
    case ARTILLERY:
        return prepare_artillery(world, &object->other, height);
    default: {
        fist_pool_allocation released = {0};
        status = fist_object_pool_release(&world->pool, allocation.registry_index, &released);
        if (status != FIST_POOL_OK || flags == NULL) {
            return -1;
        }
        *flags |= DELETED;
        return 0;
    }
    }
}

int fist_mission_world_prepare(fist_mission_world *world, const fist_klc_image *height,
                               uint8_t link_mode) {
    uint8_t ignored = 0;
    if (world == NULL || !fist_object_pool_is_valid(&world->pool) ||
        world->random.next_stream >= FIST_RANDOM_STREAMS ||
        world->pending_player_impact != FIST_POOL_NO_SLOT ||
        fist_ground_height_sample(height, 0, 0, &ignored) != 0) {
        return -1;
    }
    fist_mission_world *prepared = malloc(sizeof(*prepared));
    if (prepared == NULL) {
        return -1;
    }
    *prepared = *world;
    prepared->preparation = (fist_mission_preparation){0};
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        for (size_t entry = 0; entry < FIST_MISSION_ARTILLERY_SIDE_SLOTS; ++entry) {
            prepared->preparation.artillery[side][entry].allocation.slot = FIST_POOL_NO_SLOT;
        }
    }
    int status = 0;
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT && status == 0; ++index) {
        const uint16_t slot = prepared->pool.registry[index].slot;
        if (slot == FIST_POOL_NO_SLOT) {
            continue;
        }
        fist_pool_allocation allocation = {0};
        if (fist_object_pool_find(&prepared->pool, slot, &allocation) != FIST_POOL_OK) {
            status = -1;
        } else {
            status = prepare_binding(prepared, allocation, height, link_mode);
        }
    }
    if (status == 0) {
        prepared->preparation.prepared = 1;
        *world = *prepared;
    }
    free(prepared);
    return status;
}
