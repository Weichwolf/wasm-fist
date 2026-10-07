#include "sim/mission_world.h"

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/geometry.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/rotation.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    MOTION_BLOCKED = 8,
    TURRET_CONTROLLED = 4,
    MANEUVER_MASK = 6,
    SEARCH_STATE = 2,
    TURN_STATE = 4,
    REVERSE_STATE = 6,
    FIRST_BLOCKED_COUNT = 3,
    TURN_COUNT = 4,
    SEARCH_DIRECTIONS = 15,
    SELECTED_OFFSET = 8,
    SEARCH_MAGNITUDE = 32,
    MOTION_MAGNITUDE = 64,
    VECTOR_SCALE = 8,
    PREDICTION_SAMPLES = 24,
    SEARCH_MARGIN = 1024,
    MOTION_MARGIN = 3584,
    SEARCH_SEPARATION = 7680,
    COLLIDABLE = 64,
    TREE = 21,
    QUARTER_TURN = 16384,
    HALF_TURN = 32768,
    THREE_QUARTER_TURN = 49152,
    REVERSE_MASK = 0x3f,
    FORWARD_MASK = 0x3c0,
    RANDOM_MASK = 0x1c00,
    OFFSET_MASK = 0x3fff,
    OFFSET_CENTER = 8192
};

/* Original DS:97aa. Only the fifteen tested and eight-ahead selected offsets
 * belong to these callbacks; exhaustion selects the untested index fifteen. */
static const int16_t turn_offsets[SEARCH_DIRECTIONS + SELECTED_OFFSET + 1] = {
    910,  -910,  1820, -1820, 2730, -2730, 3640, -3640, 4550,  -4550,  5460,  -5460,
    6370, -6370, 7280, -7280, 8190, -8190, 9100, -9100, 10010, -10010, 10920, -10920};

typedef struct {
    uint32_t x;
    uint32_t y;
} prediction_vector;

static fist_vehicle_state *maneuver_actor(fist_mission_world *world, uint16_t slot) {
    if (fist_mission_world_object(world, slot) == NULL ||
        world->pool.slots[slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return NULL;
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    return actor->type == world->pool.slots[slot].type &&
                   actor->component_size == fist_vehicle_component_size(actor->type)
               ? actor
               : NULL;
}

static int32_t signed_coordinate(uint32_t bits) {
    return bits <= INT32_MAX ? (int32_t)bits : -1 - (int32_t)(UINT32_MAX - bits);
}

static prediction_vector scaled_vector(fist_velocity velocity) {
    return (prediction_vector){(uint32_t)((int32_t)velocity.x * VECTOR_SCALE),
                               (uint32_t)((int32_t)velocity.y * VECTOR_SCALE)};
}

static bool angular_admission(const fist_vehicle_state *actor, const fist_object_pose *candidate,
                              bool coarse) {
    const fist_planar_measurement bearing =
        fist_planar_measure((fist_order_waypoint){candidate->x, candidate->y},
                            (fist_order_waypoint){actor->map_x, actor->map_y}, coarse);
    const uint16_t difference = (uint16_t)(bearing.heading + HALF_TURN - actor->turret.heading);
    return difference < QUARTER_TURN || difference >= THREE_QUARTER_TURN;
}

static bool predicted_hit(const fist_vehicle_state *actor, fist_mission_view candidate,
                          uint16_t margin, prediction_vector vector) {
    const uint16_t radius =
        (uint16_t)(actor->projection_scale + candidate.projection_scale + margin);
    uint32_t position_x = (uint32_t)actor->map_x + (vector.x * VECTOR_SCALE);
    uint32_t position_y = (uint32_t)actor->map_y + (vector.y * VECTOR_SCALE);
    const fist_order_waypoint source = {candidate.pose->x, candidate.pose->y};
    for (unsigned sample = 0; sample < PREDICTION_SAMPLES; ++sample) {
        position_x += vector.x;
        position_y += vector.y;
        const fist_order_waypoint position = {signed_coordinate(position_x),
                                              signed_coordinate(position_y)};
        if ((uint16_t)fist_planar_distance(source, position) <= radius) {
            return true;
        }
    }
    return false;
}

static uint32_t magnitude(uint32_t bits) {
    return bits <= INT32_MAX ? bits : 0U - bits;
}

/* Return hit/clear/invalid as 1/0/-1. Iterate current bindings independently
 * of actor generation, side, logical roster or overwritten actor bindings. */
static int direction_blocked(const fist_mission_world *world, uint16_t slot,
                             const fist_vehicle_state *actor, prediction_vector vector,
                             bool coarse) {
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const uint16_t candidate = world->pool.registry[index].slot;
        if (candidate == FIST_POOL_NO_SLOT || candidate == slot ||
            world->pool.slots[candidate].type == TREE) {
            continue;
        }
        fist_object_pose storage = {0};
        fist_mission_view view = {0};
        if (fist_mission_world_view(world, candidate, &storage, &view) != 0) {
            return -1;
        }
        if ((view.flags & COLLIDABLE) == 0 || !angular_admission(actor, view.pose, coarse)) {
            continue;
        }
        const uint32_t separation = magnitude((uint32_t)actor->map_x - (uint32_t)view.pose->x) +
                                    magnitude((uint32_t)actor->map_y - (uint32_t)view.pose->y);
        if (separation <= SEARCH_SEPARATION && predicted_hit(actor, view, SEARCH_MARGIN, vector)) {
            return 1;
        }
    }
    return 0;
}

static int search_turn(const fist_mission_world *world, uint16_t slot,
                       const fist_vehicle_state *actor, bool coarse, uint16_t *out) {
    unsigned index = 0;
    for (; index < SEARCH_DIRECTIONS; ++index) {
        const fist_velocity velocity = fist_rotate((fist_rotation){
            (uint16_t)(actor->drive.heading + turn_offsets[index]), SEARCH_MAGNITUDE, coarse});
        const int hit = direction_blocked(world, slot, actor, scaled_vector(velocity), coarse);
        if (hit < 0) {
            return -1;
        }
        if (hit == 0) {
            break;
        }
    }
    *out = (uint16_t)(actor->drive.heading + turn_offsets[index + SELECTED_OFFSET]);
    return 0;
}

int fist_mission_world_maneuver(fist_mission_world *world, uint16_t slot, bool coarse) {
    fist_vehicle_state *actor = maneuver_actor(world, slot);
    if (actor == NULL) {
        return -1;
    }
    fist_vehicle_command next = actor->command;
    const uint8_t state = next.maneuver & MANEUVER_MASK;
    const bool blocked = (actor->control_flags & MOTION_BLOCKED) != 0;
    if (state == 0) {
        if (!blocked) {
            actor->command.blocked_count = 0;
            return 0;
        }
        next.blocked_count = (uint8_t)(next.blocked_count + 1U);
        if (next.blocked_count < FIRST_BLOCKED_COUNT) {
            next.maneuver = SEARCH_STATE;
            next.maneuver_count = FIRST_BLOCKED_COUNT;
            actor->command = next;
            return 0;
        }
    }
    next.maneuver_count = (uint8_t)(next.maneuver_count - 1U);
    if (next.maneuver_count == 0) {
        if (state == 0 || state == REVERSE_STATE) {
            next.maneuver = 0;
        } else if (state == TURN_STATE) {
            next.maneuver = blocked ? TURN_STATE : 0;
            next.maneuver_count = blocked ? TURN_COUNT : 0;
        } else {
            if (search_turn(world, slot, actor, coarse, &next.maneuver_heading) != 0) {
                return -1;
            }
            next.maneuver = TURN_STATE;
            next.maneuver_count = TURN_COUNT;
        }
    }
    actor->command = next;
    return 0;
}

int fist_mission_world_observe_obstacle(fist_mission_world *world,
                                        fist_obstacle_observation request) {
    fist_vehicle_state *actor = maneuver_actor(world, request.slot);
    fist_object_pose storage = {0};
    fist_mission_view view = {0};
    if (actor == NULL || fist_mission_world_view(world, request.candidate, &storage, &view) != 0) {
        return -1;
    }
    if (!angular_admission(actor, view.pose, request.coarse)) {
        return 0;
    }
    const fist_velocity velocity =
        actor->command.maneuver == 0
            ? (fist_velocity){actor->drive.velocity_x, actor->drive.velocity_y}
            : fist_rotate((fist_rotation){actor->drive.heading, MOTION_MAGNITUDE, request.coarse});
    if (predicted_hit(actor, view, MOTION_MARGIN, scaled_vector(velocity))) {
        actor->control_flags |= MOTION_BLOCKED;
    }
    return 0;
}

int fist_mission_world_idle_turret(fist_mission_world *world, uint16_t slot) {
    fist_vehicle_state *actor = maneuver_actor(world, slot);
    if (actor == NULL) {
        return -1;
    }
    if ((actor->control_flags & TURRET_CONTROLLED) != 0 || actor->command.target_reference != 0 ||
        actor->command.target.lifetime != 0) {
        return 0;
    }
    fist_random random = world->random;
    uint16_t value = 0;
    if (fist_random_next(&random, &value) != 0) {
        return -1;
    }
    uint16_t offset = actor->turret.requested_offset;
    if ((value & REVERSE_MASK) == 0) {
        offset = HALF_TURN;
    } else if ((value & FORWARD_MASK) == 0) {
        offset = 0;
    } else if ((value & RANDOM_MASK) == 0) {
        if (fist_random_next(&random, &value) != 0) {
            return -1;
        }
        offset = (uint16_t)((value & OFFSET_MASK) - OFFSET_CENTER);
    }
    actor->turret.requested_offset = offset;
    world->random = random;
    return 0;
}
