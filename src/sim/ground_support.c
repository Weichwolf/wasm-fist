#include "sim/mission_world.h"

#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "sim/ground_support.h"
#include "sim/other_damage.h"
#include "sim/vehicle_damage.h"
#include "sim/voice.h"

#include "sim/ground.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/rotation.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    SIDE = 8,
    ENEMY_SIDE = 1,
    COOLDOWN = 480,
    REQUEST_INTERVAL = 1800,
    MINIMUM_RANGE = 20,
    RETREAT = 4,
    SMOKE_HIGH_LIMIT = 64,
    RETREAT_LOW_LIMIT = 76,
    ORDINARY_LOW_LIMIT = 12,
    ARTILLERY_BIT = 32,
    SMOKE_OFFSET = 128,
    POSITION_FACTOR = 32,
    T80 = 2,
    BMP = 3,
    SMOKE_COMPONENT = 38,
    COMPONENT_REFRESH = 3,
    SMOKE_COUNTDOWN = 8,
    SMOKE_SELECTOR = 39,
    SMOKE_PACKET = 11,
    ARTILLERY = 27,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    DISPLAY_TICKS = 120,
    NOTICE_TICKS = 180,
    NOTICE_SUPPRESSED = 2,
    AIR_CONFIRM_CUE = 37,
    AIR_UNAVAILABLE_CUE = 39,
    ARTILLERY_CONFIRM_CUE = 41,
    ARTILLERY_UNAVAILABLE_CUE = 42,
    ARTILLERY_PHASE = 2,
    MESSAGE_TICKS = 360
};

typedef struct {
    fist_vehicle_state actor;
    fist_object_pool pool;
    fist_random random;
    fist_mission_support support;
    fist_support_marker marker;
    fist_target_notice display;
    fist_timed_advisory advisory;
    fist_ground_support_result result;
    uint16_t gun_rounds;
    bool gun_changed;
} support_transaction;

int fist_mission_world_configure_support(fist_mission_world *world,
                                         const fist_support_configuration *configuration,
                                         uint16_t clock) {
    if (world == NULL || configuration == NULL || !fist_object_pool_is_valid(&world->pool)) {
        return -1;
    }
    const fist_support_configuration supplied = *configuration;
    fist_mission_support next = world->support;
    next.configuration = supplied;
    next.configured = true;
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        next.air_clock[side] = (uint16_t)(clock - COOLDOWN);
        next.artillery_clock[side] = (uint16_t)(clock - COOLDOWN);
        next.air_type[side] =
            (side != 0) != supplied.reverse_aircraft ? SECOND_AIRCRAFT : FIRST_AIRCRAFT;
        for (size_t index = 0; index < FIST_SUPPORT_QUEUE_SLOTS; ++index) {
            next.air[side][index].requester = (fist_object_reference){0};
            next.artillery[side][index].phase = 0;
        }
    }
    world->support = next;
    return 0;
}

static void display(support_transaction *next, const fist_mission_world *world,
                    fist_ground_support_request request, uint8_t kind) {
    if (world->combat.selected_slot == request.slot) {
        next->display = (fist_target_notice){.duration = DISPLAY_TICKS, .kind = kind};
    }
}

static void notice(support_transaction *next, const fist_mission_world *world,
                   fist_ground_support_request request, uint8_t code) {
    if (world->combat.selected_slot == request.slot &&
        request.notice_context != NOTICE_SUPPRESSED) {
        next->advisory =
            (fist_timed_advisory){(uint16_t)(request.clock + NOTICE_TICKS), code, true};
        next->result.notice = true;
    }
}

static int smoke_stock(support_transaction *next) {
    if (next->actor.type == BMP) {
        return 1;
    }
    uint16_t *stock = &next->actor.weapons.class_parameter;
    if (next->actor.type != T80 && *stock > UINT8_MAX) {
        return -1;
    }
    if (*stock == 0) {
        return 0;
    }
    --*stock;
    if (next->actor.type == T80) {
        next->actor.components[SMOKE_COMPONENT] = COMPONENT_REFRESH;
    }
    return 1;
}

static int create_smoke(support_transaction *next, const fist_mission_world *world,
                        const fist_klc_image *height, fist_ground_support_request request) {
    const int stock = smoke_stock(next);
    if (stock < 0) {
        return -1;
    }
    int allocated = FIST_POOL_UNAVAILABLE;
    if (stock != 0) {
        allocated = fist_object_pool_allocate(
            &next->pool, (fist_pool_request){FIST_SUPPORT_SMOKE_TYPE, 0}, &next->result.marker);
        if (allocated < 0) {
            return -1;
        }
    }
    if (allocated == FIST_POOL_UNAVAILABLE) {
        next->result.smoke = stock == 0 ? FIST_SUPPORT_SMOKE_EMPTY : FIST_SUPPORT_SMOKE_CAPACITY;
        display(next, world, request, FIST_NOTICE_SMOKE_EMPTY);
        return 0;
    }
    const fist_velocity rotated =
        fist_rotate((fist_rotation){next->actor.drive.heading, SMOKE_OFFSET, request.coarse});
    const int32_t delta_x = ((int32_t)rotated.x + next->actor.drive.velocity_x) * POSITION_FACTOR;
    const int32_t delta_y = ((int32_t)rotated.y + next->actor.drive.velocity_y) * POSITION_FACTOR;
    fist_support_marker marker = {.allocation = next->result.marker,
                                  .pose = {.x = fist_position_add(next->actor.map_x, delta_x),
                                           .y = fist_position_add(next->actor.map_y, delta_y)}};
    uint8_t sampled = 0;
    if (fist_ground_height_sample(height, marker.pose.x, marker.pose.y, &sampled) != 0) {
        return -1;
    }
    marker.pose.altitude = fist_altitude_set_height(0, sampled);
    next->marker = marker;
    next->result.smoke = FIST_SUPPORT_SMOKE_CREATED;
    if (next->actor.type == T80) {
        next->actor.reload_countdown = SMOKE_COUNTDOWN;
    }
    if (request.sound_source == request.slot) {
        next->result.sound = (fist_voice_request){.ax = SMOKE_PACKET, .emitted = true};
    }
    return 0;
}

static int air_support(support_transaction *next, const fist_mission_world *world,
                       fist_ground_support_request request) {
    if ((uint16_t)(request.clock - next->support.air_clock[ENEMY_SIDE]) < COOLDOWN) {
        next->result.support = FIST_SUPPORT_AIR_COOLDOWN;
        return 0;
    }
    if (next->support.configuration.air_stock[ENEMY_SIDE] != 0 &&
        next->support.configuration.air_delay[ENEMY_SIDE] == 0) {
        next->support.air_clock[ENEMY_SIDE] = request.clock;
        for (size_t index = 0; index < FIST_SUPPORT_QUEUE_SLOTS; ++index) {
            fist_air_support_entry *entry = &next->support.air[ENEMY_SIDE][index];
            if (entry->requester.lifetime == 0) {
                if (fist_object_pool_reference(&next->pool, request.slot, &entry->requester) != 0) {
                    return -1;
                }
                entry->tick = request.tick;
                next->result.queue_index = (uint8_t)index;
                next->result.support = FIST_SUPPORT_AIR_CONFIRMED;
                display(next, world, request, FIST_NOTICE_AIR_CONFIRMED);
                notice(next, world, request, AIR_CONFIRM_CUE);
                return 0;
            }
        }
    }
    next->result.support = FIST_SUPPORT_AIR_UNAVAILABLE;
    notice(next, world, request, AIR_UNAVAILABLE_CUE);
    display(next, world, request, FIST_NOTICE_AIR_UNAVAILABLE);
    return 0;
}

static int choose_artillery(support_transaction *next, const fist_mission_world *world) {
    const uint16_t count = world->preparation.artillery_count[ENEMY_SIDE];
    if (count > FIST_MISSION_ARTILLERY_SIDE_SLOTS) {
        return -1;
    }
    for (size_t index = 0; index < count; ++index) {
        const fist_artillery_resource resource = world->preparation.artillery[ENEMY_SIDE][index];
        if (!fist_object_reference_is_valid(resource.reference) ||
            resource.reference.lifetime == 0 ||
            resource.reference.slot != resource.allocation.slot ||
            resource.allocation.type != ARTILLERY) {
            return -1;
        }
        if (!fist_object_pool_reference_is_live(&next->pool, resource.reference)) {
            continue;
        }
        if (next->pool.slots[resource.reference.slot].type != ARTILLERY) {
            return -1;
        }
        const fist_other_actor *gun = &world->objects[resource.reference.slot].other;
        if (gun->allocation.type != ARTILLERY || gun->allocation.slot != resource.reference.slot) {
            return -1;
        }
        if (gun->state.type27.rounds != 0) {
            next->result.resource_slot = resource.reference.slot;
            next->gun_rounds = (uint16_t)(gun->state.type27.rounds - 1);
            next->gun_changed = true;
            return 1;
        }
    }
    next->result.support =
        count == 0 ? FIST_SUPPORT_ARTILLERY_NOT_IN_PLACE : FIST_SUPPORT_ARTILLERY_EMPTY;
    return 0;
}

static int artillery_support(support_transaction *next, const fist_mission_world *world,
                             fist_ground_support_request request) {
    if ((uint16_t)(request.clock - next->support.artillery_clock[ENEMY_SIDE]) < COOLDOWN) {
        next->result.support = FIST_SUPPORT_ARTILLERY_COOLDOWN;
        return 0;
    }
    const int chosen = choose_artillery(next, world);
    if (chosen < 0) {
        return -1;
    }
    if (chosen == 0) {
        notice(next, world, request, ARTILLERY_UNAVAILABLE_CUE);
        display(next, world, request, FIST_NOTICE_ARTILLERY_UNAVAILABLE);
        return 0;
    }
    /* Typed per-side storage repairs the original index2/3 unaligned write. */
    next->support.artillery_clock[ENEMY_SIDE] = request.clock;
    display(next, world, request, FIST_NOTICE_ARTILLERY_CONFIRMED);
    if (next->support.configuration.artillery_delay[ENEMY_SIDE] != 0) {
        next->result.support = FIST_SUPPORT_ARTILLERY_BUSY;
        notice(next, world, request, ARTILLERY_UNAVAILABLE_CUE);
        display(next, world, request, FIST_NOTICE_ARTILLERY_UNAVAILABLE);
        return 0;
    }
    for (size_t index = 0; index < FIST_SUPPORT_QUEUE_SLOTS; ++index) {
        fist_artillery_support_entry *entry = &next->support.artillery[ENEMY_SIDE][index];
        if (entry->phase == 0) {
            fist_object_pose ground = {0};
            fist_mission_view target = {0};
            if (fist_mission_world_view(world, next->actor.command.target.slot, &ground, &target) !=
                0) {
                return -1;
            }
            entry->phase = ARTILLERY_PHASE;
            entry->clock = request.clock;
            entry->target = (fist_order_waypoint){target.pose->x, target.pose->y};
            next->result.queue_index = (uint8_t)index;
            next->result.support = FIST_SUPPORT_ARTILLERY_CONFIRMED;
            next->result.message = true;
            notice(next, world, request, ARTILLERY_CONFIRM_CUE);
            return 0;
        }
    }
    next->result.support = FIST_SUPPORT_ARTILLERY_QUEUE_FULL;
    return 0;
}

static void commit_support(fist_mission_world *world, fist_ground_support_request request,
                           const support_transaction *next, fist_ground_support_result *out) {
    world->objects[request.slot].vehicle = next->actor;
    world->pool = next->pool;
    world->random = next->random;
    world->support = next->support;
    world->target_notice = next->display;
    world->advisory = next->advisory;
    if (next->gun_changed) {
        world->objects[next->result.resource_slot].other.state.type27.rounds = next->gun_rounds;
    }
    if (next->result.smoke == FIST_SUPPORT_SMOKE_CREATED) {
        world->objects[next->result.marker.slot] =
            (fist_mission_object){.support_marker = next->marker};
    }
    if (next->result.sound.emitted) {
        world->sound_selector = SMOKE_SELECTOR;
    }
    if (next->result.message) {
        world->artillery_message_ticks = MESSAGE_TICKS;
    }
    *out = next->result;
}

int fist_mission_world_request_support(fist_mission_world *world, const fist_klc_image *height,
                                       fist_ground_support_request request,
                                       fist_ground_support_result *out) {
    if (out == NULL || fist_mission_world_object(world, request.slot) == NULL ||
        world->preparation.prepared != 1 ||
        world->pool.slots[request.slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    support_transaction next = {.actor = world->objects[request.slot].vehicle,
                                .pool = world->pool,
                                .random = world->random,
                                .support = world->support,
                                .display = world->target_notice,
                                .advisory = world->advisory,
                                .result = {.marker = {.slot = FIST_POOL_NO_SLOT},
                                           .resource_slot = FIST_POOL_NO_SLOT,
                                           .queue_index = FIST_SUPPORT_NO_QUEUE}};
    if (next.actor.type != world->pool.slots[request.slot].type ||
        next.actor.component_size != fist_vehicle_component_size(next.actor.type) ||
        !fist_object_reference_is_valid(next.actor.command.target)) {
        return -1;
    }
    if (!fist_object_pool_reference_is_live(&next.pool, next.actor.command.target)) {
        next.actor.command.target = (fist_object_reference){0};
    }
    if ((next.actor.object_flags & SIDE) == 0 || next.actor.command.target.lifetime == 0 ||
        next.actor.command.target_range < MINIMUM_RANGE ||
        (uint16_t)(request.clock - next.support.last_request) < REQUEST_INTERVAL) {
        commit_support(world, request, &next, out);
        return 0;
    }
    uint16_t value = 0;
    if (fist_random_next(&next.random, &value) != 0) {
        return -1;
    }
    const bool retreat = next.actor.command.mode == RETREAT;
    const bool smoke = retreat && (value >> 8) <= SMOKE_HIGH_LIMIT;
    if (smoke && create_smoke(&next, world, height, request) != 0) {
        return -1;
    }
    const uint8_t limit = retreat ? RETREAT_LOW_LIMIT : ORDINARY_LOW_LIMIT;
    if (!smoke && (uint8_t)value > limit) {
        commit_support(world, request, &next, out);
        return 0;
    }
    if (!next.support.configured) {
        return -1;
    }
    next.support.last_request = request.clock;
    const int status = smoke || (value & ARTILLERY_BIT) != 0
                           ? artillery_support(&next, world, request)
                           : air_support(&next, world, request);
    if (status != 0) {
        return status;
    }
    commit_support(world, request, &next, out);
    return 0;
}
