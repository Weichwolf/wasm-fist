#include "sim/mission_world.h"

#include "assets/klc.h"
#include "assets/units.h"
#include "sim/automatic_fire.h"
#include "sim/ground_support.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum { AUTOMATIC = 1, HEADING_INTERVAL_MASK = 15, DIAGNOSTIC_CAPTIONS = 2 };

enum {
    CALLBACK_SELECT_COMMAND = 0,
    CALLBACK_ASSIGN_GOAL = 1,
    CALLBACK_BEAR_COMMAND = 2,
    CALLBACK_THROTTLE = 3,
    CALLBACK_ADVANCE_ROUTE = 4,
    CALLBACK_MISSILE_FIRST = 5,
    CALLBACK_DISCOVER_TARGETS = 6,
    CALLBACK_MANEUVER_FIRST = 7,
    CALLBACK_ACQUIRE_TARGET = 8,
    CALLBACK_SELECT_STATION = 9,
    CALLBACK_AUTOMATIC_FIRE = 10,
    CALLBACK_IDLE_TURRET = 11,
    CALLBACK_REQUEST_SUPPORT = 12,
    CALLBACK_PROMOTE_MEMBER = 13,
    CALLBACK_MISSILE_SECOND = 14,
    CALLBACK_MANEUVER_SECOND = 15
};

static int32_t signed_heading(uint16_t word) {
    return word <= INT16_MAX ? (int32_t)word : -1 - (int32_t)(UINT16_MAX - word);
}

static void sample_heading(fist_vehicle_state *actor) {
    fist_vehicle_command *command = &actor->command;
    int32_t sum = signed_heading(actor->drive.heading);
    for (size_t index = 0; index < FIST_VEHICLE_HEADING_SAMPLES; ++index) {
        sum += signed_heading(command->heading_history[index]);
    }
    for (size_t index = FIST_VEHICLE_HEADING_SAMPLES - 1; index != 0; --index) {
        command->heading_history[index] = command->heading_history[index - 1];
    }
    command->heading_history[0] = actor->drive.heading;
    /* MOVSX additions followed by a logical32-bit SHR, then the word store. */
    command->heading_average = (uint16_t)((uint32_t)sum >> 2);
}

static int diagnostic(const fist_mission_world *world, fist_ground_phase_request request,
                      fist_ground_phase_result *result) {
    if (request.diagnostic_actor.slot != request.slot ||
        !fist_object_pool_reference_is_live(&world->pool, request.diagnostic_actor)) {
        return 0;
    }
    const fist_vehicle_state *actor = &world->objects[request.slot].vehicle;
    if (world->orders_loaded != 1 || actor->platoon >= FIST_UNIT_PLATOON_COUNT ||
        request.inhibition >= DIAGNOSTIC_CAPTIONS ||
        !fist_object_reference_is_valid(actor->command.candidate)) {
        return -1;
    }
    fist_ground_diagnostic next = {
        .actor = request.diagnostic_actor,
        .candidate = actor->command.candidate,
        .throttle_bits = (uint16_t)actor->drive.throttle,
        .platoon_speed = world->orders.descriptors[actor->platoon].words[3],
        .route_points = world->orders.routes[actor->platoon].count,
        .saved = request.diagnostic_saved,
        .navigation_range = actor->command.range,
        .mode = actor->command.mode,
        .discovery_count = actor->command.discovery_count,
        .maneuver = actor->command.maneuver,
        .platoon = actor->platoon,
        .member = actor->member,
        .inhibited = request.inhibition != 0,
        .candidate_live =
            fist_object_pool_reference_is_live(&world->pool, actor->command.candidate) != 0};
    result->diagnostic = next;
    result->diagnostic_emitted = true;
    return 0;
}

static int dispatch(fist_mission_world *world, const fist_klc_image *height,
                    fist_ground_phase_request request, fist_ground_phase_result *result) {
    const uint16_t slot = request.slot;
    switch (result->callback) {
    case CALLBACK_SELECT_COMMAND:
        return fist_mission_world_select_command(
            world, (fist_command_selection){slot, result->phase_random});
    case CALLBACK_ASSIGN_GOAL:
        return fist_mission_world_assign_command_goal(world, slot);
    case CALLBACK_BEAR_COMMAND:
        return fist_mission_world_bear_command(world, slot, request.coarse);
    case CALLBACK_THROTTLE:
        return result->automatic
                   ? fist_mission_world_throttle_command(world, slot, &result->drive)
                   : fist_mission_world_update_command_profile(world, slot, &result->drive);
    case CALLBACK_ADVANCE_ROUTE:
        return fist_mission_world_advance_command_route(world, slot);
    case CALLBACK_DISCOVER_TARGETS:
        return fist_mission_world_discover_targets(
            world, height,
            (fist_target_discovery_request){slot, request.tick, request.voice_gate,
                                            request.link_mode, request.coarse},
            &result->discovery);
    case CALLBACK_PROMOTE_MEMBER:
        return fist_mission_world_promote_member(world, slot);
    case CALLBACK_MISSILE_FIRST:
    case CALLBACK_AUTOMATIC_FIRE:
    case CALLBACK_MISSILE_SECOND:
        return result->automatic
                   ? fist_mission_world_automatic_fire(
                         world,
                         (fist_automatic_fire_request){{slot, request.sound_source},
                                                       result->phase_random,
                                                       result->callback != CALLBACK_AUTOMATIC_FIRE},
                         &result->fire)
                   : 0;
    case CALLBACK_MANEUVER_FIRST:
    case CALLBACK_MANEUVER_SECOND:
        return result->automatic ? fist_mission_world_maneuver(world, slot, request.coarse) : 0;
    case CALLBACK_ACQUIRE_TARGET:
        return result->automatic ? fist_mission_world_acquire_target(
                                       world, height,
                                       (fist_target_acquisition_request){
                                           slot, {0}, request.tick, request.voice_gate, true},
                                       &result->acquisition)
                                 : 0;
    case CALLBACK_SELECT_STATION:
        return result->automatic
                   ? fist_mission_world_select_station(
                         world,
                         (fist_ground_station_request){slot, request.clock, request.tick,
                                                       request.voice_gate, request.notice_context},
                         &result->station)
                   : 0;
    case CALLBACK_IDLE_TURRET:
        return result->automatic ? fist_mission_world_idle_turret(world, slot) : 0;
    case CALLBACK_REQUEST_SUPPORT:
        return result->automatic
                   ? fist_mission_world_request_support(
                         world, height,
                         (fist_ground_support_request){slot, request.sound_source, request.clock,
                                                       request.tick, request.notice_context,
                                                       request.coarse},
                         &result->support)
                   : 0;
    default:
        return -1;
    }
}

int fist_mission_world_ground_command(fist_mission_world *world, const fist_klc_image *height,
                                      fist_ground_phase_request request,
                                      fist_ground_phase_result *out) {
    if (out == NULL || fist_mission_world_object(world, request.slot) == NULL ||
        world->preparation.prepared != 1 ||
        world->pool.slots[request.slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    const fist_vehicle_state *original = &world->objects[request.slot].vehicle;
    if (original->type != world->pool.slots[request.slot].type ||
        original->component_size != fist_vehicle_component_size(original->type)) {
        return -1;
    }
    fist_mission_world *next = malloc(sizeof(*next));
    if (next == NULL) {
        return -1;
    }
    *next = *world;
    fist_ground_phase_result result = {
        .fire = {.missile = {.slot = FIST_POOL_NO_SLOT}},
        .station = {.weapon = {.voice_request = FIST_WEAPON_NO_REQUEST,
                               .notice_request = FIST_WEAPON_NO_REQUEST}},
        .support = {.marker = {.slot = FIST_POOL_NO_SLOT},
                    .resource_slot = FIST_POOL_NO_SLOT,
                    .queue_index = FIST_SUPPORT_NO_QUEUE},
        .callback = FIST_GROUND_COMMAND_NO_CALLBACK};
    int status = fist_random_next(&next->random, &result.phase_random);
    if (status == 0 && request.inhibition == 0) {
        fist_vehicle_state *actor = &next->objects[request.slot].vehicle;
        if (next->orders_loaded != 1 || actor->platoon >= FIST_UNIT_PLATOON_COUNT) {
            status = -1;
        } else {
            actor->random_phases[1] = (uint8_t)(actor->random_phases[1] + 1);
            result.callback = actor->random_phases[1] & HEADING_INTERVAL_MASK;
            result.automatic = (actor->control_flags & AUTOMATIC) != 0;
            if (result.callback == CALLBACK_SELECT_COMMAND) {
                sample_heading(actor);
                result.heading_sampled = true;
            }
            status = dispatch(next, height, request, &result);
        }
    }
    if (status == 0) {
        status = diagnostic(next, request, &result);
    }
    if (status == 0) {
        *world = *next;
        *out = result;
    }
    free(next);
    return status;
}
