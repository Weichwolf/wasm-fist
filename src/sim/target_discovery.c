#include "sim/mission_world.h"

#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "sim/geometry.h"
#include "sim/ground.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    ELIGIBLE = 4,
    SIDE = 8,
    SECONDARY = 8,
    SECONDARY_MOTION = 128,
    AUTOMATIC = 1,
    LINK_OPERATING = 16,
    LINK_THRESHOLD = 2,
    DISTANCE_SHIFT = 8,
    DISTANCE_LIMIT = 1 << 24,
    PRIORITY_AH = 0xff00,
    HALF_TURN = 32768,
    ACQUIRED_VOICE = 14
};

/* Complete original DS:98a4/98c0; selected by 989c for classes 0/2 and 1/3. */
static const uint8_t preferences[2][FIST_UNIT_TYPE_COUNT] = {
    {0,  0,  0,  0,  99, 1,  1,  99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 2,  2},
    {1,  1,  1,  1,  99, 0,  0,  99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 2,  2}};
static const uint16_t ranges[3][FIST_UNIT_GROUND_VEHICLE_COUNT] = {
    {1000, 1000, 1000, 1000}, {150, 150, 150, 150}, {625, 625, 450, 450}};
static uint16_t range_limit(const fist_vehicle_state *actor, uint8_t link) {
    size_t bank = 0;
    if (link >= LINK_THRESHOLD) {
        bank = (actor->operating_flags & LINK_OPERATING) != 0 ? 2 : 1;
    }
    return ranges[bank][actor->type];
}

static uint16_t primary_operand(fist_target_discovery_result *result, uint8_t priority,
                                fist_object_reference candidate, uint16_t limit,
                                fist_order_waypoint source, fist_order_waypoint target) {
    if (priority > result->priority) {
        return (uint16_t)(PRIORITY_AH | priority);
    }
    if (priority < result->priority) {
        result->primary_range = UINT16_MAX;
        result->secondary_operand = UINT16_MAX;
        result->priority = priority;
    }
    const uint32_t distance = fist_planar_proximity(source, target);
    if (distance >= DISTANCE_LIMIT) {
        return (uint16_t)distance;
    }
    const uint16_t operand = (uint16_t)(distance >> DISTANCE_SHIFT);
    if (operand <= limit && operand < result->primary_range) {
        result->primary_range = operand;
        result->primary = candidate;
        ++result->count;
    }
    return operand;
}

static int scan(const fist_mission_world *world, const fist_klc_image *height,
                const fist_vehicle_state *actor, fist_target_discovery_request request,
                fist_target_discovery_result *result) {
    const uint16_t limit = range_limit(actor, request.link_mode);
    const uint8_t *preference = preferences[actor->type & 1];
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const uint16_t slot = world->pool.registry[index].slot;
        if (slot == FIST_POOL_NO_SLOT || slot == request.slot) {
            continue;
        }
        fist_object_pose ground = {0};
        fist_mission_view view = {0};
        const int status = fist_mission_world_view(world, slot, &ground, &view);
        if (status != 0) {
            return status;
        }
        if ((view.flags & ELIGIBLE) == 0 || ((view.flags ^ actor->object_flags) & SIDE) == 0) {
            continue;
        }
        const uint16_t type = world->pool.slots[slot].type;
        fist_object_pose source = {0};
        fist_object_pose target = {0};
        if (fist_mission_world_target_positions(world, request.slot, slot, &source, &target) != 0) {
            return -1;
        }
        bool visible = false;
        if (fist_ground_visible(height, &source, &target, &visible) != 0) {
            return -1;
        }
        fist_object_reference candidate = {0};
        if (fist_object_pool_reference(&world->pool, slot, &candidate) != FIST_POOL_OK) {
            return -1;
        }
        const uint16_t operand =
            visible ? primary_operand(result, preference[type], candidate, limit,
                                      (fist_order_waypoint){view.pose->x, view.pose->y},
                                      (fist_order_waypoint){actor->map_x, actor->map_y})
                    : 0;
        /* Actual AX is branch-dependent, including invisible zero and ffXX.
         * Better priority resets distances but deliberately retains pointers. */
        if ((view.secondary_flags & SECONDARY) != 0 && operand < result->secondary_operand) {
            result->secondary_operand = operand;
            result->secondary = candidate;
        }
    }
    return 0;
}

int fist_mission_world_discover_targets(fist_mission_world *world, const fist_klc_image *height,
                                        fist_target_discovery_request request,
                                        fist_target_discovery_result *out) {
    if (world == NULL || out == NULL || world->preparation.prepared != 1 ||
        fist_mission_world_object(world, request.slot) == NULL ||
        world->pool.slots[request.slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    const fist_vehicle_state *actor = &world->objects[request.slot].vehicle;
    if (actor->type != world->pool.slots[request.slot].type ||
        actor->component_size != fist_vehicle_component_size(actor->type) ||
        !fist_object_reference_is_valid(actor->command.target) ||
        !fist_object_reference_is_valid(actor->command.candidate)) {
        return -1;
    }
    /* Validate the borrowed installed plane even for an empty/ineligible scan. */
    uint8_t sampled = 0;
    if (fist_ground_height_sample(height, actor->map_x, actor->map_y, &sampled) != 0) {
        return -1;
    }
    fist_target_discovery_result result = {
        .primary_range = UINT16_MAX, .secondary_operand = UINT16_MAX, .priority = UINT8_MAX};
    const int status = scan(world, height, actor, request, &result);
    if (status != 0) {
        return status;
    }
    fist_vehicle_state next = *actor;
    next.drive.motion_flags &= (uint8_t)~SECONDARY_MOTION;
    if (result.secondary.lifetime != 0) {
        fist_object_pose ground = {0};
        fist_mission_view view = {0};
        if (fist_mission_world_view(world, result.secondary.slot, &ground, &view) != 0) {
            return -1;
        }
        next.drive.motion_flags |= SECONDARY_MOTION;
        const fist_planar_measurement bearing =
            fist_planar_measure((fist_order_waypoint){view.pose->x, view.pose->y},
                                (fist_order_waypoint){actor->map_x, actor->map_y}, request.coarse);
        next.command.secondary_heading = (uint16_t)(bearing.heading + HALF_TURN);
    }
    bool clear_target = !fist_object_pool_reference_is_live(&world->pool, next.command.target);
    if (!clear_target && (next.control_flags & AUTOMATIC) != 0 &&
        (next.command.target.slot != result.primary.slot ||
         next.command.target.lifetime != result.primary.lifetime) &&
        preferences[next.type & 1][world->pool.slots[next.command.target.slot].type] >
            result.priority) {
        clear_target = true;
    }
    if (clear_target) {
        next.command.target = (fist_object_reference){0};
    }
    fist_voice_history voice = world->voice;
    const uint8_t candidate =
        actor->command.discovery_count == 0 && result.count != 0 ? ACQUIRED_VOICE : FIST_VOICE_NONE;
    if (fist_voice_admit(&voice,
                         (fist_voice_environment){request.voice_gate, request.tick,
                                                  actor->object_flags,
                                                  world->combat.selected_slot == request.slot},
                         candidate, &result.voice) != 0) {
        return -1;
    }
    next.command.discovery_count = result.count;
    next.command.candidate = result.primary;
    world->objects[request.slot].vehicle = next;
    world->voice = voice;
    *out = result;
    return 0;
}
