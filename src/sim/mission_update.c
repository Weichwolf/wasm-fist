#include "sim/mission_update.h"

#include "assets/units.h"
#include "sim/aircraft_death.h"
#include "sim/collision.h"
#include "sim/destruction_updates.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    EXPLOSION = 4,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    SHELL = 8,
    SMOKE = 17,
    MUZZLE = 18,
    RETIRING = 19,
    TREE = 21,
    WRECK = 23,
    TARGET = 26,
    ARTILLERY = 27,
    DESTROYED_TARGET = 4,
    AIRCRAFT_DEATH = 12
};

static int collision_views(const fist_mission_world *world, fist_collision_body *bodies,
                           fist_object_pose *ground) {
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (world->pool.slots[slot].used == 0) {
            continue;
        }
        fist_mission_view view = {0};
        const int status = fist_mission_world_view(world, (uint16_t)slot, &ground[slot], &view);
        if (status != 0) {
            return status;
        }
        bodies[slot] =
            (fist_collision_body){view.pose, view.projection_scale, view.flags, view.mode};
    }
    return 0;
}

static void publish_explosion(fist_mission_world *world, const fist_explosion *effect) {
    world->objects[effect->allocation.slot] = (fist_mission_object){.explosion = *effect};
}

static void publish_smoke(fist_mission_world *world, const fist_drifting_smoke *smoke) {
    world->objects[smoke->allocation.slot] = (fist_mission_object){.smoke = *smoke};
}

static int damage(fist_mission_world *world, fist_projectile *source,
                  fist_mission_update_result *result) {
    const fist_damage_environment environment = {&world->pool, &world->random, &world->combat};
    const fist_vehicle_damage_request request = {source, result->flight.hit};
    fist_mission_object *target = &world->objects[request.hit.slot];
    const uint16_t type = world->pool.slots[request.hit.slot].type;
    result->target_type = type;
    int status = 0;
    if (type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        status =
            fist_vehicle_damage_m1(&target->vehicle, &environment, request, &result->ground_damage);
        if (status != 0) {
            return status;
        }
        for (size_t index = 0; index < result->ground_damage.explosion_count; ++index) {
            publish_explosion(world, &result->ground_damage.explosions[index]);
        }
        if (result->ground_damage.has_wreck) {
            const fist_vehicle_wreck *wreck = &result->ground_damage.wreck;
            world->objects[wreck->allocation.slot] = (fist_mission_object){.wreck = *wreck};
        }
        result->player_loss = result->ground_damage.selected_destroyed;
    } else {
        if (type == WRECK) {
            status = fist_wreck_damage_m1(&environment, request, &result->other_damage);
        } else if (type == FIRST_AIRCRAFT || type == SECOND_AIRCRAFT || type == TARGET ||
                   type == ARTILLERY) {
            status =
                fist_other_damage_m1(&target->other, &environment, request, &result->other_damage);
        } else {
            return FIST_MISSION_UNSUPPORTED;
        }
        if (status != 0) {
            return status;
        }
        if (result->other_damage.has_explosion) {
            publish_explosion(world, &result->other_damage.explosion);
        }
    }
    result->damaged = true;
    return 0;
}

static int finish_impact(fist_mission_world *world, fist_projectile *source,
                         fist_projectile_impact *out) {
    if (fist_projectile_finish_impact(&world->pool, source, out) != 0) {
        return -1;
    }
    if (out->has_explosion) {
        publish_explosion(world, &out->explosion);
    }
    return 0;
}

static int shell_visit(fist_mission_world *world, fist_projectile *source,
                       const fist_mission_update_environment *environment,
                       fist_mission_update_result *result) {
    fist_collision_body bodies[FIST_UNIT_REGISTRY_COUNT] = {0};
    fist_object_pose ground[FIST_UNIT_REGISTRY_COUNT] = {0};
    const int views = collision_views(world, bodies, ground);
    if (views != 0) {
        return views;
    }
    const fist_projectile_environment flight = {&world->pool,
                                                {&world->pool, bodies, FIST_UNIT_REGISTRY_COUNT},
                                                environment->height,
                                                &world->random};
    if (fist_projectile_advance(source, &flight, &result->flight) != 0) {
        return -1;
    }
    if (result->flight.phase == FIST_PROJECTILE_UNIT_IMPACT) {
        const int status = damage(world, source, result);
        if (status != 0) {
            return status;
        }
        if (result->player_loss) {
            world->pending_player_impact = source->allocation.slot;
            return 0;
        }
    }
    if (result->flight.phase == FIST_PROJECTILE_GROUND_IMPACT ||
        result->flight.phase == FIST_PROJECTILE_UNIT_IMPACT) {
        if (finish_impact(world, source, &result->impact) != 0) {
            return -1;
        }
        result->has_impact = true;
    }
    return 0;
}

static int visit(fist_mission_world *world, fist_pool_allocation allocation,
                 const fist_mission_update_environment *environment,
                 fist_mission_update_result *result) {
    fist_mission_object *object = &world->objects[allocation.slot];
    const fist_destruction_environment destruction = {&world->pool, &world->random,
                                                      environment->weather.enabled};
    int status = 0;
    switch (allocation.type) {
    case SHELL:
        return shell_visit(world, &object->projectile, environment, result);
    case EXPLOSION:
        return fist_explosion_advance(&world->pool, &object->explosion);
    case MUZZLE:
        return fist_muzzle_smoke_advance(&world->pool, &object->muzzle);
    case SMOKE:
        return fist_drifting_smoke_advance(&world->pool, &object->smoke, environment->weather);
    case RETIRING: {
        bool released = false;
        return fist_vehicle_retirement_advance(&world->pool, allocation, &object->vehicle,
                                               &released);
    }
    case TREE:
        return fist_tree_advance(&world->pool, &object->tree, environment->trees);
    case WRECK:
        status = fist_vehicle_wreck_advance(&object->wreck, &destruction, &result->destruction);
        break;
    case TARGET:
        if ((object->other.mode & DESTROYED_TARGET) == 0) {
            return FIST_MISSION_UNSUPPORTED;
        }
        status = fist_destroyed_target_advance(&object->other, &destruction, &result->destruction);
        break;
    case ARTILLERY:
        status = fist_destroyed_target_advance(&object->other, &destruction, &result->destruction);
        break;
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT: {
        if (object->other.state.pair.behavior != AIRCRAFT_DEATH) {
            return FIST_MISSION_UNSUPPORTED;
        }
        const fist_aircraft_environment aircraft = {&world->pool,
                                                    &world->random,
                                                    environment->height,
                                                    environment->tick,
                                                    environment->animation_phase,
                                                    environment->weather.enabled,
                                                    environment->coarse};
        status = fist_aircraft_death_advance(&object->other, &aircraft, &result->aircraft);
        if (status == 0 && result->aircraft.has_explosion) {
            publish_explosion(world, &result->aircraft.explosion);
        }
        if (status == 0 && result->aircraft.has_smoke) {
            publish_smoke(world, &result->aircraft.smoke);
        }
        return status;
    }
    default:
        return FIST_MISSION_UNSUPPORTED;
    }
    if (status == 0 && result->destruction.has_smoke) {
        publish_smoke(world, &result->destruction.smoke);
    }
    return status;
}

int fist_mission_world_visit(fist_mission_world *world, fist_pool_allocation allocation,
                             const fist_mission_update_environment *environment,
                             fist_mission_update_result *out) {
    if (world == NULL || environment == NULL || out == NULL ||
        world->pending_player_impact != FIST_POOL_NO_SLOT ||
        world->random.next_stream >= FIST_RANDOM_STREAMS ||
        !fist_object_pool_is_current(&world->pool, allocation)) {
        return -1;
    }
    fist_mission_world *next = malloc(sizeof(*next));
    if (next == NULL) {
        return -1;
    }
    *next = *world;
    fist_mission_update_result result = {.target_type = FIST_UNIT_TYPE_COUNT};
    const int status = visit(next, allocation, environment, &result);
    if (status == 0) {
        *world = *next;
        *out = result;
    }
    free(next);
    return status;
}

int fist_mission_world_resume_impact(fist_mission_world *world, fist_projectile_impact *out) {
    if (world == NULL || out == NULL ||
        fist_mission_world_object(world, world->pending_player_impact) == NULL ||
        world->pool.slots[world->pending_player_impact].type != SHELL ||
        world->objects[world->pending_player_impact].projectile.phase !=
            FIST_PROJECTILE_UNIT_IMPACT) {
        return -1;
    }
    fist_mission_world *next = malloc(sizeof(*next));
    if (next == NULL) {
        return -1;
    }
    *next = *world;
    fist_projectile_impact result = {0};
    const int status =
        finish_impact(next, &next->objects[next->pending_player_impact].projectile, &result);
    if (status == 0) {
        next->pending_player_impact = FIST_POOL_NO_SLOT;
        *world = *next;
        *out = result;
    }
    free(next);
    return status;
}
