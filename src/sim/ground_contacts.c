#include "sim/mission_world.h"

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/damage_common.h"
#include "sim/geometry.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/tree.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    MOTION_BLOCKED = 8,
    CONTACT = 2,
    COLLIDABLE = 64,
    CONTACT_EXCLUDED = 16,
    DELETED = 1,
    TREE = 21,
    RADIUS_PADDING = 256,
    PREDICTION_GAP = 7680,
    REWIND_SCALE = 32,
    COOLDOWN = 4,
    TREE_THRESHOLD = 20,
    NON_TREE_SELECTOR = 45,
    TREE_SELECTOR = 48,
    SOUND_SAMPLES = 16,
    SOUND_SAMPLE_MASK = 255
};

/* Original 9b5d, shared arithmetic consumes its real random draw even though
 * spread is zero. Catalog factors belong to the existing combat owner. */
static const uint8_t tree_damage_record[2] = {100, 0};

static int collision_sound(fist_mission_world *world, fist_ground_contact_request request,
                           bool tree, fist_ground_contact_result *result) {
    if (request.sound_source == request.slot) {
        if (request.audio == NULL) {
            return -1;
        }
        const fist_contact_sound_record record =
            tree ? request.audio->tree : request.audio->obstacle;
        world->sound_selector = tree ? TREE_SELECTOR : NON_TREE_SELECTOR;
        if ((record.packet & SOUND_SAMPLE_MASK) < SOUND_SAMPLES) {
            result->sound = (fist_voice_request){
                .ax = record.packet, .dx = record.attenuation >> 1, .emitted = true};
        }
    }
    return 0;
}

static int tree_contact(fist_mission_world *world, uint16_t slot,
                        fist_ground_contact_result *result) {
    fist_tree *tree = &world->objects[result->candidate].tree;
    fist_pool_allocation allocation = {0};
    if (world->random.next_stream >= FIST_RANDOM_STREAMS ||
        fist_object_pool_find(&world->pool, result->candidate, &allocation) != FIST_POOL_OK ||
        tree->allocation.type != TREE || tree->allocation.slot != allocation.slot ||
        tree->allocation.registry_index != allocation.registry_index ||
        tree->allocation.value != allocation.value) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    /* Portable original signed SAR: C division truncates negative odd values
     * toward zero, while the instruction rounds them toward negative infinity. */
    actor->drive.speed = (int16_t)(((int32_t)actor->drive.speed -
                                    (actor->drive.speed < 0 && actor->drive.speed % 2 != 0)) /
                                   2);
    actor->drive.throttle =
        (int16_t)(((int32_t)actor->drive.throttle -
                   (actor->drive.throttle < 0 && actor->drive.throttle % 2 != 0)) /
                  2);
    const uint16_t roll = fist_damage_base_roll(&world->random, tree_damage_record);
    const uint16_t damage =
        fist_damage_scale_word(roll, world->combat.source_scale[world->combat.damage_source_enemy]);
    tree->damage = (uint8_t)(tree->damage + damage);
    if (tree->damage >= TREE_THRESHOLD) {
        fist_pool_allocation released = {0};
        if (fist_object_pool_release(&world->pool, allocation.registry_index, &released) !=
                FIST_POOL_OK ||
            released.slot != allocation.slot) {
            return -1;
        }
        tree->flags |= DELETED;
        world->preparation.trees = (uint16_t)(world->preparation.trees - 1U);
        result->tree_released = true;
    }
    result->refresh_damage_display = true;
    return 0;
}

static int first_contact(fist_mission_world *world, fist_ground_contact_request request,
                         fist_ground_contact_result *result) {
    fist_vehicle_state *actor = &world->objects[request.slot].vehicle;
    actor->contact_flags |= CONTACT;
    const bool tree = world->pool.slots[result->candidate].type == TREE;
    if (collision_sound(world, request, tree, result) != 0) {
        return -1;
    }
    if (tree) {
        return tree_contact(world, request.slot, result);
    }
    actor->map_x = fist_position_add(actor->map_x, -actor->drive.velocity_x * REWIND_SCALE);
    actor->map_y = fist_position_add(actor->map_y, -actor->drive.velocity_y * REWIND_SCALE);
    /* Negating the minimum signed word retains its original bit pattern. */
    if (actor->drive.speed != INT16_MIN) {
        actor->drive.speed = (int16_t)-actor->drive.speed;
    }
    actor->drive.throttle = 0;
    actor->contact_cooldown = COOLDOWN;
    return 0;
}

static int scan_contacts(fist_mission_world *world, fist_ground_contact_request request,
                         fist_ground_contact_result *result) {
    fist_vehicle_state *actor = &world->objects[request.slot].vehicle;
    actor->control_flags &= (uint16_t)~MOTION_BLOCKED;
    if (actor->contact_cooldown != 0) {
        --actor->contact_cooldown;
        return 0;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const uint16_t candidate = world->pool.registry[index].slot;
        if (candidate == FIST_POOL_NO_SLOT || candidate == request.slot) {
            continue;
        }
        fist_object_pose storage = {0};
        fist_mission_view view = {0};
        if (fist_mission_world_view(world, candidate, &storage, &view) != 0) {
            return -1;
        }
        if ((view.flags & COLLIDABLE) == 0 || (view.flags & CONTACT_EXCLUDED) != 0) {
            continue;
        }
        const uint32_t separation =
            fist_planar_proximity((fist_order_waypoint){view.pose->x, view.pose->y},
                                  (fist_order_waypoint){actor->map_x, actor->map_y});
        if (separation > UINT16_MAX) {
            continue;
        }
        const uint16_t margin =
            (uint16_t)(actor->projection_scale + view.projection_scale + RADIUS_PADDING) / 2;
        if (separation >= margin) {
            if (separation - margin <= PREDICTION_GAP &&
                fist_mission_world_observe_obstacle(
                    world, (fist_obstacle_observation){request.slot, candidate, request.coarse}) !=
                    0) {
                return -1;
            }
            continue;
        }
        result->hit = true;
        result->candidate = candidate;
        if ((actor->contact_flags & CONTACT) != 0) {
            actor->contact_cooldown = COOLDOWN;
            return 0;
        }
        return first_contact(world, request, result);
    }
    actor->contact_flags &= (uint8_t)~CONTACT;
    return 0;
}

int fist_mission_world_ground_contacts(fist_mission_world *world,
                                       fist_ground_contact_request request,
                                       fist_ground_contact_result *out) {
    if (out == NULL || fist_mission_world_object(world, request.slot) == NULL ||
        world->pool.slots[request.slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        world->objects[request.slot].vehicle.type != world->pool.slots[request.slot].type) {
        return -1;
    }
    fist_ground_contact_result result = {.candidate = FIST_POOL_NO_SLOT};
    const bool selected = world->combat.selected_slot == request.slot;
    if (selected != request.selected_wrapper) {
        *out = result;
        return 0;
    }
    fist_mission_world *next = malloc(sizeof(*next));
    if (next == NULL) {
        return -1;
    }
    *next = *world;
    result.admitted = true;
    const int status = scan_contacts(next, request, &result);
    if (status == 0) {
        *world = *next;
        *out = result;
    }
    free(next);
    return status;
}
