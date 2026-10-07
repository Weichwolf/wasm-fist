#include "sim/mission_world.h"

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>

enum {
    AUTOMATIC = 1,
    CONTROL_10 = 16,
    DAMAGE_COMMAND = 32,
    FORCED_DAMAGE_COMMAND = 64,
    MOTION_MODE_FLAGS = 6,
    COMMAND_CHOICES = 4,
    MODE_LEADER = 0,
    MODE_FOLLOWER = 2,
    MODE_DAMAGE = 4,
    MODE_TARGET = 6,
    MODE_MANEUVER = 8,
    MODE_CONTROL_10 = 10,
    MODE_MOTION = 12,
    MODE_BEHAVIOR_THREE = 14
};

/* Original DS:9946 and DS:9956, indexed by PINF words +0 and +2.
 * Original UI 61f0/6253 cycles each word through exactly four choices. */
static const uint8_t damage_chance[COMMAND_CHOICES] = {100, 10, 200, 0};
static const uint8_t target_chance[COMMAND_CHOICES] = {0, 50, 255, 0};

static int priority_mode(const fist_vehicle_state *actor, uint16_t behavior) {
    if (behavior == 3) {
        return MODE_BEHAVIOR_THREE;
    }
    if ((actor->drive.motion_flags & MOTION_MODE_FLAGS) != 0) {
        return MODE_MOTION;
    }
    if ((actor->control_flags & CONTROL_10) != 0) {
        return MODE_CONTROL_10;
    }
    if (actor->command.maneuver != 0) {
        return MODE_MANEUVER;
    }
    if ((actor->control_flags & FORCED_DAMAGE_COMMAND) != 0) {
        return MODE_DAMAGE;
    }
    return -1;
}

static int automatic_mode(fist_vehicle_state *actor, const fist_order_descriptor *descriptor,
                          fist_random *random, uint16_t phase_random) {
    const uint16_t behavior = descriptor->words[0];
    const int priority = priority_mode(actor, behavior);
    if (priority >= 0) {
        actor->command.mode = (uint8_t)priority;
        return 0;
    }
    if ((actor->control_flags & DAMAGE_COMMAND) != 0) {
        if (behavior >= COMMAND_CHOICES) {
            return -1;
        }
        actor->control_flags &= (uint16_t)~DAMAGE_COMMAND;
        if ((uint8_t)phase_random <= damage_chance[behavior]) {
            actor->command.mode = MODE_DAMAGE;
            return 0;
        }
    }
    if (actor->command.target_reference != 0) {
        const uint16_t waypoint_mode = descriptor->words[1];
        uint16_t rolled = 0;
        if (waypoint_mode >= COMMAND_CHOICES || fist_random_next(random, &rolled) != 0) {
            return -1;
        }
        if ((uint8_t)rolled < target_chance[waypoint_mode]) {
            actor->command.mode = MODE_TARGET;
            return 0;
        }
    }
    actor->command.mode = actor->member == 0 ? MODE_LEADER : MODE_FOLLOWER;
    return 0;
}

int fist_mission_world_select_command(fist_mission_world *world, fist_command_selection request) {
    const uint16_t slot = request.slot;
    if (world == NULL || world->orders_loaded != 1 ||
        world->random.next_stream >= FIST_RANDOM_STREAMS ||
        fist_mission_world_object(world, slot) == NULL ||
        world->pool.slots[slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (actor->type != world->pool.slots[slot].type || actor->platoon >= FIST_UNIT_PLATOON_COUNT ||
        actor->component_size != fist_vehicle_component_size(actor->type)) {
        return -1;
    }
    fist_vehicle_state next = *actor;
    fist_random random = world->random;
    if ((next.control_flags & AUTOMATIC) == 0) {
        next.command.mode = next.member == 0 ? MODE_LEADER : MODE_FOLLOWER;
    } else if (automatic_mode(&next, &world->orders.descriptors[next.platoon], &random,
                              request.phase_random) != 0) {
        return -1;
    }
    *actor = next;
    world->random = random;
    return 0;
}
