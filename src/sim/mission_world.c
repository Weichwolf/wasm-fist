#include "sim/mission_world.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/destruction_updates.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    PARTICIPANT = 32,
    WRECK_FLAGS = 64,
    WRECK_SECONDARY = 36,
    TREE = 21,
    WRECK = 23,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    TARGET = 26,
    ARTILLERY = 27,
    SMOKE = 17,
    MUZZLE = 18,
    SAVED_TEMPORARY_FIRST = 11,
    SAVED_TEMPORARY_SECOND = 13,
    COUNTED_STATIC = 16,
    HEIGHT_STATIC = 25,
    FLAGS = 22,
    EXTENT = 18,
    SCALE = 20,
    SECONDARY = 23,
    GROUND = 24,
    VARIANT = 25
};

void fist_mission_world_reset(fist_mission_world *world) {
    if (world == NULL) {
        return;
    }
    *world = (fist_mission_world){0};
    fist_object_pool_reset(&world->pool);
    world->combat.selected_slot = FIST_POOL_NO_SLOT;
    world->pending_player_impact = FIST_POOL_NO_SLOT;
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        world->combat.roster[index] = FIST_POOL_NO_SLOT;
    }
}

static int restore(fist_mission_world *world, const fist_unit_definition *definition,
                   fist_pool_allocation allocation, uint8_t link_mode) {
    fist_mission_object *object = &world->objects[allocation.slot];
    const int participant = (definition->flags & PARTICIPANT) != 0;
    if (definition->type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return participant ? fist_vehicle_initialize(definition, &world->random, link_mode,
                                                     &object->vehicle)
                           : fist_vehicle_restore(definition, &object->vehicle);
    }
    if (participant && definition->type != WRECK) {
        return -1;
    }
    switch (definition->type) {
    case SAVED_TEMPORARY_FIRST:
    case SAVED_TEMPORARY_SECOND:
    case COUNTED_STATIC:
    case HEIGHT_STATIC: {
        const uint8_t *raw = definition->snapshot.data;
        object->saved_base =
            (fist_saved_object_base){.allocation = allocation,
                                     .pose = {definition->map_x, definition->map_y,
                                              definition->altitude, definition->heading},
                                     .projection_extent = fist_read_u16le(raw + EXTENT),
                                     .projection_scale = fist_read_u16le(raw + SCALE),
                                     .flags = raw[FLAGS],
                                     .secondary_flags = raw[SECONDARY],
                                     .ground_height = raw[GROUND],
                                     .variant = raw[VARIANT]};
        return 0;
    }
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT:
    case TARGET:
    case ARTILLERY:
        return fist_other_actor_restore(definition, allocation, &object->other);
    case SMOKE:
        return fist_drifting_smoke_restore(definition, allocation, &object->smoke);
    case MUZZLE:
        return fist_muzzle_smoke_restore(definition, allocation, &object->muzzle);
    case TREE:
        return fist_tree_restore(definition, allocation, &object->tree);
    case WRECK: {
        const int status = fist_vehicle_wreck_restore(definition, allocation, &object->wreck);
        if (status == 0) {
            object->wreck.flags |= WRECK_FLAGS;
            object->wreck.secondary_flags |= WRECK_SECONDARY;
        }
        return status;
    }
    default:
        return FIST_MISSION_UNSUPPORTED;
    }
}

static int install(const fist_units *units, uint8_t link_mode, fist_mission_world *world) {
    uint16_t physical[FIST_UNIT_REGISTRY_COUNT] = {0};
    for (size_t index = 0; index < units->count; ++index) {
        const fist_unit_definition *definition = &units->definitions[index];
        if (definition->snapshot.data == NULL || definition->snapshot.size < FIST_UNIT_SHORT_SIZE ||
            definition->snapshot.size != fist_unit_state_size(definition->type) ||
            fist_read_u16le(definition->snapshot.data) != definition->type ||
            definition->flags != definition->snapshot.data[FLAGS]) {
            return -1;
        }
        fist_pool_allocation allocation = {0};
        const fist_pool_import request = {definition->type, definition->registry_index,
                                          definition->generation};
        int status = fist_object_pool_import(&world->pool, request, &allocation);
        if (status != FIST_POOL_OK) {
            return status;
        }
        physical[index] = allocation.slot;
        status = restore(world, definition, allocation, link_mode);
        if (status != 0) {
            return status;
        }
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        const uint16_t definition = units->roster[index];
        if (definition != FIST_UNIT_NO_DEFINITION) {
            if (definition >= units->count) {
                return -1;
            }
            world->combat.roster[index] = physical[definition];
        }
    }
    return 0;
}

int fist_mission_world_initialize(const fist_units *units, const fist_random *random,
                                  uint8_t link_mode, fist_mission_world *out) {
    if (units == NULL || random == NULL || out == NULL ||
        (units->count != 0 && units->definitions == NULL) ||
        random->next_stream >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    if (units->count > FIST_UNIT_REGISTRY_COUNT) {
        return FIST_POOL_UNAVAILABLE;
    }
    fist_mission_world *world = calloc(1, sizeof(*world));
    if (world == NULL) {
        return -1;
    }
    fist_mission_world_reset(world);
    world->random = *random;
    const int status = install(units, link_mode, world);
    if (status == 0) {
        *out = *world;
    }
    free(world);
    return status;
}

const fist_mission_object *fist_mission_world_object(const fist_mission_world *world,
                                                     uint16_t slot) {
    if (world == NULL || slot >= FIST_UNIT_REGISTRY_COUNT ||
        !fist_object_pool_is_valid(&world->pool) || world->pool.slots[slot].used == 0) {
        return NULL;
    }
    return &world->objects[slot];
}

int fist_mission_world_fire_untargeted(fist_mission_world *world, fist_fire_history *history,
                                       fist_fire_request request, fist_fire_result *out) {
    if (world == NULL || out == NULL || world->pending_player_impact != FIST_POOL_NO_SLOT ||
        fist_mission_world_object(world, request.launch.origin_slot) == NULL ||
        world->pool.slots[request.launch.origin_slot].type != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[request.launch.origin_slot].vehicle;
    fist_fire_result result = {0};
    if (fist_m1_fire_untargeted(&world->pool, actor, history, request, &result) != 0) {
        return -1;
    }
    if (result.dispatched && result.launch.outcome == FIST_LAUNCH_FIRED) {
        const fist_projectile *projectile = &result.launch.projectile;
        world->objects[projectile->allocation.slot] =
            (fist_mission_object){.projectile = *projectile};
        if (result.launch.has_muzzle) {
            const fist_muzzle_smoke *muzzle = &result.launch.muzzle;
            world->objects[muzzle->allocation.slot] = (fist_mission_object){.muzzle = *muzzle};
        }
    }
    *out = result;
    return 0;
}
