#include "assets/units.h"
#include "sim/automatic_fire.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

int fist_mission_world_view(const fist_mission_world *world, uint16_t slot,
                            fist_object_pose *ground_pose, fist_mission_view *out) {
    enum {
        EXPLOSION = 4,
        SHELL = 8,
        SMOKE = 17,
        MUZZLE = 18,
        RETIRING = 19,
        TREE = 21,
        WRECK = 23,
        FIRST_AIRCRAFT = 5,
        SECOND_AIRCRAFT = 6,
        TARGET = 26,
        ARTILLERY = 27
    };
    if (out == NULL || ground_pose == NULL || fist_mission_world_object(world, slot) == NULL) {
        return -1;
    }
    const uint16_t type = world->pool.slots[slot].type;
    const fist_mission_object *object = &world->objects[slot];
    fist_mission_view view = {0};
    const fist_pool_allocation *allocation = NULL;
    if (type < FIST_UNIT_GROUND_VEHICLE_COUNT || type == RETIRING) {
        const fist_vehicle_state *actor = &object->vehicle;
        if (actor->type != type) {
            return -1;
        }
        *ground_pose =
            (fist_object_pose){actor->map_x, actor->map_y, actor->altitude, actor->turret.heading};
        *out = (fist_mission_view){ground_pose, actor->projection_scale, actor->object_flags,
                                   actor->secondary_flags, actor->drive.motion_flags};
        return 0;
    }
    switch (type) {
    case FIST_SURFACE_AIR_TYPE:
        allocation = &object->surface_air.allocation;
        view = (fist_mission_view){&object->surface_air.pose, object->surface_air.projection_scale,
                                   object->surface_air.flags, object->surface_air.secondary_flags,
                                   object->surface_air.variant};
        break;
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT:
    case TARGET:
    case ARTILLERY:
        allocation = &object->other.allocation;
        view = (fist_mission_view){&object->other.pose, object->other.projection_scale,
                                   object->other.flags, object->other.secondary_flags,
                                   object->other.mode};
        break;
    case EXPLOSION:
        allocation = &object->explosion.allocation;
        view = (fist_mission_view){&object->explosion.pose, object->explosion.projection_scale,
                                   object->explosion.flags, object->explosion.secondary_flags,
                                   object->explosion.frame};
        break;
    case SHELL:
        allocation = &object->projectile.allocation;
        view = (fist_mission_view){&object->projectile.pose, object->projectile.projection_scale,
                                   object->projectile.flags, object->projectile.secondary_flags,
                                   object->projectile.mode};
        break;
    case SMOKE:
        allocation = &object->smoke.allocation;
        view = (fist_mission_view){&object->smoke.pose, object->smoke.projection_scale,
                                   object->smoke.flags, object->smoke.secondary_flags,
                                   object->smoke.animation_frame};
        break;
    case MUZZLE:
        allocation = &object->muzzle.allocation;
        view = (fist_mission_view){&object->muzzle.pose, object->muzzle.projection_scale,
                                   object->muzzle.flags, object->muzzle.secondary_flags,
                                   object->muzzle.animation_frame};
        break;
    case TREE:
        allocation = &object->tree.allocation;
        view = (fist_mission_view){&object->tree.pose, object->tree.projection_scale,
                                   object->tree.flags, object->tree.secondary_flags,
                                   object->tree.variant};
        break;
    case WRECK:
        allocation = &object->wreck.allocation;
        view = (fist_mission_view){&object->wreck.pose, object->wreck.projection_scale,
                                   object->wreck.flags, object->wreck.secondary_flags, 0};
        break;
    default:
        /* Explicit common-field payloads, including static/temporary saved
         * classes without delivered live methods. Projection is not dispatch. */
        allocation = &object->saved_base.allocation;
        view = (fist_mission_view){&object->saved_base.pose, object->saved_base.projection_scale,
                                   object->saved_base.flags, object->saved_base.secondary_flags,
                                   object->saved_base.variant};
        break;
    }
    if (allocation->type != type || allocation->slot != slot) {
        return -1;
    }
    *out = view;
    return 0;
}
