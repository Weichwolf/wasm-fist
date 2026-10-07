#include "sim/projectile_launch.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/rotation.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    M1_TYPE = 0,
    PROJECTILE_TYPE = 8,
    MUZZLE_TYPE = 18,
    PROJECTILE_SPEED = 853,
    PROJECTILE_HEIGHT = 2048,
    PROJECTILE_GRACE = 2,
    PROJECTILE_PARAMETER = 5,
    MUZZLE_FORWARD = 800,
    MUZZLE_HEIGHT = 2112,
    MUZZLE_SCALE = 512,
    HALF_TURN = 32768,
    SIDE_FLAG = 8,
    AMMO_COMPONENT = 37,
    COMPONENT_REFRESH = 3,
    RELOAD_COUNT = 20,
    RECOIL_COUNT = 16,
    SOUND_REQUEST = 12
};

static fist_projectile initialize_projectile(const fist_vehicle_state *vehicle,
                                             fist_launch_request request,
                                             fist_pool_allocation allocation) {
    return (fist_projectile){
        .allocation = allocation,
        .pose = {vehicle->map_x, vehicle->map_y,
                 fist_position_add(vehicle->altitude, PROJECTILE_HEIGHT), vehicle->turret.heading},
        .velocity = fist_rotate_spatial((fist_spatial_rotation){vehicle->turret.heading,
                                                                (uint16_t)vehicle->turret.elevation,
                                                                PROJECTILE_SPEED, request.coarse}),
        .speed = PROJECTILE_SPEED,
        .collision_grace = PROJECTILE_GRACE,
        .origin_slot = request.origin_slot,
        .target_slot = FIST_POOL_NO_SLOT,
        .flags = (uint8_t)(vehicle->object_flags & SIDE_FLAG),
        .launch_parameter = PROJECTILE_PARAMETER};
}

int fist_muzzle_smoke_restore(const fist_unit_definition *definition,
                              fist_pool_allocation allocation, fist_muzzle_smoke *out) {
    enum { SCALE = 20, FLAGS = 22, SECONDARY = 23, FRAME = 25, COUNTER = 26 };
    if (definition == NULL || out == NULL || definition->type != MUZZLE_TYPE ||
        allocation.type != MUZZLE_TYPE || allocation.slot >= FIST_POOL_SHORT_SLOTS ||
        allocation.registry_index != definition->registry_index ||
        allocation.registry_index >= FIST_UNIT_REGISTRY_COUNT ||
        allocation.value != definition->generation || definition->snapshot.data == NULL ||
        definition->snapshot.size != FIST_UNIT_SHORT_SIZE ||
        fist_read_u16le(definition->snapshot.data) != MUZZLE_TYPE) {
        return -1;
    }
    const uint8_t *raw = definition->snapshot.data;
    *out = (fist_muzzle_smoke){
        .allocation = allocation,
        .pose = {definition->map_x, definition->map_y, definition->altitude, definition->heading},
        .projection_scale = fist_read_u16le(raw + SCALE),
        .animation_counter = fist_read_u16le(raw + COUNTER),
        .animation_frame = raw[FRAME],
        .flags = raw[FLAGS],
        .secondary_flags = raw[SECONDARY]};
    return 0;
}

static fist_muzzle_smoke initialize_muzzle(const fist_vehicle_state *vehicle,
                                           fist_launch_request request,
                                           fist_pool_allocation allocation) {
    const fist_velocity forward =
        fist_rotate((fist_rotation){vehicle->turret.heading, MUZZLE_FORWARD, request.coarse});
    return (fist_muzzle_smoke){.allocation = allocation,
                               .pose = {fist_position_add(vehicle->map_x, forward.x),
                                        fist_position_add(vehicle->map_y, forward.y),
                                        fist_position_add(vehicle->altitude, MUZZLE_HEIGHT),
                                        (uint16_t)(vehicle->turret.heading + HALF_TURN)},
                               .projection_scale = MUZZLE_SCALE};
}

bool fist_m1_launch_source_valid(const fist_object_pool *pool, const fist_vehicle_state *vehicle,
                                 fist_launch_request request) {
    return fist_object_pool_is_valid(pool) && vehicle != NULL && vehicle->type == M1_TYPE &&
           vehicle->component_size == fist_vehicle_component_size(M1_TYPE) &&
           request.origin_slot >= FIST_POOL_SHORT_SLOTS &&
           request.origin_slot < FIST_UNIT_REGISTRY_COUNT &&
           pool->slots[request.origin_slot].used != 0 &&
           pool->slots[request.origin_slot].type == M1_TYPE;
}

int fist_m1_launch_untargeted(fist_object_pool *pool, fist_vehicle_state *vehicle,
                              fist_launch_request request, fist_launch_result *out) {
    if (out == NULL || !fist_m1_launch_source_valid(pool, vehicle, request)) {
        return -1;
    }
    fist_launch_result result = {.outcome = FIST_LAUNCH_EMPTY,
                                 .sound_request = FIST_LAUNCH_NO_SOUND};
    if (vehicle->weapons.rounds[0] == 0) {
        *out = result;
        return 0;
    }
    fist_object_pool updated = *pool;
    fist_vehicle_state actor = *vehicle;
    --actor.weapons.rounds[0];
    actor.components[AMMO_COMPONENT] = COMPONENT_REFRESH;
    fist_pool_allocation allocation = {0};
    const int status =
        fist_object_pool_allocate(&updated, (fist_pool_request){PROJECTILE_TYPE, 0}, &allocation);
    if (status < 0) {
        return -1;
    }
    result.outcome = FIST_LAUNCH_CAPACITY;
    if (status == FIST_POOL_OK) {
        result.projectile = initialize_projectile(&actor, request, allocation);
        const int muzzle_status =
            fist_object_pool_allocate(&updated, (fist_pool_request){MUZZLE_TYPE, 1}, &allocation);
        if (muzzle_status < 0) {
            return -1;
        }
        if (muzzle_status == FIST_POOL_OK) {
            result.muzzle = initialize_muzzle(&actor, request, allocation);
            result.has_muzzle = true;
        }
        actor.reload_countdown = RELOAD_COUNT;
        actor.weapons.recoil = RECOIL_COUNT;
        actor.weapons.trigger = 0;
        result.sound_request = SOUND_REQUEST;
        result.outcome = FIST_LAUNCH_FIRED;
    }
    *pool = updated;
    *vehicle = actor;
    *out = result;
    return 0;
}
