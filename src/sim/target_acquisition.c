#include "sim/mission_world.h"

#include "assets/klc.h"
#include "assets/units.h"
#include "sim/geometry.h"
#include "sim/ground.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    TARGET = 26,
    VARIANT_MASK = 3,
    DELETED = 1,
    INELIGIBLE = 64,
    SIDE = 8,
    BEHAVIORS = 4,
    ATTEMPT_FLAG = 128,
    NOTICE_DURATION = 120,
    RANGE_SHIFT = 8
};

/* One owner for actual e21c and f69:abd5 source/target heights. */
static const uint16_t source_heights[FIST_UNIT_GROUND_VEHICLE_COUNT] = {2048, 2560, 2048, 1920};
static const uint16_t target_heights[FIST_UNIT_TYPE_COUNT] = {
    1792, 2048, 1792, 1536, 0, 256, 256, 0, 0, 0, 0, 0, 0, 0,
    0,    0,    0,    0,    0, 0,   0,   0, 0, 0, 0, 0, 0, 2048};
static const uint16_t variant_heights[VARIANT_MASK + 1] = {3840, 4352, 2560, 3072};
static const uint8_t chances[BEHAVIORS] = {120, 200, 40, 0};
static const uint8_t voices[FIST_UNIT_TYPE_COUNT] = {15, 16, 15, 16, 18, 18, 18, 18, 18, 18,
                                                     18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
                                                     18, 18, 18, 18, 18, 18, 18, 17};

int fist_mission_world_target_positions(const fist_mission_world *world, uint16_t actor,
                                        uint16_t target, fist_object_pose *source,
                                        fist_object_pose *destination) {
    if (source == NULL || destination == NULL || source == destination ||
        fist_mission_world_object(world, actor) == NULL ||
        world->pool.slots[actor].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    fist_object_pose ground_source = {0};
    fist_object_pose ground_target = {0};
    fist_mission_view origin = {0};
    fist_mission_view target_view = {0};
    if (fist_mission_world_view(world, actor, &ground_source, &origin) != 0 ||
        fist_mission_world_view(world, target, &ground_target, &target_view) != 0) {
        return -1;
    }
    const uint16_t type = world->pool.slots[target].type;
    fist_object_pose from = *origin.pose;
    fist_object_pose to = *target_view.pose;
    from.altitude = fist_position_add(from.altitude, source_heights[world->pool.slots[actor].type]);
    to.altitude = fist_position_add(
        to.altitude,
        type == TARGET ? variant_heights[target_view.mode & VARIANT_MASK] : target_heights[type]);
    *source = from;
    *destination = to;
    return 0;
}

static fist_vehicle_state *actor_state(fist_mission_world *world, uint16_t slot) {
    if (world == NULL || world->preparation.prepared != 1 || world->orders_loaded != 1 ||
        world->random.next_stream >= FIST_RANDOM_STREAMS ||
        fist_mission_world_object(world, slot) == NULL ||
        world->pool.slots[slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return NULL;
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (actor->type != world->pool.slots[slot].type ||
        actor->component_size != fist_vehicle_component_size(actor->type) ||
        actor->platoon >= FIST_UNIT_PLATOON_COUNT ||
        !fist_object_reference_is_valid(actor->command.target) ||
        !fist_object_reference_is_valid(actor->command.candidate)) {
        return NULL;
    }
    return actor;
}

static fist_object_reference live_reference(const fist_object_pool *pool,
                                            fist_object_reference reference) {
    return fist_object_pool_reference_is_live(pool, reference) ? reference
                                                               : (fist_object_reference){0};
}

static int install_target(const fist_mission_world *world, const fist_klc_image *height,
                          fist_target_acquisition_request request, fist_object_reference candidate,
                          fist_vehicle_state *actor, fist_target_notice *notice,
                          fist_voice_history *history, fist_target_acquisition_result *result) {
    if (candidate.lifetime == 0) {
        return 0;
    }
    fist_object_pose ground = {0};
    fist_mission_view view = {0};
    if (fist_mission_world_view(world, candidate.slot, &ground, &view) != 0) {
        return -1;
    }
    if ((view.flags & DELETED) != 0 || (view.secondary_flags & INELIGIBLE) != 0) {
        return 0;
    }
    fist_object_pose source = {0};
    fist_object_pose target = {0};
    bool visible = false;
    if (fist_mission_world_target_positions(world, request.slot, candidate.slot, &source,
                                            &target) != 0 ||
        fist_ground_visible(height, &source, &target, &visible) != 0) {
        return -1;
    }
    actor->command.target = visible ? candidate : (fist_object_reference){0};
    result->installed = visible;
    if (!visible || world->combat.selected_slot != request.slot) {
        return 0;
    }
    const uint16_t type = world->pool.slots[candidate.slot].type;
    if (type == TARGET && view.mode > VARIANT_MASK) {
        return -1; /* Proved adjacent-text read has no valid typed message. */
    }
    *notice = (fist_target_notice){NOTICE_DURATION, type, type == TARGET ? view.mode : 0,
                                   (view.flags & SIDE) != 0};
    result->message = true;
    enum { FRIENDLY_VOICE = 20 };
    return fist_voice_admit(
        history,
        (fist_voice_environment){request.voice_gate, request.tick, actor->object_flags, true},
        (view.flags & SIDE) != 0 ? voices[type] : FRIENDLY_VOICE, &result->voice);
}

int fist_mission_world_acquire_target(fist_mission_world *world, const fist_klc_image *height,
                                      fist_target_acquisition_request request,
                                      fist_target_acquisition_result *out) {
    fist_vehicle_state *actor = actor_state(world, request.slot);
    if (actor == NULL || out == NULL ||
        (!request.automatic && !fist_object_reference_is_valid(request.candidate))) {
        return -1;
    }
    fist_vehicle_state next = *actor;
    fist_random random = world->random;
    fist_voice_history history = world->voice;
    fist_target_notice notice = world->target_notice;
    fist_target_acquisition_result result = {0};
    next.command.target = live_reference(&world->pool, next.command.target);
    next.command.candidate = live_reference(&world->pool, next.command.candidate);
    fist_object_reference candidate = request.automatic
                                          ? next.command.candidate
                                          : live_reference(&world->pool, request.candidate);
    if (request.automatic) {
        if (next.command.discovery_count != 0 && next.command.target.lifetime == 0) {
            const uint16_t behavior = world->orders.descriptors[next.platoon].words[0];
            uint16_t value = 0;
            if (behavior >= BEHAVIORS || fist_random_next(&random, &value) != 0) {
                return -1;
            }
            result.attempted = (uint8_t)value <= chances[behavior];
        }
    } else {
        result.attempted = true;
    }
    if (result.attempted) {
        if (install_target(world, height, request, candidate, &next, &notice, &history, &result) !=
            0) {
            return -1;
        }
        if (request.automatic) {
            next.control_flags |= ATTEMPT_FLAG;
        }
    }
    *actor = next;
    world->random = random;
    world->voice = history;
    world->target_notice = notice;
    *out = result;
    return 0;
}

int fist_mission_world_aim_target(fist_mission_world *world, uint16_t slot, bool coarse) {
    fist_vehicle_state *actor = actor_state(world, slot);
    if (actor == NULL) {
        return -1;
    }
    const fist_object_reference target = live_reference(&world->pool, actor->command.target);
    if (target.lifetime == 0) {
        actor->command.target = target;
        return 0;
    }
    fist_object_pose source = {0};
    fist_object_pose destination = {0};
    if (fist_mission_world_target_positions(world, slot, target.slot, &source, &destination) != 0) {
        return -1;
    }
    const fist_spatial_measurement aim = fist_spatial_measure(source, destination, coarse);
    actor->command.target_heading = aim.heading;
    actor->command.target_range = (uint16_t)(aim.distance >> RANGE_SHIFT);
    const int32_t elevation = aim.elevation <= INT16_MAX
                                  ? (int32_t)aim.elevation
                                  : -1 - (int32_t)(UINT16_MAX - aim.elevation);
    actor->turret.elevation = (int16_t)elevation;
    return 0;
}
