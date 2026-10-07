#include "assets/bytes.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/destruction_updates.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER = 4,
    WRECK = 23,
    LEADER = FIST_UNIT_EXTENDED_SIZE,
    PRESENCE = LEADER + FIST_UNIT_EXTENDED_SIZE,
    DESCRIPTOR = PRESENCE + 1,
    ROUTE = DESCRIPTOR + (FIST_ORDER_DESCRIPTOR_WORDS * 2),
    RANDOM = ROUTE + FIST_ORDER_PATH_BYTES,
    STREAM = RANDOM + (FIST_RANDOM_STREAMS * 2),
    CASE_BYTES = STREAM + 1,
    PRESENT_VEHICLE = 1,
    PRESENT_WRECK = 2,
    PRESENT_SELF = 3,
    TREE = 21,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    HEADING = 16,
    WAYPOINT_BYTES = 8,
    RETIRING = 19
};

enum {
    TARGET_NONE,
    TARGET_LEADER,
    TARGET_SELF,
    TARGET_RELEASE,
    TARGET_REUSE,
    TARGET_REUSE_OTHER,
    TARGET_ORPHAN,
    TARGET_RETYPE,
    TARGET_BAD_REFERENCE,
    TARGET_BAD_PAYLOAD,
    TARGET_STALE
};

enum { ASSIGN_GOAL, ADVANCE_ROUTE, BEAR_FINE, BEAR_COARSE };

typedef struct {
    fist_vehicle_state actor;
    fist_mission_object leader;
    fist_order_descriptor descriptor;
    fist_order_route route;
    fist_random random;
    uint8_t presence;
    uint8_t target_action;
    uint8_t coarse;
} goal_case;

static fist_unit_definition definition(const uint8_t *raw, size_t size) {
    return (fist_unit_definition){.type = fist_read_u16le(raw),
                                  .map_x = fist_read_i32le(raw + MAP_X),
                                  .map_y = fist_read_i32le(raw + MAP_Y),
                                  .altitude = fist_read_i32le(raw + ALTITUDE),
                                  .heading = fist_read_u16le(raw + HEADING),
                                  .snapshot = {raw, size}};
}

static int decode_case(const uint8_t *raw, goal_case *out, int bearing) {
    const fist_unit_definition actor = definition(raw, FIST_UNIT_EXTENDED_SIZE);
    if (fist_vehicle_restore(&actor, &out->actor) != 0 || raw[PRESENCE] > PRESENT_SELF) {
        return -1;
    }
    out->presence = raw[PRESENCE];
    if (bearing != 0) {
        out->target_action = raw[CASE_BYTES];
        out->coarse = raw[CASE_BYTES + 1];
        if (out->target_action > TARGET_STALE) {
            return -1;
        }
    }
    if (out->presence == PRESENT_VEHICLE ||
        (out->presence != PRESENT_WRECK && out->target_action >= TARGET_RELEASE &&
         out->target_action <= TARGET_RETYPE)) {
        fist_unit_definition leader = definition(raw + LEADER, FIST_UNIT_EXTENDED_SIZE);
        leader.registry_index = 1;
        if (fist_vehicle_restore(&leader, &out->leader.vehicle) != 0) {
            return -1;
        }
    } else if (out->presence == PRESENT_WRECK) {
        fist_unit_definition leader = definition(raw + LEADER, FIST_UNIT_SHORT_SIZE);
        leader.registry_index = 1;
        const fist_pool_allocation allocation = {WRECK, 0, 1, 0};
        if (fist_vehicle_wreck_restore(&leader, allocation, &out->leader.wreck) != 0) {
            return -1;
        }
    }
    for (size_t word = 0; word < FIST_ORDER_DESCRIPTOR_WORDS; ++word) {
        out->descriptor.words[word] = fist_read_u16le(raw + DESCRIPTOR + (word * 2));
    }
    out->route.count = raw[ROUTE];
    for (size_t byte = 0; byte < FIST_ORDER_HEADER_BYTES; ++byte) {
        out->route.header[byte] = raw[ROUTE + 1 + byte];
    }
    for (size_t point = 0; point < FIST_ORDER_WAYPOINTS; ++point) {
        const uint8_t *coordinates =
            raw + ROUTE + 1 + FIST_ORDER_HEADER_BYTES + (point * WAYPOINT_BYTES);
        out->route.points[point] =
            (fist_order_waypoint){fist_read_i32le(coordinates), fist_read_i32le(coordinates + 4)};
    }
    for (size_t word = 0; word < FIST_RANDOM_STREAMS; ++word) {
        out->random.words[word] = fist_read_u16le(raw + RANDOM + (word * 2));
    }
    out->random.next_stream = raw[STREAM];
    if (bearing != 0 && out->random.next_stream < FIST_RANDOM_STREAMS) {
        for (uint8_t link = 0; link <= 2; ++link) {
            fist_random random = out->random;
            fist_vehicle_state retained = {0};
            if (fist_vehicle_initialize(&actor, &random, link, &retained) != 0 ||
                retained.command.retreat_count != out->actor.command.retreat_count ||
                retained.command.maneuver_heading != out->actor.command.maneuver_heading ||
                fist_vehicle_prepare(&retained, link) != 0 ||
                retained.command.retreat_count != out->actor.command.retreat_count ||
                retained.command.maneuver_heading != out->actor.command.maneuver_heading) {
                return -1;
            }
        }
    }
    return 0;
}

static goal_case *decode(const uint8_t *data, size_t size, size_t *count, int bearing) {
    const size_t stride = CASE_BYTES + (bearing != 0 ? 2U : 0U);
    if (size < HEADER || fist_read_u32le(data) != (size - HEADER) / stride ||
        (size - HEADER) % stride != 0) {
        return NULL;
    }
    *count = fist_read_u32le(data);
    goal_case *cases = calloc(*count == 0 ? 1 : *count, sizeof(*cases));
    if (cases == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < *count; ++index) {
        if (decode_case(data + HEADER + (index * stride), &cases[index], bearing) != 0) {
            free(cases);
            return NULL;
        }
    }
    return cases;
}

static int install(fist_mission_world *world, const goal_case *input, uint16_t *slot) {
    fist_mission_world_reset(world);
    fist_pool_allocation actor = {0};
    const fist_pool_import request = {input->actor.type, 0, 0};
    if (fist_object_pool_import(&world->pool, request, &actor) != 0) {
        return -1;
    }
    *slot = actor.slot;
    world->objects[*slot].vehicle = input->actor;
    world->orders_loaded = 1;
    world->random = input->random;
    uint16_t leader_slot = FIST_POOL_NO_SLOT;
    if (input->presence == PRESENT_SELF) {
        leader_slot = *slot;
    } else if (input->presence != 0) {
        fist_pool_allocation leader = {0};
        const uint16_t type = input->presence == PRESENT_WRECK ? WRECK : input->leader.vehicle.type;
        if (fist_object_pool_import(&world->pool, (fist_pool_import){type, 1, 0}, &leader) != 0) {
            return -1;
        }
        leader_slot = leader.slot;
        world->objects[leader_slot] = input->leader;
    }
    if (input->actor.platoon < FIST_UNIT_PLATOON_COUNT) {
        const size_t platoon = input->actor.platoon;
        world->combat.roster[platoon * FIST_UNIT_MEMBERS_PER_PLATOON] = leader_slot;
        world->orders.descriptors[platoon] = input->descriptor;
        world->orders.routes[platoon] = input->route;
    }
    return 0;
}

static int apply_command(fist_mission_world *world, uint16_t slot, int routes) {
    if (routes >= 2) {
        return fist_mission_world_bear_command(world, slot, routes == 3);
    }
    return routes != 0 ? fist_mission_world_advance_command_route(world, slot)
                       : fist_mission_world_assign_command_goal(world, slot);
}

static int mutate_bearing_target(fist_mission_world *world, const goal_case *input,
                                 fist_pool_allocation target) {
    const uint8_t action = input->target_action;
    const uint16_t type = target.type;
    if (action == TARGET_RETYPE) {
        if (type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
            fist_object_pool_retype(&world->pool, target, RETIRING, &target) != 0) {
            return -1;
        }
        world->objects[target.slot].vehicle.type = RETIRING;
        return 0;
    }
    if (action == TARGET_ORPHAN) {
        fist_pool_allocation successor = {0};
        if (fist_object_pool_import(&world->pool, (fist_pool_import){type, 2, 0}, &successor) !=
            0) {
            return -1;
        }
        world->objects[successor.slot] = input->leader;
        if (type == WRECK) {
            world->objects[successor.slot].wreck.allocation = successor;
        } else {
            world->objects[successor.slot].vehicle.map_x = INT32_MAX;
        }
        return 0;
    }
    if (fist_object_pool_release(&world->pool, 2, &target) != 0) {
        return -1;
    }
    if (action == TARGET_RELEASE) {
        return 0;
    }
    fist_pool_allocation successor = {0};
    const uint16_t successor_type = action == TARGET_REUSE_OTHER ? 2 : type;
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){successor_type, 0},
                                  &successor) != 0 ||
        successor.slot != target.slot) {
        return -1;
    }
    world->objects[successor.slot] = input->leader;
    if (successor_type == WRECK) {
        world->objects[successor.slot].wreck.allocation = successor;
        world->objects[successor.slot].wreck.pose.x = INT32_MIN;
    } else {
        world->objects[successor.slot].vehicle.type = successor_type;
        world->objects[successor.slot].vehicle.map_x = INT32_MIN;
        world->objects[successor.slot].vehicle.component_size =
            fist_vehicle_component_size(successor_type);
    }
    return 0;
}

static int bearing_target(fist_mission_world *world, const goal_case *input, uint16_t slot) {
    const uint8_t action = input->target_action;
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (action == TARGET_NONE) {
        return 0;
    }
    if (action == TARGET_BAD_REFERENCE) {
        actor->command.target = (fist_object_reference){0, FIST_POOL_EXTENDED_SLOTS};
        return 0;
    }
    if (action == TARGET_STALE) {
        actor->command.target = (fist_object_reference){UINT64_MAX, slot};
        return 0;
    }
    if (action == TARGET_SELF) {
        return fist_object_pool_reference(&world->pool, slot, &actor->command.target);
    }
    if (action == TARGET_LEADER || action == TARGET_BAD_PAYLOAD) {
        const uint16_t leader = world->combat.roster[(size_t)actor->platoon * 4];
        if (leader == FIST_POOL_NO_SLOT) {
            return 0;
        }
        if (fist_object_pool_reference(&world->pool, leader, &actor->command.target) != 0) {
            return -1;
        }
        if (action == TARGET_BAD_PAYLOAD) {
            world->objects[leader].vehicle.type = UINT16_MAX;
        }
        return 0;
    }
    fist_pool_allocation target = {0};
    const uint16_t type = input->presence == PRESENT_WRECK ? WRECK : input->leader.vehicle.type;
    if (fist_object_pool_import(&world->pool, (fist_pool_import){type, 2, 0}, &target) != 0) {
        return -1;
    }
    world->objects[target.slot] = input->leader;
    if (type == WRECK) {
        world->objects[target.slot].wreck.allocation = target;
    }
    if (fist_object_pool_reference(&world->pool, target.slot, &actor->command.target) != 0) {
        return -1;
    }
    return mutate_bearing_target(world, input, target);
}

static int run_case(fist_mission_world *world, fist_mission_world *before, const goal_case *input,
                    int routes) {
    uint16_t slot = 0;
    if (install(world, input, &slot) != 0) {
        return -1;
    }
    if (routes >= 2) {
        if (input->actor.platoon < FIST_UNIT_PLATOON_COUNT &&
            bearing_target(world, input, slot) != 0) {
            return -1;
        }
        world->preparation.prepared = 1;
        routes = input->coarse != 0 ? 3 : 2;
    }
    fist_probe_capture(world, sizeof(*world), before);
    if (apply_command(NULL, slot, routes) != -1 ||
        apply_command(world, FIST_POOL_NO_SLOT, routes) != -1 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    const int status = apply_command(world, slot, routes);
    if (status == 0) {
        if (routes == 1) {
            const size_t platoon = world->objects[slot].vehicle.platoon;
            before->orders.routes[platoon] = world->orders.routes[platoon];
        } else {
            before->objects[slot].vehicle.command.goal = world->objects[slot].vehicle.command.goal;
        }
        before->objects[slot].vehicle.control_flags = world->objects[slot].vehicle.control_flags;
        if (routes >= 2) {
            fist_vehicle_state *expected = &before->objects[slot].vehicle;
            const fist_vehicle_state *actual = &world->objects[slot].vehicle;
            expected->drive.requested_heading = actual->drive.requested_heading;
            expected->command.range = actual->command.range;
            expected->command.retreat_count = actual->command.retreat_count;
            expected->command.mode = actual->command.mode;
            expected->command.target = actual->command.target;
        }
    }
    if (!fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    printf("status %d\n", status);
    fist_probe_write_vehicle_state(&world->objects[slot].vehicle);
    if (routes == 1) {
        const size_t platoon = world->objects[slot].vehicle.platoon;
        if (platoon < FIST_UNIT_PLATOON_COUNT) {
            fist_probe_write_route(platoon, &world->orders.routes[platoon]);
        } else {
            puts("route unavailable");
        }
    }
    if (routes >= 2) {
        const fist_vehicle_state *actor = &world->objects[slot].vehicle;
        printf("bearing %u %u %d\n", (unsigned)actor->command.retreat_count,
               (unsigned)actor->command.maneuver_heading,
               fist_object_pool_reference_is_live(&world->pool, actor->command.target));
    }
    /* Full orders and leader payload preservation is also checked above. */
    printf("random %u", (unsigned)world->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)world->random.words[index]);
    }
    puts("");
    return 0;
}

static int rejected(fist_mission_world *world, fist_mission_world *before, uint16_t slot,
                    int routes) {
    fist_probe_capture(world, sizeof(*world), before);
    return apply_command(world, slot, routes) == -1 &&
                   fist_probe_unchanged(world, sizeof(*world), before)
               ? 0
               : -1;
}

static int invalid_roster(fist_mission_world *world, fist_mission_world *before, uint16_t slot) {
    world->combat.roster[0] = FIST_UNIT_REGISTRY_COUNT;
    if (rejected(world, before, slot, 0) != 0) {
        return -1;
    }
    world->combat.roster[0] = 0;
    if (rejected(world, before, slot, 0) != 0) {
        return -1;
    }
    fist_pool_allocation leader = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){TREE, 1, 0}, &leader) != 0) {
        return -1;
    }
    world->combat.roster[0] = leader.slot;
    if (rejected(world, before, slot, 0) != 0 ||
        fist_object_pool_import(&world->pool, (fist_pool_import){1, 2, 0}, &leader) != 0) {
        return -1;
    }
    world->combat.roster[0] = leader.slot;
    if (rejected(world, before, slot, 0) != 0) {
        return -1;
    }
    world->objects[leader.slot].vehicle.component_size = fist_vehicle_component_size(0);
    return rejected(world, before, slot, 0);
}

static int invalid_worlds(fist_mission_world *world, fist_mission_world *before, int routes) {
    fist_mission_world_reset(world);
    fist_pool_allocation allocation = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){0, 0, 0}, &allocation) != 0) {
        return -1;
    }
    const uint16_t slot = allocation.slot;
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    actor->component_size = fist_vehicle_component_size(0);
    actor->command.mode = 2;
    actor->control_flags = 1;
    actor->member = 1;
    for (unsigned loaded = 0; loaded <= UINT8_MAX; ++loaded) {
        if (loaded == 1) {
            continue;
        }
        world->orders_loaded = (uint8_t)loaded;
        if (rejected(world, before, slot, routes) != 0) {
            return -1;
        }
    }
    world->orders_loaded = 1;
    for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
        if (size == fist_vehicle_component_size(0)) {
            continue;
        }
        actor->component_size = size;
        if (rejected(world, before, slot, routes) != 0) {
            return -1;
        }
    }
    actor->type = 1;
    actor->component_size = fist_vehicle_component_size(1);
    if (rejected(world, before, slot, routes) != 0) {
        return -1;
    }
    actor->type = 0;
    actor->component_size = fist_vehicle_component_size(0);
    world->pool.registry[0].slot = FIST_UNIT_REGISTRY_COUNT;
    if (rejected(world, before, slot, routes) != 0) {
        return -1;
    }
    world->pool.registry[0].slot = slot;
    if (routes == 0) {
        return invalid_roster(world, before, slot);
    }
    world->pool.slots[slot].used = 0;
    return rejected(world, before, slot, routes);
}

int main(int argc, char **argv) {
    if (argc != 2 &&
        (argc != 3 || (strcmp(argv[2], "--routes") != 0 && strcmp(argv[2], "--bearing") != 0))) {
        return EXIT_FAILURE;
    }
    int mode = ASSIGN_GOAL;
    if (argc == 3) {
        mode = strcmp(argv[2], "--routes") == 0 ? ADVANCE_ROUTE : BEAR_FINE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    size_t count = 0;
    goal_case *cases = data == NULL || closed != 0 ? NULL : decode(data, size, &count, mode == 2);
    if (data != NULL) {
        for (size_t index = 0; index < size; ++index) {
            data[index] = UINT8_MAX;
        }
    }
    free(data);
    fist_mission_world *world = calloc(1, sizeof(*world));
    fist_mission_world *before = malloc(sizeof(*before));
    int status = cases == NULL || world == NULL || before == NULL ? -1 : 0;
    if (status == 0) {
        status = invalid_worlds(world, before, mode);
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        status = run_case(world, before, &cases[index], mode);
    }
    free(before);
    free(world);
    free(cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
