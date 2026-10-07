#include "sim/mission_world.h"

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>

enum { WRECK = 23, RETIRING = 19, MOTION_BLOCKED = 0x16, GOAL_VALID = 2 };

/* Positive admits promotion, zero retains the roster, negative rejects input. */
static int admit_predecessor(fist_mission_world *world, const fist_vehicle_state *actor,
                             uint16_t predecessor, uint8_t **member) {
    if (predecessor == FIST_POOL_NO_SLOT) {
        return 1;
    }
    if (fist_mission_world_object(world, predecessor) == NULL) {
        return -1;
    }
    const uint16_t type = world->pool.slots[predecessor].type;
    fist_mission_object *object = &world->objects[predecessor];
    if (type == WRECK) {
        if (object->wreck.allocation.type != type || object->wreck.allocation.slot != predecessor) {
            return -1;
        }
        *member = &object->wreck.member;
        return 1;
    }
    if ((actor->drive.motion_flags & MOTION_BLOCKED) != 0) {
        return 0;
    }
    if ((type >= FIST_UNIT_GROUND_VEHICLE_COUNT && type != RETIRING) ||
        object->vehicle.type != type) {
        return -1;
    }
    if ((object->vehicle.drive.motion_flags & MOTION_BLOCKED) == 0) {
        return 0;
    }
    *member = &object->vehicle.member;
    return 1;
}

int fist_mission_world_promote_member(fist_mission_world *world, uint16_t slot) {
    if (fist_mission_world_object(world, slot) == NULL ||
        world->pool.slots[slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (actor->type != world->pool.slots[slot].type ||
        actor->component_size != fist_vehicle_component_size(actor->type)) {
        return -1;
    }
    if (actor->member == 0) {
        return 0;
    }
    if (actor->platoon >= FIST_UNIT_PLATOON_COUNT ||
        actor->member >= FIST_UNIT_MEMBERS_PER_PLATOON) {
        return -1;
    }
    const size_t index = ((size_t)actor->platoon * FIST_UNIT_MEMBERS_PER_PLATOON) + actor->member;
    const uint16_t predecessor = world->combat.roster[index - 1];
    uint8_t *member = NULL;
    const int admitted = admit_predecessor(world, actor, predecessor, &member);
    if (admitted <= 0) {
        return admitted;
    }
    /* All used identities and fields are validated before the first write. */
    if (member != NULL) {
        *member = (uint8_t)(*member + 1U);
    }
    world->combat.roster[index - 1] = slot;
    world->combat.roster[index] = predecessor;
    actor->member = (uint8_t)(actor->member - 1U);
    actor->control_flags &= (uint16_t)~GOAL_VALID;
    return 0;
}
