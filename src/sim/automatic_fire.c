#include "sim/automatic_fire.h"

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"
#include "sim/weapon_control.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    M3 = 1,
    BMP = 3,
    SIDE = 8,
    AIRBORNE = 16,
    OVERRIDE = 128,
    PASSIVE_BEHAVIOR = 3,
    PHASE_MASK = 3,
    NEAR_RANGE = 200,
    POSITIVE_WINDOW = 181,
    NEGATIVE_WINDOW = 65354,
    READY = 4,
    ARMING = 2,
    COMPONENT_REFRESH = 3,
    M3_FIRST_COMPONENT = 36,
    BMP_FIRST_COMPONENT = 10,
    COMPONENT_STEP = 2,
    NOTICE_DURATION = 120,
    MISSILE_ALTITUDE = 3072,
    MISSILE_SPEED = 160,
    MISSILE_STEERING = 364,
    MISSILE_SECONDARY = 1,
    SOUND_SELECTOR = 24,
    SOUND_PACKET = 7
};

/* Actual authored DS:9952. Index admission belongs to the used branch. */
static const uint8_t probabilities[] = {80, 140, 40, 0};

typedef struct {
    fist_vehicle_state actor;
    fist_object_pool pool;
    fist_surface_air_missile missile;
    fist_target_notice notice;
    fist_automatic_fire_result result;
} fire_transaction;

static fist_vehicle_state *fire_actor(fist_mission_world *world, uint16_t slot) {
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

static bool missile_class(uint16_t type) {
    return type == M3 || type == BMP;
}

static bool target_present(const fist_vehicle_state *actor) {
    return actor->command.target.lifetime != 0 || actor->command.target_reference != 0;
}

static int target_view(const fist_mission_world *world, const fist_vehicle_state *actor,
                       fist_mission_view *out, fist_object_pose *storage) {
    const fist_object_reference target = actor->command.target;
    if (!fist_object_pool_reference_is_live(&world->pool, target)) {
        return -1;
    }
    return fist_mission_world_view(world, target.slot, storage, out);
}

static fire_transaction begin_fire(const fist_mission_world *world,
                                   const fist_vehicle_state *actor) {
    return (fire_transaction){.actor = *actor,
                              .pool = world->pool,
                              .notice = world->target_notice,
                              .result = {.missile = {FIST_UNIT_TYPE_COUNT, FIST_POOL_NO_SLOT,
                                                     FIST_UNIT_REGISTRY_COUNT, 0},
                                         .rack = FIST_SURFACE_AIR_NO_RACK}};
}

static void failure_notice(const fist_mission_world *world, fist_surface_air_request request,
                           uint8_t kind, fire_transaction *next) {
    if (world->combat.selected_slot == request.slot) {
        next->notice = (fist_target_notice){
            .duration = NOTICE_DURATION, .type = FIST_UNIT_TYPE_COUNT, .kind = kind};
        next->result.message = true;
    }
}

static uint8_t ready_rack(const fist_vehicle_state *actor) {
    if (actor->weapons.rack_state[0] == READY &&
        (actor->type == BMP || actor->weapons.cycle[0] != 0)) {
        return 0;
    }
    return actor->weapons.rack_state[1] == READY ? 1 : FIST_SURFACE_AIR_NO_RACK;
}

static uint8_t empty_rack(const fist_vehicle_state *actor) {
    if (actor->weapons.rack_state[0] == 0 && (actor->type == M3 || actor->weapons.cycle[0] != 0)) {
        return 0;
    }
    return actor->weapons.rack_state[1] == 0 ? 1 : FIST_SURFACE_AIR_NO_RACK;
}

static int launch_missile(const fist_mission_world *world, fist_surface_air_request request,
                          fire_transaction *next) {
    const uint8_t rack = next->result.rack;
    if (next->actor.weapons.cycle[rack] == 0) {
        failure_notice(world, request, FIST_NOTICE_SAM_EMPTY, next);
        return 0;
    }
    if (!target_present(&next->actor)) {
        failure_notice(world, request, FIST_NOTICE_SAM_TARGET, next);
        return 0;
    }
    fist_object_pose storage = {0};
    fist_mission_view view = {0};
    if (target_view(world, &next->actor, &view, &storage) != 0) {
        return -1;
    }
    if ((view.flags & AIRBORNE) == 0) {
        failure_notice(world, request, FIST_NOTICE_SAM_TARGET, next);
        return 0;
    }
    fist_object_reference origin = {0};
    if (fist_object_pool_reference(&world->pool, request.slot, &origin) != 0) {
        return -1;
    }
    --next->actor.weapons.cycle[rack];
    const int allocated = fist_object_pool_allocate(
        &next->pool, (fist_pool_request){FIST_SURFACE_AIR_TYPE, 0}, &next->result.missile);
    if (allocated < 0) {
        return -1;
    }
    if (allocated == FIST_POOL_UNAVAILABLE) {
        failure_notice(world, request, FIST_NOTICE_SAM_CAPACITY, next);
        return 0;
    }
    next->missile = (fist_surface_air_missile){
        .allocation = next->result.missile,
        .pose = {next->actor.map_x, next->actor.map_y,
                 fist_position_add(next->actor.altitude, MISSILE_ALTITUDE),
                 next->actor.turret.heading},
        .origin = origin,
        .target = next->actor.command.target,
        .steering = MISSILE_STEERING,
        .speed = MISSILE_SPEED,
        .secondary_flags = MISSILE_SECONDARY};
    next->result.launched = true;
    if (request.sound_source == request.slot) {
        next->result.sound = (fist_voice_request){.ax = SOUND_PACKET, .emitted = true};
    }
    return 0;
}

static int service_racks(const fist_mission_world *world, fist_surface_air_request request,
                         fire_transaction *next) {
    next->result.rack = ready_rack(&next->actor);
    if (next->result.rack != FIST_SURFACE_AIR_NO_RACK) {
        if (launch_missile(world, request, next) != 0) {
            return -1;
        }
    } else {
        next->result.rack = empty_rack(&next->actor);
        if (next->result.rack == FIST_SURFACE_AIR_NO_RACK) {
            return 0;
        }
        next->actor.weapons.rack_state[next->result.rack] = ARMING;
        next->result.armed = true;
    }
    const size_t first = next->actor.type == M3 ? M3_FIRST_COMPONENT : BMP_FIRST_COMPONENT;
    next->actor.components[first] = COMPONENT_REFRESH;
    next->actor.components[first + COMPONENT_STEP] = COMPONENT_REFRESH;
    return 0;
}

/* -1 is invalid used state, zero is rejected, one is admitted. */
static int fire_admission(const fist_mission_world *world, const fist_vehicle_state *actor,
                          uint16_t phase_random) {
    if (world->orders_loaded != 1 || actor->platoon >= FIST_UNIT_PLATOON_COUNT) {
        return -1;
    }
    const uint16_t behavior = world->orders.descriptors[actor->platoon].words[0];
    if (behavior == PASSIVE_BEHAVIOR || !target_present(actor)) {
        return 0;
    }
    const uint8_t phase = (uint8_t)phase_random;
    if ((actor->object_flags & SIDE) == 0 && (phase & PHASE_MASK) != 0) {
        return 0;
    }
    if (actor->command.target_range <= NEAR_RANGE || (actor->control_flags & OVERRIDE) != 0) {
        return 1;
    }
    if (behavior >= sizeof(probabilities) / sizeof(probabilities[0])) {
        return -1;
    }
    return phase < probabilities[behavior] ? 1 : 0;
}

static int automatic_fire(const fist_mission_world *world, fist_automatic_fire_request request,
                          fire_transaction *next) {
    if (request.missile_only && !missile_class(next->actor.type)) {
        return 0;
    }
    const int admission = fire_admission(world, &next->actor, request.phase_random);
    if (admission <= 0) {
        return admission;
    }
    fist_object_pose storage = {0};
    fist_mission_view view = {0};
    if (target_view(world, &next->actor, &view, &storage) != 0) {
        return -1;
    }
    if ((view.flags & AIRBORNE) != 0 && missile_class(next->actor.type)) {
        return service_racks(world, request.source, next);
    }
    const uint16_t difference =
        (uint16_t)(next->actor.turret.requested_offset - next->actor.turret.offset);
    if (difference <= POSITIVE_WINDOW || difference >= NEGATIVE_WINDOW) {
        if (fist_weapon_request_fire(&next->actor) != 0) {
            return -1;
        }
        next->actor.control_flags &= (uint16_t)~OVERRIDE;
        next->result.requested = true;
    }
    return 0;
}

static void publish_fire(fist_mission_world *world, uint16_t slot, const fire_transaction *next,
                         fist_automatic_fire_result *out) {
    world->objects[slot].vehicle = next->actor;
    if (next->result.launched) {
        world->pool = next->pool;
        world->objects[next->result.missile.slot] =
            (fist_mission_object){.surface_air = next->missile};
    }
    if (next->result.message) {
        world->target_notice = next->notice;
    }
    if (next->result.sound.emitted) {
        world->sound_selector = SOUND_SELECTOR;
    }
    *out = next->result;
}

int fist_mission_world_automatic_fire(fist_mission_world *world,
                                      fist_automatic_fire_request request,
                                      fist_automatic_fire_result *out) {
    fist_vehicle_state *actor = fire_actor(world, request.source.slot);
    if (actor == NULL || out == NULL) {
        return -1;
    }
    fire_transaction next = begin_fire(world, actor);
    if (automatic_fire(world, request, &next) != 0) {
        return -1;
    }
    publish_fire(world, request.source.slot, &next, out);
    return 0;
}

int fist_mission_world_ready_surface_air(fist_mission_world *world,
                                         fist_surface_air_request request,
                                         fist_automatic_fire_result *out) {
    fist_vehicle_state *actor = fire_actor(world, request.slot);
    if (actor == NULL || out == NULL || !missile_class(actor->type)) {
        return -1;
    }
    fire_transaction next = begin_fire(world, actor);
    if (service_racks(world, request, &next) != 0) {
        return -1;
    }
    publish_fire(world, request.slot, &next, out);
    return 0;
}
