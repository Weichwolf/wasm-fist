#include "assets/units.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/vehicle_motion.h"
#include "sim/vehicle_state.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    TARGET_TYPE = 26,
    FIRST_AIRCRAFT = 5,
    RETIRING = 19,
    ANGLE_MODES = 2,
    SOURCE_ALTITUDE = 4096,
    FIXTURE_SPEED = 51,
    FIXTURE_THROTTLE = 121,
    FIXTURE_HEADING = 65000,
    FIXTURE_REQUEST = 2000,
    FIXTURE_OFFSET = 17000,
    FIXTURE_TURRET_REQUEST = 31000,
    FIXTURE_TARGET_HEADING = 9000,
    FIXTURE_TARGET_RANGE = 65000,
    TARGET_Y = 7001,
    TARGET_ALTITUDE = 19000
};

enum { EMPTY_TARGET, RELEASED_TARGET, REUSED_TARGET, RETYPED_SUCCESSOR, RESET_TARGET, REPAIRS };
enum { FRESH_TARGET, ORPHAN_TARGET, RETYPED_TARGET, REBOUND_TARGET, SELF_TARGET, LIVE_CASES };
enum {
    NULL_WORLD,
    NULL_EVENTS,
    ABSENT_SLOT,
    OUTSIDE_SLOT,
    RELEASED_ACTOR,
    INVALID_POOL,
    UNPREPARED_WORLD,
    INVALID_PREPARATION,
    INVALID_ACTOR_TYPE,
    INVALID_COMPONENT_SIZE,
    INVALID_EMPTY_REFERENCE,
    INVALID_ABSENT_REFERENCE,
    INVALID_OUTSIDE_REFERENCE,
    FAILURES
};

enum {
    DRIVE_NULL_ACTOR,
    DRIVE_NULL_OUTPUT,
    TURRET_NULL_ACTOR,
    TURRET_NULL_OUTPUT,
    DRIVE_INVALID_COMPONENTS,
    TURRET_INVALID_COMPONENTS,
    STAGE_FAILURES
};

static int install(fist_mission_world *world, uint16_t kind, uint16_t target_kind,
                   fist_pool_allocation *source, fist_pool_allocation *target) {
    fist_mission_world_reset(world);
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){kind, 0}, source) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){target_kind, 0}, target) != 0) {
        return -1;
    }
    world->objects[source->slot].vehicle = (fist_vehicle_state){
        .type = kind,
        .registry_index = source->registry_index,
        .generation = source->value,
        .map_x = INT32_MAX,
        .map_y = INT32_MIN,
        .altitude = SOURCE_ALTITUDE,
        .component_size = fist_vehicle_component_size(kind),
        .drive = {.speed = FIXTURE_SPEED,
                  .throttle = FIXTURE_THROTTLE,
                  .heading = FIXTURE_HEADING,
                  .requested_heading = FIXTURE_REQUEST,
                  .movement_gate = UINT16_MAX},
        .turret = {.offset = FIXTURE_OFFSET, .requested_offset = FIXTURE_TURRET_REQUEST},
        .command = {.target_reference = UINT16_MAX,
                    .target_heading = FIXTURE_TARGET_HEADING,
                    .target_range = FIXTURE_TARGET_RANGE}};
    if (target_kind < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        world->objects[target->slot].vehicle =
            (fist_vehicle_state){.type = target_kind,
                                 .map_x = -SOURCE_ALTITUDE,
                                 .map_y = TARGET_Y,
                                 .altitude = TARGET_ALTITUDE,
                                 .component_size = fist_vehicle_component_size(target_kind)};
    } else {
        world->objects[target->slot].other = (fist_other_actor){
            .allocation = *target, .pose = {-SOURCE_ALTITUDE, TARGET_Y, TARGET_ALTITUDE, 0}};
    }
    world->preparation.prepared = 1;
    return fist_object_pool_reference(&world->pool, target->slot,
                                      &world->objects[source->slot].vehicle.command.target);
}

/* Target loss must be observationally identical to untargeted movement.
 * The numeric owner already has independent original comparisons; here the
 * comparison exposes unintended successor reads or unrelated-world writes. */
static int lost(fist_mission_world *world, fist_mission_world *expected, uint16_t slot,
                bool coarse) {
    fist_probe_capture(world, sizeof(*world), expected);
    fist_vehicle_state *actor = &expected->objects[slot].vehicle;
    actor->command.target = (fist_object_reference){0};
    fist_vehicle_motion_events wanted = {0};
    bool turret_changed = false;
    if (fist_vehicle_motion_drive_step(actor, coarse, &wanted) != 0 ||
        fist_vehicle_motion_turret_step(actor, &turret_changed) != 0) {
        return -1;
    }
    wanted.turret_changed = (uint8_t)turret_changed;
    fist_vehicle_motion_events actual = {0};
    if (fist_mission_world_ground_motion(world, slot, coarse, &actual) != 0 ||
        world->objects[slot].vehicle.command.target.lifetime != 0 ||
        world->objects[slot].vehicle.command.target.slot != 0) {
        return -1;
    }
    /* C11 does not specify padding in an assigned reference. Check both
     * semantic fields before copying its representation into the byte guard. */
    actor->command.target = world->objects[slot].vehicle.command.target;
    return fist_probe_unchanged(world, sizeof(*world), expected) &&
                   fist_probe_unchanged(&actual, sizeof(actual), &wanted)
               ? 0
               : -1;
}

static int repair(fist_mission_world *world, fist_mission_world *expected, uint16_t kind,
                  bool coarse, unsigned mode) {
    fist_pool_allocation source = {0};
    fist_pool_allocation target = {0};
    fist_pool_allocation removed = {0};
    if (install(world, kind, TARGET_TYPE, &source, &target) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[source.slot].vehicle;
    const fist_object_reference old = actor->command.target;
    if (mode == EMPTY_TARGET) {
        actor->command.target = (fist_object_reference){0};
    } else if (fist_object_pool_release(&world->pool, target.registry_index, &removed) != 0) {
        return -1;
    }
    if (mode == REUSED_TARGET || mode == RETYPED_SUCCESSOR) {
        const uint16_t replacement = mode == REUSED_TARGET ? TARGET_TYPE : FIRST_AIRCRAFT;
        if (fist_object_pool_allocate(&world->pool, (fist_pool_request){replacement, 0}, &target) !=
                0 ||
            target.slot != old.slot || target.registry_index != removed.registry_index ||
            target.value != removed.value) {
            return -1;
        }
        /* An incoherent successor is deliberately unusable if dereferenced. */
        world->objects[target.slot].other.allocation = (fist_pool_allocation){0};
    } else if (mode == RESET_TARGET) {
        const fist_vehicle_state saved = *actor;
        fist_mission_world_reset(world);
        if (fist_object_pool_allocate(&world->pool, (fist_pool_request){kind, 0}, &source) != 0 ||
            fist_object_pool_allocate(&world->pool, (fist_pool_request){TARGET_TYPE, 0}, &target) !=
                0 ||
            target.slot != old.slot) {
            return -1;
        }
        world->objects[source.slot].vehicle = saved;
        world->preparation.prepared = 1;
    }
    if (mode != EMPTY_TARGET && fist_object_pool_reference_is_live(&world->pool, old)) {
        return -1;
    }
    /* These dependencies are unused by movement, including after target loss. */
    world->random.next_stream = UINT8_MAX;
    world->objects[source.slot].vehicle.platoon = UINT8_MAX;
    world->orders_loaded = 0;
    return lost(world, expected, source.slot, coarse);
}

static int rejection(fist_mission_world *world, fist_mission_world *before, uint16_t kind,
                     bool coarse, unsigned mode) {
    fist_pool_allocation source = {0};
    fist_pool_allocation target = {0};
    fist_pool_allocation removed = {0};
    if (install(world, kind, TARGET_TYPE, &source, &target) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[source.slot].vehicle;
    uint16_t slot = source.slot;
    switch (mode) {
    case NULL_WORLD:
    case NULL_EVENTS:
        break;
    case ABSENT_SLOT:
        slot = FIST_POOL_NO_SLOT;
        break;
    case OUTSIDE_SLOT:
        slot = FIST_UNIT_REGISTRY_COUNT;
        break;
    case RELEASED_ACTOR:
        if (fist_object_pool_release(&world->pool, source.registry_index, &removed) != 0) {
            return -1;
        }
        break;
    case INVALID_POOL:
        ++world->pool.extended_count;
        break;
    case UNPREPARED_WORLD:
        world->preparation.prepared = 0;
        break;
    case INVALID_PREPARATION:
        world->preparation.prepared = UINT8_MAX;
        break;
    case INVALID_ACTOR_TYPE:
        actor->type = TARGET_TYPE;
        break;
    case INVALID_COMPONENT_SIZE:
        actor->component_size = 0;
        break;
    case INVALID_EMPTY_REFERENCE:
        actor->command.target = (fist_object_reference){.slot = 1};
        break;
    case INVALID_ABSENT_REFERENCE:
        actor->command.target = (fist_object_reference){.lifetime = 1, .slot = FIST_POOL_NO_SLOT};
        break;
    case INVALID_OUTSIDE_REFERENCE:
        actor->command.target =
            (fist_object_reference){.lifetime = 1, .slot = FIST_UNIT_REGISTRY_COUNT};
        break;
    default:
        return -1;
    }
    fist_vehicle_motion_events events = {UINT8_MAX, UINT8_MAX, UINT8_MAX};
    fist_vehicle_motion_events initial;
    fist_probe_capture(&events, sizeof(events), &initial);
    fist_probe_capture(world, sizeof(*world), before);
    return fist_mission_world_ground_motion(mode == NULL_WORLD ? NULL : world, slot, coarse,
                                            mode == NULL_EVENTS ? NULL : &events) == -1 &&
                   fist_probe_unchanged(world, sizeof(*world), before) &&
                   fist_probe_unchanged(&events, sizeof(events), &initial)
               ? 0
               : -1;
}

static int projection_failure(fist_mission_world *world, fist_mission_world *before, uint16_t kind,
                              bool coarse) {
    fist_pool_allocation source = {0};
    fist_pool_allocation target = {0};
    if (install(world, kind, TARGET_TYPE, &source, &target) != 0) {
        return -1;
    }
    world->objects[target.slot].other.allocation.slot = FIST_POOL_NO_SLOT;
    fist_vehicle_motion_events events = {UINT8_MAX, UINT8_MAX, UINT8_MAX};
    const fist_vehicle_motion_events initial = events;
    fist_probe_capture(world, sizeof(*world), before);
    /* T80/BMP must reject after their already-computed translation. M1/M3
     * consume retained heading and never inspect the target projection. */
    const int status = fist_mission_world_ground_motion(world, source.slot, coarse, &events);
    if (kind >= 2) {
        return status == -1 && fist_probe_unchanged(world, sizeof(*world), before) &&
                       fist_probe_unchanged(&events, sizeof(events), &initial)
                   ? 0
                   : -1;
    }
    before->objects[source.slot].vehicle = world->objects[source.slot].vehicle;
    return status == 0 && fist_probe_unchanged(world, sizeof(*world), before) &&
                   world->objects[source.slot].vehicle.command.target.lifetime != 0
               ? 0
               : -1;
}

static int live(fist_mission_world *world, fist_mission_world *expected, uint16_t kind, bool coarse,
                unsigned mode) {
    fist_pool_allocation source = {0};
    fist_pool_allocation target = {0};
    fist_pool_allocation other = {0};
    if (install(world, kind, mode == RETYPED_TARGET ? 0 : TARGET_TYPE, &source, &target) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[source.slot].vehicle;
    if (mode == ORPHAN_TARGET) {
        /* Overwrite the saved registry binding without freeing the target. */
        if (fist_object_pool_import(&world->pool,
                                    (fist_pool_import){TARGET_TYPE, target.registry_index, 1},
                                    &other) != 0 ||
            fist_object_pool_find(&world->pool, target.slot, &other) != FIST_POOL_UNAVAILABLE) {
            return -1;
        }
    } else if (mode == RETYPED_TARGET) {
        if (fist_object_pool_retype(&world->pool, target, RETIRING, &target) != 0) {
            return -1;
        }
        world->objects[target.slot].vehicle.type = RETIRING;
    } else if (mode == REBOUND_TARGET) {
        world->pool.registry[target.registry_index].value = UINT16_MAX;
    } else if (mode == SELF_TARGET) {
        if (fist_object_pool_reference(&world->pool, source.slot, &actor->command.target) != 0) {
            return -1;
        }
    }
    const fist_object_reference retained = actor->command.target;
    if (!fist_object_pool_reference_is_live(&world->pool, retained)) {
        return -1;
    }
    world->orders_loaded = 1;
    fist_probe_capture(world, sizeof(*world), expected);
    fist_vehicle_state *predicted = &expected->objects[source.slot].vehicle;
    fist_vehicle_motion_events wanted = {0};
    if (fist_vehicle_motion_drive_step(predicted, coarse, &wanted) != 0 ||
        (kind >= 2 && fist_mission_world_aim_target(expected, source.slot, coarse) != 0)) {
        return -1;
    }
    predicted->turret.requested_offset =
        (uint16_t)(predicted->command.target_heading - predicted->drive.heading);
    bool turret_changed = false;
    if (fist_vehicle_motion_turret_step(predicted, &turret_changed) != 0) {
        return -1;
    }
    wanted.turret_changed = (uint8_t)turret_changed;
    /* None of these fields participates in this stage, even for live aim. */
    world->random.next_stream = UINT8_MAX;
    expected->random.next_stream = UINT8_MAX;
    world->orders_loaded = 0;
    expected->orders_loaded = 0;
    actor->platoon = UINT8_MAX;
    predicted->platoon = UINT8_MAX;
    actor->command.candidate = (fist_object_reference){.slot = 1};
    predicted->command.candidate = actor->command.candidate;
    fist_vehicle_motion_events actual = {0};
    if (fist_mission_world_ground_motion(world, source.slot, coarse, &actual) != 0 ||
        actor->command.target.lifetime != retained.lifetime ||
        actor->command.target.slot != retained.slot) {
        return -1;
    }
    predicted->command.target = actor->command.target;
    return fist_probe_unchanged(world, sizeof(*world), expected) &&
                   fist_probe_unchanged(&actual, sizeof(actual), &wanted)
               ? 0
               : -1;
}

static int stage_rejection(fist_mission_world *world, fist_mission_world *before, uint16_t kind,
                           bool coarse, unsigned mode) {
    fist_pool_allocation source = {0};
    fist_pool_allocation target = {0};
    if (install(world, kind, TARGET_TYPE, &source, &target) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[source.slot].vehicle;
    if (mode == DRIVE_INVALID_COMPONENTS || mode == TURRET_INVALID_COMPONENTS) {
        actor->component_size = 0;
    }
    fist_vehicle_motion_events events = {UINT8_MAX, UINT8_MAX, UINT8_MAX};
    const fist_vehicle_motion_events initial = events;
    bool changed = true;
    fist_probe_capture(world, sizeof(*world), before);
    int status = 0;
    switch (mode) {
    case DRIVE_NULL_ACTOR:
        status = fist_vehicle_motion_drive_step(NULL, coarse, &events);
        break;
    case DRIVE_NULL_OUTPUT:
        status = fist_vehicle_motion_drive_step(actor, coarse, NULL);
        break;
    case TURRET_NULL_ACTOR:
        status = fist_vehicle_motion_turret_step(NULL, &changed);
        break;
    case TURRET_NULL_OUTPUT:
        status = fist_vehicle_motion_turret_step(actor, NULL);
        break;
    case DRIVE_INVALID_COMPONENTS:
        status = fist_vehicle_motion_drive_step(actor, coarse, &events);
        break;
    case TURRET_INVALID_COMPONENTS:
        status = fist_vehicle_motion_turret_step(actor, &changed);
        break;
    default:
        return -1;
    }
    return status == -1 && changed && fist_probe_unchanged(&events, sizeof(events), &initial) &&
                   fist_probe_unchanged(world, sizeof(*world), before)
               ? 0
               : -1;
}

typedef struct {
    unsigned repairs;
    unsigned failures;
    unsigned projections;
    unsigned live_cases;
    unsigned stage_failures;
} motion_contract_counts;

static int check_kind(fist_mission_world *world, fist_mission_world *before, uint16_t kind,
                      bool coarse, motion_contract_counts *counts) {
    for (unsigned mode = 0; mode < REPAIRS; ++mode) {
        if (repair(world, before, kind, coarse, mode) != 0) {
            return -1;
        }
        ++counts->repairs;
    }
    for (unsigned mode = 0; mode < FAILURES; ++mode) {
        if (rejection(world, before, kind, coarse, mode) != 0) {
            return -1;
        }
        ++counts->failures;
    }
    if (projection_failure(world, before, kind, coarse) != 0) {
        return -1;
    }
    ++counts->projections;
    for (unsigned mode = 0; mode < LIVE_CASES; ++mode) {
        if (live(world, before, kind, coarse, mode) != 0) {
            return -1;
        }
        ++counts->live_cases;
    }
    for (unsigned mode = 0; mode < STAGE_FAILURES; ++mode) {
        if (stage_rejection(world, before, kind, coarse, mode) != 0) {
            return -1;
        }
        ++counts->stage_failures;
    }
    return 0;
}

int main(void) {
    fist_mission_world *world = calloc(1, sizeof(*world));
    fist_mission_world *before = malloc(sizeof(*before));
    int status = world == NULL || before == NULL ? -1 : 0;
    motion_contract_counts counts = {0};
    for (uint16_t kind = 0; status == 0 && kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
        for (unsigned coarse = 0; status == 0 && coarse < ANGLE_MODES; ++coarse) {
            status = check_kind(world, before, kind, coarse != 0, &counts);
        }
    }
    free(before);
    free(world);
    if (status != 0 || counts.repairs != FIST_UNIT_GROUND_VEHICLE_COUNT * ANGLE_MODES * REPAIRS ||
        counts.failures != FIST_UNIT_GROUND_VEHICLE_COUNT * ANGLE_MODES * FAILURES ||
        counts.projections != FIST_UNIT_GROUND_VEHICLE_COUNT * ANGLE_MODES ||
        counts.live_cases != FIST_UNIT_GROUND_VEHICLE_COUNT * ANGLE_MODES * LIVE_CASES ||
        counts.stage_failures != FIST_UNIT_GROUND_VEHICLE_COUNT * ANGLE_MODES * STAGE_FAILURES) {
        printf("Incomplete motion contracts: %u %u %u %u %u\n", counts.repairs, counts.failures,
               counts.projections, counts.live_cases, counts.stage_failures);
        return EXIT_FAILURE;
    }
    printf("ground_motion_contracts %u %u %u %u %u\n", counts.repairs, counts.failures,
           counts.projections, counts.live_cases, counts.stage_failures);
    return ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
