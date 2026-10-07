#include "sim/mission_world.h"

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/rotation.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

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

enum { GOAL_VALID = 2, FORMATIONS = 6, FORMATION_SCALE = 32, WRECK = 23 };

typedef struct {
    uint16_t heading;
    int16_t distance;
} formation_offset;

/* Original DS:9830 points to six consecutive four-member tables. Original UI
 * 62cf/62e2 cycles PINF word +4 through six choices; each member selects four
 * bytes. Keep the authored turn words rather than guessing idealized angles. */
static const formation_offset formations[FORMATIONS][FIST_UNIT_MEMBERS_PER_PLATOON] = {
    {{0, 0}, {47320, 512}, {18200, 512}, {0, 512}},
    {{0, 0}, {49140, 512}, {16380, 512}, {16380, 1024}},
    {{0, 0}, {32760, 512}, {32760, 1024}, {0, 512}},
    {{0, 0}, {43680, 512}, {43680, 1024}, {10920, 512}},
    {{0, 0}, {21840, 512}, {21840, 1024}, {54600, 512}},
    {{0, 0}, {43680, 848}, {21840, 848}, {32760, 912}}};

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

static fist_vehicle_state *command_actor(fist_mission_world *world, uint16_t slot) {
    if (world == NULL || world->orders_loaded != 1 ||
        world->random.next_stream >= FIST_RANDOM_STREAMS ||
        fist_mission_world_object(world, slot) == NULL ||
        world->pool.slots[slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return NULL;
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (actor->type != world->pool.slots[slot].type || actor->platoon >= FIST_UNIT_PLATOON_COUNT ||
        actor->component_size != fist_vehicle_component_size(actor->type)) {
        return NULL;
    }
    return actor;
}

int fist_mission_world_select_command(fist_mission_world *world, fist_command_selection request) {
    fist_vehicle_state *actor = command_actor(world, request.slot);
    if (actor == NULL) {
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

static int formation_goal(const fist_mission_world *world, fist_vehicle_state *actor) {
    if ((actor->control_flags & AUTOMATIC) == 0) {
        return 0;
    }
    const uint16_t formation = world->orders.descriptors[actor->platoon].words[2];
    if (formation >= FORMATIONS || actor->member >= FIST_UNIT_MEMBERS_PER_PLATOON) {
        return -1;
    }
    const uint16_t leader_slot =
        world->combat.roster[(size_t)actor->platoon * FIST_UNIT_MEMBERS_PER_PLATOON];
    if (leader_slot == FIST_POOL_NO_SLOT) {
        return 0;
    }
    const fist_mission_object *leader_object = fist_mission_world_object(world, leader_slot);
    if (leader_object == NULL) {
        return -1;
    }
    if (world->pool.slots[leader_slot].type == WRECK) {
        return 0;
    }
    const fist_vehicle_state *leader = &leader_object->vehicle;
    if (world->pool.slots[leader_slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        leader->type != world->pool.slots[leader_slot].type ||
        leader->component_size != fist_vehicle_component_size(leader->type)) {
        return -1;
    }
    const formation_offset offset = formations[formation][actor->member];
    const fist_velocity rotated = fist_rotate(
        (fist_rotation){.heading = (uint16_t)(leader->command.heading_average + offset.heading),
                        .magnitude = offset.distance});
    actor->command.goal = (fist_order_waypoint){
        fist_position_add(leader->map_x, (int32_t)rotated.x * FORMATION_SCALE),
        fist_position_add(leader->map_y, (int32_t)rotated.y * FORMATION_SCALE)};
    actor->control_flags |= GOAL_VALID;
    return 0;
}

int fist_mission_world_assign_command_goal(fist_mission_world *world, uint16_t slot) {
    fist_vehicle_state *actor = command_actor(world, slot);
    if (actor == NULL || actor->command.mode > MODE_BEHAVIOR_THREE ||
        actor->command.mode % 2 != 0) {
        return -1;
    }
    fist_vehicle_state next = *actor;
    if (next.command.mode == MODE_LEADER) {
        const fist_order_route *route = &world->orders.routes[next.platoon];
        if (route->count > FIST_ORDER_WAYPOINTS) {
            return -1;
        }
        if (route->count != 0) {
            next.command.goal = route->points[0];
            next.control_flags |= GOAL_VALID;
        }
    } else if (next.command.mode == MODE_FOLLOWER && formation_goal(world, &next) != 0) {
        return -1;
    }
    *actor = next;
    return 0;
}

int fist_mission_world_advance_command_route(fist_mission_world *world, uint16_t slot) {
    enum { REACHED_RANGE = 48, CYCLIC = 3 };
    fist_vehicle_state *actor = command_actor(world, slot);
    if (actor == NULL || actor->command.mode > MODE_BEHAVIOR_THREE ||
        actor->command.mode % 2 != 0) {
        return -1;
    }
    if (actor->command.mode != MODE_LEADER || (actor->control_flags & GOAL_VALID) == 0 ||
        actor->command.range > REACHED_RANGE) {
        return 0;
    }
    fist_order_route *route = &world->orders.routes[actor->platoon];
    const size_t count = route->count;
    if (count > FIST_ORDER_WAYPOINTS) {
        return -1;
    }
    if (count != 0) {
        const uint16_t mode = world->orders.descriptors[actor->platoon].words[1];
        if (mode == CYCLIC) {
            const fist_order_waypoint first = route->points[0];
            for (size_t point = 1; point < count; ++point) {
                route->points[point - 1] = route->points[point];
            }
            route->points[count - 1] = first;
        } else {
            /* Original 0/1/2 and >=4 fallback copy old-count points, including
             * one inactive in-record value. At capacity only that final copy
             * escapes the record; preserve the newly unused last value instead.
             * This repairs the proved neighbor dependency without losing a goal. */
            for (size_t point = 1; point <= count && point < FIST_ORDER_WAYPOINTS; ++point) {
                route->points[point - 1] = route->points[point];
            }
            --route->count;
        }
    }
    actor->control_flags &= (uint16_t)~GOAL_VALID;
    return 0;
}
