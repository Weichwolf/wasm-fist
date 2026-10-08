#include "mission_probe_io.h"

#include "assets/units.h"
#include "combat_probe_io.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum {
    SHELL = 8,
    MUZZLE = 18,
    RETIRING = 19,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    SMOKE = 17,
    TREE = 21,
    WRECK = 23,
    TEMPORARY_FIRST = 11,
    TEMPORARY_SECOND = 13,
    COUNTED_STATIC = 16,
    HEIGHT_STATIC = 25,
    TARGET = 26,
    ARTILLERY = 27
};

void fist_probe_write_tree(const fist_tree *tree) {
    printf("tree %u %u %u %ld %ld %ld %u %u %u %u %u %u %u\n", (unsigned)tree->allocation.slot,
           (unsigned)tree->allocation.registry_index, (unsigned)tree->allocation.value,
           (long)tree->pose.x, (long)tree->pose.y, (long)tree->pose.altitude,
           (unsigned)tree->pose.heading, (unsigned)tree->extent, (unsigned)tree->projection_scale,
           (unsigned)tree->flags, (unsigned)tree->secondary_flags, (unsigned)tree->ground_height,
           (unsigned)tree->variant);
}

int fist_probe_write_mission_world(const fist_mission_world *world) {
    if (world->orders_loaded > 1) {
        return -1;
    }
    fist_probe_write_object_pool(&world->pool);
    printf("random %u", (unsigned)world->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)world->random.words[index]);
    }
    printf("\nroster");
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        printf(" %u", (unsigned)world->combat.roster[index]);
    }
    printf("\n");
    fist_probe_write_orders(world->orders_loaded != 0 ? &world->orders : NULL);
    if (world->preparation.prepared != 0) {
        const fist_mission_preparation *state = &world->preparation;
        printf("preparation %u %u %u %u\n", (unsigned)state->trees, (unsigned)state->counted_static,
               (unsigned)state->artillery_count[0], (unsigned)state->artillery_count[1]);
        for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
            printf("artillery %zu", side);
            for (size_t entry = 0; entry < FIST_MISSION_ARTILLERY_SIDE_SLOTS; ++entry) {
                const fist_pool_allocation allocation = state->artillery[side][entry].allocation;
                printf(" %u %u %u %u", (unsigned)allocation.type, (unsigned)allocation.slot,
                       (unsigned)allocation.registry_index, (unsigned)allocation.value);
            }
            puts("");
        }
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        const fist_mission_object *object = fist_mission_world_object(world, (uint16_t)slot);
        if (world->pool.slots[slot].used == 0) {
            if (object != NULL) {
                return -1;
            }
            continue;
        }
        if (object == NULL) {
            return -1;
        }
        printf("object %u %u\n", (unsigned)slot, (unsigned)world->pool.slots[slot].type);
        switch (world->pool.slots[slot].type) {
        case 0:
        case 1:
        case 2:
        case 3:
        case RETIRING:
            fist_probe_write_vehicle_state(&object->vehicle);
            printf("damage %u %u\n", (unsigned)object->vehicle.damage,
                   (unsigned)object->vehicle.damage_alarm_countdown);
            break;
        case 4:
            fist_probe_write_explosion(&object->explosion);
            break;
        case SHELL:
            fist_probe_write_projectile(&object->projectile);
            break;
        case MUZZLE:
            fist_probe_write_muzzle(&object->muzzle);
            break;
        case TEMPORARY_FIRST:
        case TEMPORARY_SECOND:
        case COUNTED_STATIC:
        case HEIGHT_STATIC: {
            const fist_saved_object_base *base = &object->saved_base;
            printf("saved_base %u %u %u %u %ld %ld %ld %u %u %u %u %u %u %u\n",
                   (unsigned)base->allocation.type, (unsigned)base->allocation.slot,
                   (unsigned)base->allocation.registry_index, (unsigned)base->allocation.value,
                   (long)base->pose.x, (long)base->pose.y, (long)base->pose.altitude,
                   (unsigned)base->pose.heading, (unsigned)base->projection_extent,
                   (unsigned)base->projection_scale, (unsigned)base->flags,
                   (unsigned)base->secondary_flags, (unsigned)base->ground_height,
                   (unsigned)base->variant);
            break;
        }
        case FIRST_AIRCRAFT:
        case SECOND_AIRCRAFT:
        case TARGET:
        case ARTILLERY:
            fist_probe_write_other_actor(&object->other);
            break;
        case SMOKE:
            fist_probe_write_smoke(&object->smoke);
            break;
        case TREE:
            fist_probe_write_tree(&object->tree);
            break;
        case WRECK:
            fist_probe_write_wreck(&object->wreck);
            break;
        default:
            return -1;
        }
    }
    return 0;
}
