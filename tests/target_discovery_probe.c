#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    BATCH_HEADER = 8,
    HEADER = 25,
    ACQUISITION_HEADER = 7,
    REQUESTED_CANDIDATE_OFFSET = 5,
    RETAINED_NOTICE = 77,
    VOICE_INTERVAL = 30,
    ACQUIRE = 1,
    AUTOMATIC = 2,
    AIM = 4,
    THROTTLE = 8,
    DISCOVER = 16,
    PROMOTION = 32,
    MANEUVER = 64,
    IDLE_TURRET = 65,
    MOTION_OBSTACLE = 66,
    ACQUISITION_OPERATIONS = 31,
    ROSTER_BYTES = FIST_UNIT_ROSTER_COUNT * 2,
    MAX_PROMOTIONS = 4,
    PLATOON = 27,
    MEMBER = 28,
    WRECK_PLATOON = 35,
    WRECK_MEMBER = 36,
    CONTROL_FLAGS = 64,
    PROMOTION_BLOCKED_FLAG = 16,
    PROMOTION_GOAL_VALID = 2,
    ACTOR = 0,
    SELECTED = 2,
    TICK = 4,
    GATE = 6,
    LAST = 8,
    OLD_TARGET = 10,
    LINK = 12,
    COARSE = 13,
    CURSOR = 14,
    SEEDS = 15,
    OBJECTS = 23,
    REGISTRY_BYTES = FIST_UNIT_REGISTRY_COUNT * 4,
    FLAGS = 22,
    SECONDARY = 23,
    MODE = 25,
    SCALE = 20,
    MAX_DETAIL = 4096,
    RETIRING = 19,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    TARGET = 26,
    ARTILLERY = 27,
    EXPLOSION = 4,
    SHELL = 8,
    SMOKE = 17,
    MUZZLE = 18,
    TREE = 21,
    WRECK = 23,
    FIXTURE_ALTITUDE = 4096,
    FIXTURE_DISTANCE = 8192,
    OPPOSING_FLAGS = 12,
    ATTEMPT_FLAG = 128,
    TARGET_COMMAND = 6
};

typedef struct {
    const uint8_t *header;
    const uint8_t *registry;
    const uint8_t *roster;
    uint8_t operation;
    uint16_t behavior;
    uint16_t candidate;
    fist_object_reference runtime_candidate;
    uint16_t requested_candidate;
    fist_object_reference runtime_request;
    uint8_t *raw[FIST_UNIT_REGISTRY_COUNT];
} query_case;

static void poison(uint8_t *data, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        data[index] = UINT8_MAX;
    }
}

static int validate_case(const query_case *value) {
    uint8_t owners[FIST_UNIT_REGISTRY_COUNT] = {0};
    for (size_t entry = 0; entry < FIST_UNIT_REGISTRY_COUNT; ++entry) {
        const uint16_t slot = fist_read_u16le(value->registry + (entry * 4));
        if (slot != FIST_POOL_NO_SLOT) {
            if (slot >= FIST_UNIT_REGISTRY_COUNT || value->raw[slot] == NULL || owners[slot] != 0) {
                return -1;
            }
            owners[slot] = 1;
        }
    }
    const uint16_t actor = fist_read_u16le(value->header + ACTOR);
    const uint16_t target = fist_read_u16le(value->header + OLD_TARGET);
    if ((value->candidate != FIST_POOL_NO_SLOT && value->candidate >= FIST_UNIT_REGISTRY_COUNT) ||
        (value->requested_candidate != FIST_POOL_NO_SLOT &&
         value->requested_candidate >= FIST_UNIT_REGISTRY_COUNT) ||
        actor >= FIST_UNIT_REGISTRY_COUNT || value->raw[actor] == NULL ||
        fist_read_u16le(value->raw[actor]) >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        (target != FIST_POOL_NO_SLOT && target >= FIST_UNIT_REGISTRY_COUNT)) {
        return -1;
    }
    return 0;
}

static int decode_case(uint8_t *data, size_t size, size_t *cursor, query_case *value,
                       bool acquisition) {
    size_t position = *cursor;
    if (position > size ||
        size - position < HEADER + REGISTRY_BYTES + (acquisition ? ACQUISITION_HEADER : 0)) {
        return -1;
    }
    value->header = data + position;
    const uint16_t records = fist_read_u16le(value->header + OBJECTS);
    if (records > FIST_UNIT_REGISTRY_COUNT || value->header[CURSOR] >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    position += HEADER;
    value->candidate = FIST_POOL_NO_SLOT;
    value->requested_candidate = FIST_POOL_NO_SLOT;
    if (acquisition) {
        value->operation = data[position];
        value->behavior = fist_read_u16le(data + position + 1);
        value->candidate = fist_read_u16le(data + position + 3);
        value->requested_candidate = fist_read_u16le(data + position + REQUESTED_CANDIDATE_OFFSET);
        if (value->operation > ACQUISITION_OPERATIONS && value->operation != PROMOTION &&
            (value->operation < MANEUVER || value->operation > MOTION_OBSTACLE)) {
            return -1;
        }
        position += ACQUISITION_HEADER;
        if (value->operation == PROMOTION) {
            if (size - position < ROSTER_BYTES + REGISTRY_BYTES || value->behavior == 0 ||
                value->behavior > MAX_PROMOTIONS) {
                return -1;
            }
            value->roster = data + position;
            position += ROSTER_BYTES;
        }
    }
    value->registry = data + position;
    position += REGISTRY_BYTES;
    for (size_t entry = 0; entry < records; ++entry) {
        if (position > size || size - position < 2 * sizeof(uint16_t)) {
            return -1;
        }
        const uint16_t slot = fist_read_u16le(data + position);
        position += sizeof(uint16_t);
        const uint16_t type = fist_read_u16le(data + position);
        const size_t length = fist_unit_state_size(type);
        if (slot >= FIST_UNIT_REGISTRY_COUNT || value->raw[slot] != NULL || length == 0 ||
            length > size - position ||
            (length == FIST_UNIT_EXTENDED_SIZE) != (slot >= FIST_POOL_SHORT_SLOTS)) {
            return -1;
        }
        value->raw[slot] = data + position;
        position += length;
    }

    *cursor = position;
    return validate_case(value);
}

static int decode(uint8_t *data, size_t size, fist_klc_image *height, query_case **cases,
                  uint32_t *count, bool acquisition) {
    if (size < BATCH_HEADER) {
        return -1;
    }
    const uint32_t side = fist_read_u32le(data);
    *count = fist_read_u32le(data + sizeof(uint32_t));
    if (side == 0 || side > MAX_DETAIL || (side & (side - 1)) != 0 ||
        (size_t)side * side > size - BATCH_HEADER || *count > size / HEADER) {
        return -1;
    }
    const size_t pixels = (size_t)side * side;
    *height = (fist_klc_image){.width = side, .height = side, .pixels = data + BATCH_HEADER};
    query_case *values = calloc(*count == 0 ? 1 : *count, sizeof(*values));
    if (values == NULL) {
        return -1;
    }
    size_t position = BATCH_HEADER + pixels;
    for (size_t index = 0; index < *count; ++index) {
        if (decode_case(data, size, &position, &values[index], acquisition) != 0) {
            free(values);
            return -1;
        }
    }
    if (position != size) {
        free(values);
        return -1;
    }
    *cases = values;
    return 0;
}

static int restore_view(fist_mission_world *world, uint16_t slot, const uint8_t *raw) {
    const uint16_t type = fist_read_u16le(raw);
    fist_pool_allocation allocation = {type, slot, 0, 0};
    const int found = fist_object_pool_find(&world->pool, slot, &allocation);
    if (found < 0) {
        return -1;
    }
    const fist_object_pose pose = {fist_read_i32le(raw + 4), fist_read_i32le(raw + 8),
                                   fist_read_i32le(raw + 12), fist_read_u16le(raw + 16)};
    fist_mission_object *object = &world->objects[slot];
    const uint16_t scale = fist_read_u16le(raw + SCALE);
    if (type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        const fist_unit_definition definition = {.type = type,
                                                 .registry_index = allocation.registry_index,
                                                 .generation = allocation.value,
                                                 .map_x = pose.x,
                                                 .map_y = pose.y,
                                                 .altitude = pose.altitude,
                                                 .heading = pose.heading,
                                                 .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
        if (fist_vehicle_restore(&definition, &object->vehicle) != 0) {
            return -1;
        }
        /* Raw near pointers are opaque input transport, never runtime IDs. */
        object->vehicle.command.target_reference = 0;
        object->vehicle.command.candidate_reference = 0;
        return 0;
    }
    switch (type) {
    case RETIRING:
        object->vehicle =
            (fist_vehicle_state){.type = type,
                                 .platoon = raw[PLATOON],
                                 .member = raw[MEMBER],
                                 .control_flags = fist_read_u16le(raw + CONTROL_FLAGS),
                                 .map_x = pose.x,
                                 .map_y = pose.y,
                                 .altitude = pose.altitude,
                                 .turret = {.heading = pose.heading},
                                 .drive = {.motion_flags = raw[MODE]},
                                 .object_flags = raw[FLAGS],
                                 .secondary_flags = raw[SECONDARY],
                                 .projection_scale = scale};
        break;
    case EXPLOSION:
        object->explosion = (fist_explosion){.allocation = allocation,
                                             .pose = pose,
                                             .projection_scale = scale,
                                             .flags = raw[FLAGS],
                                             .secondary_flags = raw[SECONDARY],
                                             .frame = raw[MODE]};
        break;
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT:
    case TARGET:
    case ARTILLERY:
        object->other = (fist_other_actor){.allocation = allocation,
                                           .pose = pose,
                                           .projection_scale = scale,
                                           .flags = raw[FLAGS],
                                           .secondary_flags = raw[SECONDARY],
                                           .mode = raw[MODE]};
        break;
    case SHELL:
        object->projectile = (fist_projectile){.allocation = allocation,
                                               .projection_scale = scale,
                                               .pose = pose,
                                               .flags = raw[FLAGS],
                                               .secondary_flags = raw[SECONDARY],
                                               .mode = raw[MODE]};
        break;
    case SMOKE:
        object->smoke = (fist_drifting_smoke){.allocation = allocation,
                                              .pose = pose,
                                              .projection_scale = scale,
                                              .flags = raw[FLAGS],
                                              .secondary_flags = raw[SECONDARY],
                                              .animation_frame = raw[MODE]};
        break;
    case MUZZLE:
        object->muzzle = (fist_muzzle_smoke){.allocation = allocation,
                                             .pose = pose,
                                             .projection_scale = scale,
                                             .flags = raw[FLAGS],
                                             .secondary_flags = raw[SECONDARY],
                                             .animation_frame = raw[MODE]};
        break;
    case TREE:
        object->tree = (fist_tree){.allocation = allocation,
                                   .pose = pose,
                                   .projection_scale = scale,
                                   .flags = raw[FLAGS],
                                   .secondary_flags = raw[SECONDARY],
                                   .variant = raw[MODE]};
        break;
    case WRECK:
        object->wreck = (fist_vehicle_wreck){.allocation = allocation,
                                             .platoon = raw[WRECK_PLATOON],
                                             .member = raw[WRECK_MEMBER],
                                             .pose = pose,
                                             .projection_scale = scale,
                                             .flags = raw[FLAGS],
                                             .secondary_flags = raw[SECONDARY]};
        break;
    default:
        object->saved_base = (fist_saved_object_base){.allocation = allocation,
                                                      .pose = pose,
                                                      .projection_scale = scale,
                                                      .flags = raw[FLAGS],
                                                      .secondary_flags = raw[SECONDARY],
                                                      .variant = raw[MODE]};
        break;
    }
    return 0;
}

static int allocate_arena(fist_mission_world *world, const query_case *value, size_t begin,
                          size_t end, uint16_t fallback) {
    for (size_t slot = begin; slot < end; ++slot) {
        const uint16_t type =
            value->raw[slot] == NULL ? fallback : fist_read_u16le(value->raw[slot]);
        fist_pool_allocation allocation = {0};
        if (fist_object_pool_import(&world->pool, (fist_pool_import){type, (uint16_t)slot, 1},
                                    &allocation) != 0 ||
            allocation.slot != slot) {
            return -1;
        }
    }
    return 0;
}

static void include_slot(size_t ends[2], uint16_t slot) {
    if (slot != FIST_POOL_NO_SLOT && ends[slot >= FIST_POOL_SHORT_SLOTS] <= slot) {
        ends[slot >= FIST_POOL_SHORT_SLOTS] = (size_t)slot + 1;
    }
}

static int install_pool(fist_mission_world *world, query_case *value,
                        fist_object_reference *old_target) {
    fist_mission_world_reset(world);
    const uint16_t target = fist_read_u16le(value->header + OLD_TARGET);
    /* Build physical arenas through the allocator, then restore the declared
     * original current bindings/orphans. Temporary holes receive real releases. */
    size_t ends[2] = {0, FIST_POOL_SHORT_SLOTS};
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (value->raw[slot] != NULL) {
            ends[slot >= FIST_POOL_SHORT_SLOTS] = slot + 1;
        }
    }
    const uint16_t candidate = value->candidate;
    const uint16_t requested = value->requested_candidate;
    include_slot(ends, target);
    include_slot(ends, candidate);
    include_slot(ends, requested);
    if (allocate_arena(world, value, 0, ends[0], SHELL) != 0 ||
        allocate_arena(world, value, FIST_POOL_SHORT_SLOTS, ends[1], 0) != 0) {
        return -1;
    }
    if (target != FIST_POOL_NO_SLOT &&
        fist_object_pool_reference(&world->pool, target, old_target) != 0) {
        return -1;
    }
    if (candidate != FIST_POOL_NO_SLOT &&
        fist_object_pool_reference(&world->pool, candidate, &value->runtime_candidate) != 0) {
        return -1;
    }
    if (requested != FIST_POOL_NO_SLOT &&
        fist_object_pool_reference(&world->pool, requested, &value->runtime_request) != 0) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (world->pool.slots[slot].used != 0 && value->raw[slot] == NULL) {
            fist_pool_allocation removed = {0};
            if (fist_object_pool_release(&world->pool, (uint16_t)slot, &removed) != 0) {
                return -1;
            }
        }
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const uint8_t *entry = value->registry + (index * 4);
        world->pool.registry[index] =
            (fist_pool_entry){fist_read_u16le(entry), fist_read_u16le(entry + sizeof(uint16_t))};
    }
    return fist_object_pool_is_valid(&world->pool) ? 0 : -1;
}

static int install(fist_mission_world *world, query_case *value) {
    fist_object_reference old_target = {0};
    if (install_pool(world, value, &old_target) != 0) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (value->raw[slot] != NULL &&
            restore_view(world, (uint16_t)slot, value->raw[slot]) != 0) {
            return -1;
        }
    }
    world->preparation.prepared = 1;
    world->combat.selected_slot = fist_read_u16le(value->header + SELECTED);
    world->voice.admitted_at = fist_read_u16le(value->header + LAST);
    world->random.next_stream = value->header[CURSOR];
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        world->random.words[index] = fist_read_u16le(value->header + SEEDS + (index * 2));
    }
    const uint16_t actor = fist_read_u16le(value->header + ACTOR);
    if (actor >= FIST_UNIT_REGISTRY_COUNT || world->pool.slots[actor].used == 0) {
        return -1;
    }
    world->objects[actor].vehicle.command.target = old_target;
    world->objects[actor].vehicle.command.candidate = value->runtime_candidate;
    return 0;
}

static unsigned reference_slot(fist_object_reference reference) {
    return reference.lifetime == 0 ? FIST_POOL_NO_SLOT : reference.slot;
}

static int canonical_boundary(const fist_mission_world *world, const query_case *value) {
    enum {
        POSITION_Y = 8,
        ALTITUDE = 12,
        POSE_HEADING = 16,
        HEADING = 142,
        COUNT = 148,
        LAST_SEED = 6
    };
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const fist_pool_entry entry = world->pool.registry[index];
        const uint8_t *binding = value->registry + (index * 4);
        if (entry.slot != fist_read_u16le(binding) ||
            entry.value != fist_read_u16le(binding + sizeof(uint16_t)) ||
            (world->pool.slots[index].used != 0) != (value->raw[index] != NULL)) {
            return -1;
        }
        const uint8_t *raw = value->raw[index];
        if (raw == NULL) {
            continue;
        }
        fist_object_pose ground = {0};
        fist_mission_view view = {0};
        if (world->pool.slots[index].type != fist_read_u16le(raw) ||
            fist_mission_world_view(world, (uint16_t)index, &ground, &view) != 0 ||
            view.pose->x != fist_read_i32le(raw + 4) ||
            view.pose->y != fist_read_i32le(raw + POSITION_Y) ||
            view.pose->altitude != fist_read_i32le(raw + ALTITUDE) ||
            view.pose->heading != fist_read_u16le(raw + POSE_HEADING) || view.flags != raw[FLAGS] ||
            view.secondary_flags != raw[SECONDARY] ||
            view.projection_scale != fist_read_u16le(raw + SCALE) ||
            (world->pool.slots[index].type != WRECK && view.mode != raw[MODE])) {
            return -1;
        }
    }
    const uint16_t slot = fist_read_u16le(value->header + ACTOR);
    const fist_vehicle_command *command = &world->objects[slot].vehicle.command;
    return command->secondary_heading == fist_read_u16le(value->raw[slot] + HEADING) &&
                   command->discovery_count == value->raw[slot][COUNT] &&
                   world->random.next_stream == value->header[CURSOR] &&
                   world->random.words[0] == fist_read_u16le(value->header + SEEDS) &&
                   world->random.words[1] == fist_read_u16le(value->header + SEEDS + 2) &&
                   world->random.words[2] == fist_read_u16le(value->header + SEEDS + 4) &&
                   world->random.words[3] == fist_read_u16le(value->header + SEEDS + LAST_SEED)
               ? 0
               : -1;
}

static int query(fist_mission_world *world, fist_mission_world *before,
                 const fist_klc_image *height, query_case *value, bool canonical) {
    if (!canonical && install(world, value) != 0) {
        return -1;
    }
    if (canonical) {
        if (canonical_boundary(world, value) != 0) {
            return -1;
        }
        world->combat.selected_slot = fist_read_u16le(value->header + SELECTED);
        world->voice.admitted_at = fist_read_u16le(value->header + LAST);
    }
    fist_target_discovery_result result = {0};
    const fist_target_discovery_request request = {.slot = fist_read_u16le(value->header + ACTOR),
                                                   .tick = fist_read_u16le(value->header + TICK),
                                                   .voice_gate =
                                                       fist_read_u16le(value->header + GATE),
                                                   .link_mode = value->header[LINK],
                                                   .coarse = value->header[COARSE] != 0};
    /* Payload inputs no longer survive in any simulation state. */
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (value->raw[slot] != NULL) {
            poison(value->raw[slot], fist_unit_state_size(world->pool.slots[slot].type));
        }
    }
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_discover_targets(NULL, height, request, &result) != -1 ||
        fist_mission_world_discover_targets(world, NULL, request, &result) != -1 ||
        fist_mission_world_discover_targets(world, height, request, NULL) != -1 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    if (fist_mission_world_discover_targets(world, height, request, &result) != 0) {
        return -1;
    }
    const fist_vehicle_state *actor = &world->objects[request.slot].vehicle;
    fist_vehicle_state *allowed = &before->objects[request.slot].vehicle;
    allowed->drive.motion_flags = actor->drive.motion_flags;
    allowed->command.secondary_heading = actor->command.secondary_heading;
    allowed->command.discovery_count = actor->command.discovery_count;
    allowed->command.target = actor->command.target;
    allowed->command.candidate = actor->command.candidate;
    before->voice = world->voice;
    if (!fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    printf("target %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %lu %u\n", (unsigned)request.slot,
           reference_slot(result.primary), reference_slot(result.secondary),
           (unsigned)result.primary_range, (unsigned)result.secondary_operand,
           (unsigned)result.priority, (unsigned)result.count, (unsigned)actor->drive.motion_flags,
           (unsigned)actor->command.secondary_heading, reference_slot(actor->command.target),
           reference_slot(actor->command.candidate), (unsigned)actor->command.discovery_count,
           (unsigned)result.voice.emitted, (unsigned)result.voice.ax, (unsigned)result.voice.dx,
           (unsigned long)result.voice.ecx, (unsigned)world->voice.admitted_at);
    return 0;
}

static int lifetime_case(fist_mission_world *world, fist_mission_world *before,
                         uint16_t replacement) {
    const uint8_t pixel = 0;
    const fist_klc_image height = {.width = 1, .height = 1, .pixels = (uint8_t *)&pixel};
    fist_mission_world_reset(world);
    fist_pool_allocation actor_binding = {0};
    fist_pool_allocation target_binding = {0};
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){0, 0}, &actor_binding) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){0, 0}, &target_binding) != 0) {
        return -1;
    }
    const uint16_t slot = actor_binding.slot;
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    actor->type = 0;
    actor->altitude = FIXTURE_ALTITUDE;
    actor->component_size = fist_vehicle_component_size(0);
    world->objects[target_binding.slot].vehicle =
        (fist_vehicle_state){.type = 0,
                             .map_x = -FIXTURE_ALTITUDE,
                             .altitude = FIXTURE_ALTITUDE,
                             .object_flags = OPPOSING_FLAGS};
    world->preparation.prepared = 1;
    fist_object_reference old = {0};
    fist_pool_allocation removed = {0};
    if (fist_object_pool_reference(&world->pool, target_binding.slot, &old) != 0 ||
        fist_object_pool_release(&world->pool, target_binding.registry_index, &removed) != 0 ||
        fist_object_pool_reference_is_live(&world->pool, old)) {
        return -1;
    }
    actor->command.target = old;
    const fist_target_discovery_request request = {.slot = slot};
    fist_target_discovery_result result = {0};
    if (fist_mission_world_discover_targets(world, &height, request, &result) != 0 ||
        actor->command.target.lifetime != 0 || result.count != 0) {
        return -1;
    }
    fist_pool_allocation fresh = {0};
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){replacement, 0}, &fresh) != 0 ||
        fresh.slot != old.slot || fresh.registry_index != target_binding.registry_index ||
        fresh.value != target_binding.value ||
        fist_object_pool_reference_is_live(&world->pool, old)) {
        return -1;
    }
    world->objects[fresh.slot].vehicle = (fist_vehicle_state){.type = replacement,
                                                              .map_x = -FIXTURE_DISTANCE,
                                                              .altitude = FIXTURE_ALTITUDE,
                                                              .object_flags = OPPOSING_FLAGS};
    actor->command.target = old;
    if (fist_mission_world_discover_targets(world, &height, request, &result) != 0 ||
        actor->command.target.lifetime != 0 || result.count != 1 ||
        result.primary.slot != fresh.slot || result.primary.lifetime == old.lifetime) {
        return -1;
    }
    /* The mode selector consumes runtime presence, never a saved near word. */
    world->orders_loaded = 1;
    world->random.words[0] = 2;
    world->orders.descriptors[0].words[1] = 2;
    actor->control_flags = 1;
    actor->command.target_reference = UINT16_MAX;
    actor->command.target = old;
    const fist_random random = world->random;
    if (fist_mission_world_select_command(world, (fist_command_selection){slot, 0}) != 0 ||
        actor->command.target.lifetime != 0 || actor->command.mode != 0 ||
        !fist_probe_unchanged(&world->random, sizeof(random), &random)) {
        return -1;
    }
    actor->command.target = result.primary;
    if (fist_mission_world_select_command(world, (fist_command_selection){slot, 0}) != 0 ||
        actor->command.mode != TARGET_COMMAND || world->random.next_stream == random.next_stream) {
        return -1;
    }
    /* Retype and orphaning preserve physical identity; reset invalidates it. */
    fist_object_reference retained = {0};
    if (fist_object_pool_reference(&world->pool, fresh.slot, &retained) != 0 ||
        fist_object_pool_retype(&world->pool, fresh, RETIRING, &fresh) != 0 ||
        !fist_object_pool_reference_is_live(&world->pool, retained)) {
        return -1;
    }
    world->objects[fresh.slot].vehicle.type = RETIRING;
    fist_pool_allocation other = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){0, fresh.registry_index, 1},
                                &other) != 0 ||
        fist_object_pool_find(&world->pool, fresh.slot, &removed) != FIST_POOL_UNAVAILABLE ||
        !fist_object_pool_reference_is_live(&world->pool, retained)) {
        return -1;
    }
    fist_probe_capture(world, sizeof(*world), before);
    fist_mission_world_reset(world);
    if (fist_object_pool_reference_is_live(&world->pool, retained) ||
        !fist_object_pool_reference_is_live(&before->pool, retained) ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){0, 0}, &other) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){0, 0}, &other) != 0 ||
        fist_object_pool_reference_is_live(&world->pool, old)) {
        return -1;
    }
    return 0;
}

static int lifetime_contracts(fist_mission_world *world, fist_mission_world *before) {
    for (uint16_t replacement = 0; replacement <= 2; replacement += 2) {
        if (lifetime_case(world, before, replacement) != 0) {
            return -1;
        }
    }
    return 0;
}

static int retained_aim_fields(const fist_vehicle_state *actor) {
    enum { TARGET_RANGE = 153, TARGET_HEADING = 155, ELEVATION = 56, BYTE_SHIFT = 8 };
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE] = {0};
    raw[0] = (uint8_t)actor->type;
    const uint16_t elevation = (uint16_t)actor->turret.elevation;
    raw[ELEVATION] = (uint8_t)elevation;
    raw[ELEVATION + 1] = (uint8_t)(elevation >> BYTE_SHIFT);
    raw[TARGET_RANGE] = (uint8_t)actor->command.target_range;
    raw[TARGET_RANGE + 1] = (uint8_t)(actor->command.target_range >> BYTE_SHIFT);
    raw[TARGET_HEADING] = (uint8_t)actor->command.target_heading;
    raw[TARGET_HEADING + 1] = (uint8_t)(actor->command.target_heading >> BYTE_SHIFT);
    const fist_unit_definition definition = {.type = actor->type, .snapshot = {raw, sizeof(raw)}};
    for (uint8_t link = 0; link <= 2; ++link) {
        fist_random random = {.words = {1, 2, 3, 4}};
        fist_vehicle_state restored = {0};
        if (fist_vehicle_initialize(&definition, &random, link, &restored) != 0 ||
            restored.command.target_range != actor->command.target_range ||
            restored.command.target_heading != actor->command.target_heading ||
            restored.turret.elevation != actor->turret.elevation ||
            fist_vehicle_prepare(&restored, link) != 0 ||
            restored.command.target_range != actor->command.target_range ||
            restored.command.target_heading != actor->command.target_heading ||
            restored.turret.elevation != actor->turret.elevation) {
            return -1;
        }
    }
    return 0;
}

static int prepare_acquisition(fist_mission_world *world, const fist_klc_image *height,
                               query_case *value, bool canonical) {
    const uint16_t slot = fist_read_u16le(value->header + ACTOR);
    if (!canonical) {
        if (install(world, value) != 0) {
            return -1;
        }
        world->orders_loaded = 1;
        world->orders.descriptors[world->objects[slot].vehicle.platoon].words[0] = value->behavior;
    } else {
        if (canonical_boundary(world, value) != 0 ||
            world->orders.descriptors[world->objects[slot].vehicle.platoon].words[0] !=
                value->behavior) {
            return -1;
        }
        if (value->candidate != FIST_POOL_NO_SLOT &&
            fist_object_pool_reference(&world->pool, value->candidate, &value->runtime_candidate) !=
                0) {
            return -1;
        }
        if (value->requested_candidate != FIST_POOL_NO_SLOT &&
            fist_object_pool_reference(&world->pool, value->requested_candidate,
                                       &value->runtime_request) != 0) {
            return -1;
        }
    }
    world->combat.selected_slot = fist_read_u16le(value->header + SELECTED);
    world->voice.admitted_at = fist_read_u16le(value->header + LAST);
    world->target_notice = (fist_target_notice){RETAINED_NOTICE, UINT16_MAX, UINT8_MAX, true};
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (retained_aim_fields(actor) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (value->raw[index] != NULL) {
            poison(value->raw[index], fist_unit_state_size(world->pool.slots[index].type));
        }
    }
    const uint16_t tick = fist_read_u16le(value->header + TICK);
    const uint16_t gate = fist_read_u16le(value->header + GATE);
    if ((value->operation & DISCOVER) != 0) {
        fist_target_discovery_result scan = {0};
        if (fist_mission_world_discover_targets(
                world, height,
                (fist_target_discovery_request){slot, tick, gate, value->header[LINK],
                                                value->header[COARSE] != 0},
                &scan) != 0) {
            return -1;
        }
        value->runtime_request = scan.primary;
    }
    return 0;
}

static int checked_acquisition(fist_mission_world *world, fist_mission_world *before,
                               const fist_klc_image *height,
                               fist_target_acquisition_request request, bool enabled,
                               fist_target_acquisition_result *result, int *observed_status) {
    const uint16_t slot = request.slot;
    poison((uint8_t *)result, sizeof(*result));
    fist_target_acquisition_result initial;
    fist_probe_capture(result, sizeof(*result), &initial);
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_acquire_target(NULL, height, request, result) != -1 ||
        fist_mission_world_acquire_target(world, height, request, NULL) != -1 ||
        fist_mission_world_aim_target(NULL, slot, false) != -1 ||
        !fist_probe_unchanged(result, sizeof(*result), &initial) ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    int status = 0;
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (enabled) {
        status = fist_mission_world_acquire_target(world, height, request, result);
    } else {
        *result = (fist_target_acquisition_result){0};
    }
    if (status != 0) {
        if (status != -1 || !fist_probe_unchanged(world, sizeof(*world), before) ||
            !fist_probe_unchanged(result, sizeof(*result), &initial)) {
            return -1;
        }
        *result = (fist_target_acquisition_result){0};
    } else {
        fist_vehicle_state *allowed = &before->objects[slot].vehicle;
        allowed->command.target = actor->command.target;
        allowed->command.candidate = actor->command.candidate;
        allowed->control_flags = actor->control_flags;
        before->random = world->random;
        before->voice = world->voice;
        before->target_notice = world->target_notice;
        if (!fist_probe_unchanged(world, sizeof(*world), before)) {
            return -1;
        }
    }
    *observed_status = status;
    return 0;
}

static int checked_aim(fist_mission_world *world, fist_mission_world *before, uint16_t slot,
                       bool coarse) {
    const fist_vehicle_state *actor = &world->objects[slot].vehicle;
    fist_probe_capture(world, sizeof(*world), before);
    const int status = fist_mission_world_aim_target(world, slot, coarse);
    if (status != 0) {
        return -1;
    }
    fist_vehicle_state *allowed = &before->objects[slot].vehicle;
    allowed->command.target = actor->command.target;
    allowed->command.target_range = actor->command.target_range;
    allowed->command.target_heading = actor->command.target_heading;
    allowed->turret.elevation = actor->turret.elevation;
    if (!fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    return 0;
}

static const size_t profile_components[FIST_UNIT_GROUND_VEHICLE_COUNT] = {23, 12, 12, 22};

static int checked_throttle(fist_mission_world *world, fist_mission_world *before, uint16_t slot,
                            fist_drive_control_events *events) {
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    const size_t component = profile_components[actor->type];
    /* Declared child continuation; the living parent is still open. */
    actor->command.mode = TARGET_COMMAND;
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_throttle_command(world, slot, events) != 0) {
        return -1;
    }
    fist_vehicle_state *allowed = &before->objects[slot].vehicle;
    allowed->drive.throttle = actor->drive.throttle;
    allowed->control_mode = actor->control_mode;
    allowed->components[component] = actor->components[component];
    if (!fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    return 0;
}

static void write_acquisition(const fist_mission_world *world, uint16_t slot, int status,
                              const fist_target_acquisition_result *result,
                              const fist_drive_control_events *events) {
    const fist_vehicle_state *actor = &world->objects[slot].vehicle;
    const size_t component = profile_components[actor->type];
    printf("acquisition %d %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %lu %u %u",
           status, (unsigned)slot, reference_slot(actor->command.target),
           reference_slot(actor->command.candidate), (unsigned)actor->control_flags,
           (unsigned)actor->command.target_heading, (unsigned)(uint16_t)actor->turret.elevation,
           (unsigned)actor->command.target_range, (unsigned)actor->command.discovery_count,
           (unsigned)actor->drive.motion_flags, (unsigned)actor->command.secondary_heading,
           (unsigned)world->target_notice.duration, (unsigned)world->target_notice.type,
           (unsigned)world->target_notice.variant, (unsigned)world->target_notice.enemy,
           (unsigned)result->attempted, (unsigned)result->installed, (unsigned)result->message,
           (unsigned)result->voice.emitted, (unsigned)result->voice.ax, (unsigned)result->voice.dx,
           (unsigned long)result->voice.ecx, (unsigned)world->voice.admitted_at,
           (unsigned)world->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)world->random.words[index]);
    }
    printf(" %d %u %u %u\n", actor->drive.throttle, (unsigned)actor->control_mode,
           (unsigned)actor->components[component], (unsigned)events->refresh_drive_display);
}

static int acquisition_query(fist_mission_world *world, fist_mission_world *before,
                             const fist_klc_image *height, query_case *value, bool canonical) {
    if (prepare_acquisition(world, height, value, canonical) != 0) {
        return -1;
    }
    const fist_target_acquisition_request request = {
        fist_read_u16le(value->header + ACTOR), value->runtime_request,
        fist_read_u16le(value->header + TICK), fist_read_u16le(value->header + GATE),
        (value->operation & AUTOMATIC) != 0};
    fist_target_acquisition_result result = {0};
    fist_drive_control_events events = {0};
    int status = 0;
    if (checked_acquisition(world, before, height, request, (value->operation & ACQUIRE) != 0,
                            &result, &status) != 0) {
        return -1;
    }
    if (status == 0 && (value->operation & AIM) != 0 &&
        checked_aim(world, before, request.slot, value->header[COARSE] != 0) != 0) {
        return -1;
    }
    if (status == 0 && (value->operation & THROTTLE) != 0 &&
        checked_throttle(world, before, request.slot, &events) != 0) {
        return -1;
    }
    write_acquisition(world, request.slot, status, &result, &events);
    return 0;
}

static int promotion_boundary(const fist_mission_world *world, const query_case *value) {
    if (canonical_boundary(world, value) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        if (world->combat.roster[index] != fist_read_u16le(value->roster + (index * 2))) {
            return -1;
        }
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        const uint8_t *raw = value->raw[slot];
        if (raw == NULL) {
            continue;
        }
        const uint16_t type = world->pool.slots[slot].type;
        if (type < FIST_UNIT_GROUND_VEHICLE_COUNT || type == RETIRING) {
            const fist_vehicle_state *actor = &world->objects[slot].vehicle;
            if (actor->platoon != raw[PLATOON] || actor->member != raw[MEMBER] ||
                actor->control_flags != fist_read_u16le(raw + CONTROL_FLAGS)) {
                return -1;
            }
        } else if (type == WRECK && (world->objects[slot].wreck.platoon != raw[WRECK_PLATOON] ||
                                     world->objects[slot].wreck.member != raw[WRECK_MEMBER])) {
            return -1;
        }
    }
    return 0;
}

static int write_promotion(const fist_mission_world *world, uint16_t slot, int status) {
    printf("promotion %d %u", status, (unsigned)slot);
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        printf(" %u", (unsigned)world->combat.roster[index]);
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (world->pool.slots[index].used == 0) {
            continue;
        }
        const uint16_t type = world->pool.slots[index].type;
        if (type < FIST_UNIT_GROUND_VEHICLE_COUNT || type == RETIRING) {
            const fist_vehicle_state *actor = &world->objects[index].vehicle;
            printf(" %u %u %u %u", (unsigned)index, (unsigned)type, (unsigned)actor->member,
                   (unsigned)actor->control_flags);
        } else if (type == WRECK) {
            printf(" %u %u %u 0", (unsigned)index, (unsigned)type,
                   (unsigned)world->objects[index].wreck.member);
        }
    }
    return putchar('\n') == EOF ? -1 : 0;
}

static int checked_promotion(fist_mission_world *world, fist_mission_world *before, uint16_t slot,
                             int *status) {
    fist_probe_capture(world, sizeof(*world), before);
    const fist_vehicle_state *old = &before->objects[slot].vehicle;
    const bool indexed = old->member != 0 && old->member < FIST_UNIT_MEMBERS_PER_PLATOON &&
                         old->platoon < FIST_UNIT_PLATOON_COUNT;
    const size_t index =
        indexed ? ((size_t)old->platoon * FIST_UNIT_MEMBERS_PER_PLATOON) + old->member : 0;
    const uint16_t predecessor = indexed ? before->combat.roster[index - 1] : FIST_POOL_NO_SLOT;
    *status = fist_mission_world_promote_member(world, slot);
    if (*status == 0) {
        before->objects[slot].vehicle.member = world->objects[slot].vehicle.member;
        before->objects[slot].vehicle.control_flags = world->objects[slot].vehicle.control_flags;
        if (indexed) {
            before->combat.roster[index - 1] = world->combat.roster[index - 1];
            before->combat.roster[index] = world->combat.roster[index];
        }
        if (predecessor < FIST_UNIT_REGISTRY_COUNT) {
            if (before->pool.slots[predecessor].type == WRECK) {
                before->objects[predecessor].wreck.member =
                    world->objects[predecessor].wreck.member;
            } else {
                before->objects[predecessor].vehicle.member =
                    world->objects[predecessor].vehicle.member;
            }
        }
    }
    return (*status == 0 || *status == -1) && fist_probe_unchanged(world, sizeof(*world), before)
               ? 0
               : -1;
}

static int promotion_query(fist_mission_world *world, fist_mission_world *before, query_case *value,
                           bool canonical) {
    if (value->operation != PROMOTION || (!canonical && install(world, value) != 0)) {
        return -1;
    }
    if (!canonical) {
        for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
            world->combat.roster[index] = fist_read_u16le(value->roster + (index * 2));
        }
    } else if (promotion_boundary(world, value) != 0) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (value->raw[slot] != NULL) {
            poison(value->raw[slot], fist_unit_state_size(world->pool.slots[slot].type));
        }
    }
    const uint16_t slot = fist_read_u16le(value->header + ACTOR);
    if (fist_mission_world_promote_member(NULL, slot) != -1) {
        return -1;
    }
    for (size_t repeat = 0; repeat < value->behavior; ++repeat) {
        int status = 0;
        if (checked_promotion(world, before, slot, &status) != 0) {
            return -1;
        }
        if (write_promotion(world, slot, status) != 0) {
            return -1;
        }
        if (status != 0) {
            break;
        }
    }
    return 0;
}

static int promotion_rejects(fist_mission_world *world, fist_mission_world *before, uint16_t slot) {
    fist_probe_capture(world, sizeof(*world), before);
    return fist_mission_world_promote_member(world, slot) == -1 &&
                   fist_probe_unchanged(world, sizeof(*world), before)
               ? 0
               : -1;
}

static int checked_maneuver(fist_mission_world *world, fist_mission_world *before,
                            uint8_t operation, fist_obstacle_observation request, int *observed) {
    const uint16_t slot = request.slot;
    fist_probe_capture(world, sizeof(*world), before);
    int status = -1;
    if (operation == MANEUVER) {
        status = fist_mission_world_maneuver(world, slot, request.coarse);
    } else if (operation == IDLE_TURRET) {
        status = fist_mission_world_idle_turret(world, slot);
    } else if (operation == MOTION_OBSTACLE) {
        status = fist_mission_world_observe_obstacle(world, request);
    }
    if (status == 0) {
        const fist_vehicle_state *actor = &world->objects[slot].vehicle;
        fist_vehicle_state *allowed = &before->objects[slot].vehicle;
        if (operation == MANEUVER) {
            allowed->command.maneuver = actor->command.maneuver;
            allowed->command.maneuver_count = actor->command.maneuver_count;
            allowed->command.blocked_count = actor->command.blocked_count;
            allowed->command.maneuver_heading = actor->command.maneuver_heading;
        } else if (operation == IDLE_TURRET) {
            allowed->turret.requested_offset = actor->turret.requested_offset;
            before->random = world->random;
        } else {
            allowed->control_flags = actor->control_flags;
        }
    }
    *observed = status;
    return (status == 0 || status == -1) && fist_probe_unchanged(world, sizeof(*world), before)
               ? 0
               : -1;
}

static int maneuver_query(fist_mission_world *world, fist_mission_world *before, query_case *value,
                          bool canonical) {
    if ((!canonical && install(world, value) != 0) ||
        (canonical && canonical_boundary(world, value) != 0)) {
        return -1;
    }
    const uint16_t slot = fist_read_u16le(value->header + ACTOR);
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    const uint8_t *raw = value->raw[slot];
    enum {
        SAVED_MANEUVER = 69,
        REMAINING = 70,
        BLOCKED = 81,
        MANEUVER_HEADING = 71,
        REQUESTED_OFFSET = 139,
        SAVED_TARGET = 151
    };
    if (canonical && value->behavior == 1) {
        if (raw[SAVED_MANEUVER] != 2 || raw[REMAINING] != 1) {
            return -1;
        }
        /* Explicit research stimulus, not an unobserved mission transition. */
        actor->command.maneuver = 2;
        actor->command.maneuver_count = 1;
    }
    if (canonical && (actor->command.maneuver != raw[SAVED_MANEUVER] ||
                      actor->command.maneuver_count != raw[REMAINING] ||
                      actor->command.blocked_count != raw[BLOCKED] ||
                      actor->command.maneuver_heading != fist_read_u16le(raw + MANEUVER_HEADING) ||
                      actor->control_flags != fist_read_u16le(raw + CONTROL_FLAGS) ||
                      actor->turret.requested_offset != fist_read_u16le(raw + REQUESTED_OFFSET))) {
        return -1;
    }
    if (!canonical) {
        actor->command.target_reference = fist_read_u16le(raw + SAVED_TARGET);
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        if (value->raw[index] != NULL) {
            poison(value->raw[index], fist_unit_state_size(world->pool.slots[index].type));
        }
    }
    int status = 0;
    if (checked_maneuver(
            world, before, value->operation,
            (fist_obstacle_observation){slot, value->candidate, value->header[COARSE] != 0},
            &status) != 0) {
        return -1;
    }
    printf("maneuver %d %u %u %u %u %u %u %u", status, actor->command.maneuver,
           actor->command.maneuver_count, actor->command.blocked_count,
           actor->command.maneuver_heading, actor->control_flags, actor->turret.requested_offset,
           world->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", world->random.words[index]);
    }
    return putchar('\n') == EOF ? -1 : 0;
}

typedef struct {
    uint16_t kind;
    uint16_t replacement;
    uint16_t flags;
    uint8_t cursor;
    uint8_t link;
    uint8_t value;
} maneuver_fixture;

static int maneuver_retention(maneuver_fixture fixture) {
    const uint16_t kind = fixture.kind;
    const uint8_t link = fixture.link;
    const uint8_t value = fixture.value;
    enum {
        REMAINING = 70,
        BLOCKED = 81,
        HEADING = 71,
        SELECTOR = 69,
        BYTE_PATTERN = 17,
        WORD_PATTERN = 257,
        HIGH_SEED = 32768
    };
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE] = {0};
    raw[0] = (uint8_t)kind;
    raw[SELECTOR] = value;
    raw[REMAINING] = (uint8_t)(value ^ UINT8_MAX);
    raw[BLOCKED] = (uint8_t)(value * BYTE_PATTERN);
    raw[HEADING] = value;
    raw[HEADING + 1] = value;
    const fist_unit_definition definition = {.type = kind, .snapshot = {raw, sizeof(raw)}};
    fist_random random = {.words = {1, 2, HIGH_SEED, UINT16_MAX},
                          .next_stream = value % FIST_RANDOM_STREAMS};
    fist_vehicle_state actor = {0};
    if (fist_vehicle_initialize(&definition, &random, link, &actor) != 0) {
        return -1;
    }
    fist_random retained;
    fist_probe_capture(&random, sizeof(random), &retained);
    poison(raw, sizeof(raw));
    for (unsigned stage = 0; stage < 2; ++stage) {
        if (actor.command.maneuver != value ||
            actor.command.maneuver_count != (uint8_t)(value ^ UINT8_MAX) ||
            actor.command.blocked_count != (uint8_t)(value * BYTE_PATTERN) ||
            actor.command.maneuver_heading != (uint16_t)(value * WORD_PATTERN) ||
            (stage == 0 && fist_vehicle_prepare(&actor, link) != 0) ||
            !fist_probe_unchanged(&random, sizeof(random), &retained)) {
            return -1;
        }
    }
    return 0;
}

static int maneuver_presence(fist_mission_world *world, fist_mission_world *before,
                             maneuver_fixture fixture) {
    enum { IDLE_SECOND_DRAW_SEED = 132, IDLE_SECOND_OFFSET = 57344 };
    const uint16_t kind = fixture.kind;
    const uint16_t replacement = fixture.replacement;
    const uint16_t flags = fixture.flags;
    const uint8_t cursor = fixture.cursor;
    fist_mission_world_reset(world);
    fist_pool_allocation binding = {0};
    fist_pool_allocation target = {0};
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){kind, 0}, &binding) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){replacement, 0}, &target) !=
            0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[binding.slot].vehicle;
    *actor = (fist_vehicle_state){.type = kind,
                                  .component_size = fist_vehicle_component_size(kind),
                                  .control_flags = flags,
                                  .turret = {.requested_offset = UINT16_MAX}};
    if (fist_object_pool_reference(&world->pool, target.slot, &actor->command.target) != 0) {
        return -1;
    }
    const fist_object_reference retained = actor->command.target;
    world->random.next_stream = UINT8_MAX;
    for (unsigned stage = 0; stage < 3; ++stage) {
        fist_pool_allocation result = {0};
        if ((stage == 1 &&
             fist_object_pool_release(&world->pool, target.registry_index, &result) != 0) ||
            (stage == 2 && (fist_object_pool_allocate(
                                &world->pool, (fist_pool_request){replacement, 0}, &result) != 0 ||
                            result.slot != target.slot ||
                            fist_object_pool_reference_is_live(&world->pool, retained)))) {
            return -1;
        }
        fist_probe_capture(world, sizeof(*world), before);
        if (fist_mission_world_idle_turret(world, binding.slot) != 0 ||
            !fist_probe_unchanged(world, sizeof(*world), before)) {
            return -1;
        }
    }
    actor->command.target = (fist_object_reference){0};
    int status = 0;
    if (checked_maneuver(world, before, IDLE_TURRET,
                         (fist_obstacle_observation){binding.slot, FIST_POOL_NO_SLOT, false},
                         &status) != 0 ||
        status != ((flags & 4U) != 0 ? 0 : -1)) {
        return -1;
    }
    actor->command.target_reference = UINT16_MAX;
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_idle_turret(world, binding.slot) != 0 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    actor->command.target_reference = 0;
    world->random.next_stream = cursor;
    world->random.words[cursor] =
        IDLE_SECOND_DRAW_SEED; /* Next output 65 enters the second-draw branch. */
    world->random.words[(cursor + 1U) % FIST_RANDOM_STREAMS] = 2;
    if (checked_maneuver(world, before, IDLE_TURRET,
                         (fist_obstacle_observation){binding.slot, FIST_POOL_NO_SLOT, false},
                         &status) != 0 ||
        status != 0 ||
        actor->turret.requested_offset != ((flags & 4U) != 0 ? UINT16_MAX : IDLE_SECOND_OFFSET)) {
        return -1;
    }
    actor->command.maneuver = 2;
    actor->command.maneuver_count = 1;
    world->objects[target.slot].vehicle.type = FIST_UNIT_TYPE_COUNT;
    if (checked_maneuver(world, before, MANEUVER,
                         (fist_obstacle_observation){binding.slot, target.slot, false},
                         &status) != 0 ||
        status != -1 ||
        checked_maneuver(world, before, MOTION_OBSTACLE,
                         (fist_obstacle_observation){binding.slot, target.slot, false},
                         &status) != 0 ||
        status != -1) {
        return -1;
    }
    actor->command.maneuver_count = 2;
    if (checked_maneuver(world, before, MANEUVER,
                         (fist_obstacle_observation){binding.slot, target.slot, false},
                         &status) != 0 ||
        status != 0) {
        return -1;
    }
    return 0;
}

static int maneuver_order_contract(fist_mission_world *world, fist_mission_world *before) {
    enum { BODY_DISTANCE = 3000, BODY_SCALE = 1024, COLLIDABLE = 64, EXHAUSTED_TURN = 54616 };
    fist_mission_world_reset(world);
    fist_pool_allocation actor_binding = {0};
    fist_pool_allocation first = {0};
    fist_pool_allocation later = {0};
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){0, 0}, &actor_binding) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){TARGET, 0}, &first) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){ARTILLERY, 0}, &later) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[actor_binding.slot].vehicle;
    *actor = (fist_vehicle_state){.type = 0,
                                  .component_size = fist_vehicle_component_size(0),
                                  .projection_scale = BODY_SCALE,
                                  .command = {.maneuver = 2, .maneuver_count = 1}};
    world->objects[first.slot].other = (fist_other_actor){.allocation = first,
                                                          .pose = {.y = BODY_DISTANCE},
                                                          .projection_scale = BODY_SCALE,
                                                          .flags = COLLIDABLE};
    world->objects[later.slot].other.allocation.type = FIST_UNIT_TYPE_COUNT;
    const fist_obstacle_observation request = {actor_binding.slot, FIST_POOL_NO_SLOT, false};
    int status = 0;
    if (checked_maneuver(world, before, MANEUVER, request, &status) != 0 || status != 0 ||
        actor->command.maneuver_heading != EXHAUSTED_TURN) {
        return -1;
    }
    /* An earlier hit does not inspect the invalid later body's payload. */
    const fist_pool_entry saved = world->pool.registry[first.registry_index];
    world->pool.registry[first.registry_index] = world->pool.registry[later.registry_index];
    world->pool.registry[later.registry_index] = saved;
    actor->command.maneuver = 2;
    actor->command.maneuver_count = 1;
    if (checked_maneuver(world, before, MANEUVER, request, &status) != 0 || status != -1) {
        return -1;
    }
    /* The actual type-21 filter precedes all payload reads. */
    fist_pool_allocation current = {0};
    if (fist_object_pool_find(&world->pool, later.slot, &current) != 0 ||
        fist_object_pool_retype(&world->pool, current, TREE, &current) != 0 ||
        checked_maneuver(world, before, MANEUVER, request, &status) != 0 || status != 0 ||
        actor->command.maneuver_heading != EXHAUSTED_TURN) {
        return -1;
    }
    actor->component_size = 0;
    for (unsigned operation = MANEUVER; operation <= MOTION_OBSTACLE; ++operation) {
        if (checked_maneuver(world, before, (uint8_t)operation, request, &status) != 0 ||
            status != -1) {
            return -1;
        }
    }
    return 0;
}

static int maneuver_contracts(fist_mission_world *world, fist_mission_world *before) {
    if (fist_mission_world_maneuver(NULL, 0, false) != -1 ||
        fist_mission_world_idle_turret(NULL, 0) != -1 ||
        fist_mission_world_observe_obstacle(NULL, (fist_obstacle_observation){0}) != -1) {
        return -1;
    }
    enum { LINKS = 3, BYTE_VALUES = 256, CONTROL_CASES = 2 };
    for (unsigned context = 0; context < FIST_UNIT_GROUND_VEHICLE_COUNT * LINKS * BYTE_VALUES;
         ++context) {
        const maneuver_fixture fixture = {.kind = (uint16_t)(context / (LINKS * BYTE_VALUES)),
                                          .link = (uint8_t)((context / BYTE_VALUES) % LINKS),
                                          .value = (uint8_t)context};
        if (maneuver_retention(fixture) != 0) {
            return -1;
        }
    }
    for (unsigned context = 0;
         context < FIST_UNIT_GROUND_VEHICLE_COUNT * FIST_UNIT_GROUND_VEHICLE_COUNT * CONTROL_CASES *
                       FIST_RANDOM_STREAMS;
         ++context) {
        const maneuver_fixture fixture = {
            .kind = (uint16_t)(context / (FIST_UNIT_GROUND_VEHICLE_COUNT * CONTROL_CASES *
                                          FIST_RANDOM_STREAMS)),
            .replacement = (uint16_t)((context / (CONTROL_CASES * FIST_RANDOM_STREAMS)) %
                                      FIST_UNIT_GROUND_VEHICLE_COUNT),
            .flags = (uint16_t)(((context / FIST_RANDOM_STREAMS) % CONTROL_CASES) * 4U),
            .cursor = (uint8_t)(context % FIST_RANDOM_STREAMS)};
        if (maneuver_presence(world, before, fixture) != 0) {
            return -1;
        }
    }
    return maneuver_order_contract(world, before);
}

static int promotion_identity_case(fist_mission_world *world, fist_mission_world *before,
                                   uint16_t kind) {
    fist_mission_world_reset(world);
    fist_pool_allocation actor_binding = {0};
    fist_pool_allocation predecessor_binding = {0};
    fist_pool_allocation found = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){kind, 0, 1}, &actor_binding) !=
            0 ||
        fist_object_pool_import(&world->pool, (fist_pool_import){0, 0, 2}, &predecessor_binding) !=
            0 ||
        fist_object_pool_find(&world->pool, actor_binding.slot, &found) != FIST_POOL_UNAVAILABLE) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[actor_binding.slot].vehicle;
    fist_vehicle_state *predecessor = &world->objects[predecessor_binding.slot].vehicle;
    *actor = (fist_vehicle_state){.type = kind,
                                  .component_size = fist_vehicle_component_size(kind),
                                  .member = 1,
                                  .control_flags = UINT16_MAX};
    *predecessor =
        (fist_vehicle_state){.type = 0, .drive = {.motion_flags = PROMOTION_BLOCKED_FLAG}};
    world->combat.roster[0] = predecessor_binding.slot;
    world->combat.roster[1] = actor_binding.slot;
    int status = 0;
    if (checked_promotion(world, before, actor_binding.slot, &status) != 0 || status != 0 ||
        actor->member != 0 || predecessor->member != 1 ||
        world->combat.roster[0] != actor_binding.slot ||
        world->combat.roster[1] != predecessor_binding.slot ||
        actor->control_flags != (uint16_t)(UINT16_MAX & ~(unsigned)PROMOTION_GOAL_VALID)) {
        return -1;
    }
    /* A leader must ignore unused platoon/roster/orders/RNG inputs. */
    actor->platoon = UINT8_MAX;
    world->combat.roster[0] = FIST_POOL_NO_SLOT;
    world->random.next_stream = UINT8_MAX;
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_promote_member(world, actor_binding.slot) != 0 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    actor->platoon = 0;
    actor->member = 1;
    predecessor->member = 0;
    world->combat.roster[0] = predecessor_binding.slot;
    world->combat.roster[1] = actor_binding.slot;
    predecessor->type = WRECK; /* Used mistagged payload must fail atomically. */
    if (promotion_rejects(world, before, actor_binding.slot) != 0) {
        return -1;
    }
    actor->drive.motion_flags = PROMOTION_BLOCKED_FLAG;
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_promote_member(world, actor_binding.slot) != 0 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    actor->drive.motion_flags = 0;
    predecessor->type = RETIRING;
    if (fist_object_pool_retype(&world->pool, predecessor_binding, RETIRING,
                                &predecessor_binding) != 0 ||
        checked_promotion(world, before, actor_binding.slot, &status) != 0 || status != 0 ||
        predecessor->member != 1 || actor->member != 0) {
        return -1;
    }
    actor->member = 1;
    world->combat.roster[0] = predecessor_binding.slot;
    if (fist_object_pool_release(&world->pool, predecessor_binding.registry_index, &found) != 0 ||
        promotion_rejects(world, before, actor_binding.slot) != 0) {
        return -1;
    }
    world->combat.roster[0] = FIST_POOL_NO_SLOT;
    actor->drive.motion_flags = UINT8_MAX;
    if (checked_promotion(world, before, actor_binding.slot, &status) != 0 || status != 0 ||
        actor->member != 0) {
        return -1;
    }
    actor->component_size = 0;
    if (promotion_rejects(world, before, actor_binding.slot) != 0 ||
        promotion_rejects(world, before, FIST_POOL_NO_SLOT) != 0) {
        return -1;
    }
    fist_mission_world_reset(world);
    return promotion_rejects(world, before, actor_binding.slot);
}

static int promotion_contracts(fist_mission_world *world, fist_mission_world *before) {
    for (unsigned kind = 0; kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
        if (promotion_identity_case(world, before, (uint16_t)kind) != 0) {
            return -1;
        }
    }
    return 0;
}

static int invalid_acquisition_state(fist_mission_world *world, fist_mission_world *before,
                                     const fist_klc_image *height,
                                     fist_target_acquisition_request request) {
    enum {
        INVALID_PLATOON = FIST_UNIT_PLATOON_COUNT,
        INVALID_CASES = 8,
        BAD_PLATOON = 5,
        BAD_TARGET = 6
    };
    fist_vehicle_state *actor = &world->objects[request.slot].vehicle;
    const fist_vehicle_state saved_actor = *actor;
    const uint8_t saved_cursor = world->random.next_stream;
    for (unsigned index = 0; index < INVALID_CASES; ++index) {
        switch (index) {
        case 0:
            world->preparation.prepared = 0;
            break;
        case 1:
            world->orders_loaded = 0;
            break;
        case 2:
            world->random.next_stream = FIST_RANDOM_STREAMS;
            break;
        case 3:
            actor->type = RETIRING;
            break;
        case 4:
            actor->component_size = 0;
            break;
        case BAD_PLATOON:
            actor->platoon = INVALID_PLATOON;
            break;
        case BAD_TARGET:
            actor->command.target = (fist_object_reference){0, 1};
            break;
        default:
            actor->command.candidate = (fist_object_reference){0, 1};
            break;
        }
        fist_target_acquisition_result result = {0};
        fist_target_acquisition_result initial;
        poison((uint8_t *)&result, sizeof(result));
        fist_probe_capture(&result, sizeof(result), &initial);
        fist_probe_capture(world, sizeof(*world), before);
        if (fist_mission_world_acquire_target(world, height, request, &result) != -1 ||
            fist_mission_world_aim_target(world, request.slot, false) != -1 ||
            !fist_probe_unchanged(world, sizeof(*world), before) ||
            !fist_probe_unchanged(&result, sizeof(result), &initial)) {
            return -1;
        }
        *actor = saved_actor;
        world->preparation.prepared = 1;
        world->orders_loaded = 1;
        world->random.next_stream = saved_cursor;
    }
    return 0;
}

static int acquisition_lifetime_case(fist_mission_world *world, fist_mission_world *before,
                                     uint16_t kind, uint16_t replacement) {
    const uint8_t pixel = 0;
    const fist_klc_image height = {.width = 1, .height = 1, .pixels = (uint8_t *)&pixel};
    fist_mission_world_reset(world);
    fist_pool_allocation binding = {0};
    fist_pool_allocation target = {0};
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){kind, 0}, &binding) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){0, 0}, &target) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[binding.slot].vehicle;
    *actor = (fist_vehicle_state){.type = kind,
                                  .altitude = FIXTURE_ALTITUDE,
                                  .component_size = fist_vehicle_component_size(kind),
                                  .turret = {.elevation = INT16_MIN},
                                  .command = {.discovery_count = 1,
                                              .target_heading = UINT16_MAX,
                                              .target_range = UINT16_MAX}};
    world->preparation.prepared = 1;
    world->orders_loaded = 1;
    world->random.words[0] = 2; /* Original next return zero admits every behavior. */
    world->combat.selected_slot = binding.slot;
    fist_object_reference old = {0};
    fist_pool_allocation removed = {0};
    if (fist_object_pool_reference(&world->pool, target.slot, &old) != 0 ||
        fist_object_pool_release(&world->pool, target.registry_index, &removed) != 0) {
        return -1;
    }
    actor->command.target = old;
    actor->command.candidate = old;
    fist_random expected_random = world->random;
    uint16_t draw = 0;
    fist_target_acquisition_result result = {0};
    fist_target_acquisition_request request = {binding.slot, {0, FIST_POOL_NO_SLOT}, 0, 0, true};
    if (fist_random_next(&expected_random, &draw) != 0 || draw != 0 ||
        fist_mission_world_acquire_target(world, NULL, request, &result) != 0 ||
        !result.attempted || result.installed || result.message || result.voice.emitted ||
        actor->command.target.lifetime != 0 || actor->command.candidate.lifetime != 0 ||
        actor->control_flags != ATTEMPT_FLAG ||
        !fist_probe_unchanged(&world->random, sizeof(expected_random), &expected_random)) {
        return -1;
    }
    fist_pool_allocation fresh = {0};
    if (fist_object_pool_allocate(&world->pool, (fist_pool_request){replacement, 0}, &fresh) != 0 ||
        fresh.slot != old.slot || fresh.registry_index != target.registry_index ||
        fresh.value != target.value || fist_object_pool_reference_is_live(&world->pool, old)) {
        return -1;
    }
    world->objects[fresh.slot].vehicle = (fist_vehicle_state){.type = replacement,
                                                              .map_x = -FIXTURE_DISTANCE,
                                                              .altitude = FIXTURE_ALTITUDE,
                                                              .object_flags = OPPOSING_FLAGS};
    actor->command.target = old;
    actor->command.candidate = old;
    request.candidate = old;
    request.automatic = false;
    if (fist_mission_world_acquire_target(world, &height, request, &result) != 0 ||
        !result.attempted || result.installed || actor->command.target.lifetime != 0 ||
        actor->command.candidate.lifetime != 0) {
        return -1;
    }
    actor->command.target = old;
    if (fist_mission_world_aim_target(world, binding.slot, false) != 0 ||
        actor->command.target.lifetime != 0 || actor->command.target_heading != UINT16_MAX ||
        actor->command.target_range != UINT16_MAX || actor->turret.elevation != INT16_MIN) {
        return -1;
    }
    fist_object_reference live = {0};
    if (fist_object_pool_reference(&world->pool, fresh.slot, &live) != 0 ||
        live.lifetime == old.lifetime) {
        return -1;
    }
    request.candidate = live;
    request.tick = VOICE_INTERVAL;
    request.voice_gate = UINT16_MAX;
    if (fist_mission_world_acquire_target(world, &height, request, &result) != 0 ||
        !result.installed || !result.message || !result.voice.emitted ||
        actor->command.target.lifetime != live.lifetime ||
        world->target_notice.type != replacement ||
        fist_mission_world_aim_target(world, binding.slot, false) != 0 ||
        actor->command.target_range == UINT16_MAX) {
        return -1;
    }
    const uint16_t retyped = replacement == 0 ? 2 : 0;
    if (fist_object_pool_retype(&world->pool, fresh, retyped, &fresh) != 0 ||
        !fist_object_pool_reference_is_live(&world->pool, live)) {
        return -1;
    }
    world->objects[fresh.slot].vehicle.type = retyped;
    fist_pool_allocation other = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){0, fresh.registry_index, 1},
                                &other) != 0 ||
        fist_object_pool_find(&world->pool, live.slot, &removed) != FIST_POOL_UNAVAILABLE ||
        fist_mission_world_acquire_target(world, &height, request, &result) != 0 ||
        !result.installed || world->target_notice.type != retyped ||
        actor->command.target.lifetime != live.lifetime ||
        fist_mission_world_aim_target(world, binding.slot, true) != 0) {
        return -1;
    }
    /* Used projection/height failure preserves all notification/RNG/actor/output bytes. */
    fist_probe_capture(world, sizeof(*world), before);
    fist_target_acquisition_result initial;
    fist_probe_capture(&result, sizeof(result), &initial);
    if (fist_mission_world_acquire_target(world, NULL, request, &result) != -1 ||
        !fist_probe_unchanged(world, sizeof(*world), before) ||
        !fist_probe_unchanged(&result, sizeof(result), &initial) ||
        invalid_acquisition_state(world, before, &height, request) != 0) {
        return -1;
    }
    /* The same lifetime cannot become live after a complete reset. */
    fist_mission_world_reset(world);
    if (fist_object_pool_reference_is_live(&world->pool, live) ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){kind, 0}, &other) != 0 ||
        fist_object_pool_allocate(&world->pool, (fist_pool_request){replacement, 0}, &other) != 0 ||
        fist_object_pool_reference_is_live(&world->pool, live)) {
        return -1;
    }
    return 0;
}

static int acquisition_lifetimes(fist_mission_world *world, fist_mission_world *before) {
    for (unsigned kind = 0; kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
        for (uint16_t replacement = 0; replacement <= 2; replacement += 2) {
            if (acquisition_lifetime_case(world, before, (uint16_t)kind, replacement) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int load_mission(const char *path, const fist_klc_image *height, fist_mission_world *world,
                        bool acquisition) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return -1;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    fist_units units = {0};
    fist_mission_orders orders = {0};
    const fist_random random = {.words = {1, 2, 32768, acquisition ? 2 : UINT16_MAX},
                                .next_stream = 3};
    int status = data == NULL || closed != 0 ? -1 : fist_scenario_decode(data, size, &scenario);
    if (status == 0) {
        status = fist_units_decode(&scenario, &units);
    }
    if (status == 0) {
        status = fist_mission_orders_decode(&scenario, &orders);
    }
    if (status == 0) {
        status = fist_mission_world_initialize(&units, &random, 0, world);
        if (status == 0) {
            world->orders = orders;
            world->orders_loaded = 1;
        }
    }
    fist_units_destroy(&units);
    if (data != NULL) {
        poison(data, size);
    }
    free(data);
    return status == 0 ? fist_mission_world_prepare(world, height, 0) : status;
}

static uint8_t *prepare_probe(fist_mission_world *world, fist_mission_world *before,
                              const fist_klc_image *height, bool acquisition, bool promotion) {
    if (lifetime_contracts(world, before) != 0 ||
        (acquisition && acquisition_lifetimes(world, before) != 0) ||
        (promotion && promotion_contracts(world, before) != 0)) {
        return NULL;
    }
    const size_t bytes = (size_t)height->width * height->height;
    uint8_t *pixels = malloc(bytes);
    if (pixels != NULL) {
        for (size_t index = 0; index < bytes; ++index) {
            pixels[index] = height->pixels[index];
        }
    }
    return pixels;
}

static int run_query(fist_mission_world *world, fist_mission_world *before,
                     const fist_klc_image *height, query_case *value, bool canonical,
                     bool acquisition, bool promotion) {
    if (value->operation >= MANEUVER && value->operation <= MOTION_OBSTACLE) {
        return maneuver_query(world, before, value, canonical);
    }
    if (promotion) {
        return promotion_query(world, before, value, canonical);
    }
    return acquisition ? acquisition_query(world, before, height, value, canonical)
                       : query(world, before, height, value, canonical);
}

static unsigned probe_mode(int argc, char **argv) {
    if (argc <= 1) {
        return 0;
    }
    if (strcmp(argv[1], "--acquisition") == 0) {
        return ACQUIRE;
    }
    if (strcmp(argv[1], "--promotion") == 0) {
        return PROMOTION;
    }
    return strcmp(argv[1], "--maneuver") == 0 ? MANEUVER : 0;
}

int main(int argc, char **argv) {
    const unsigned mode = probe_mode(argc, argv);
    const bool acquisition = mode == ACQUIRE;
    const bool promotion = mode == PROMOTION;
    if (mode != 0) {
        --argc;
        ++argv;
    }
    if (argc != 2 && argc != 3) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[argc - 1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_klc_image height = {0};
    query_case *cases = NULL;
    uint32_t count = 0;
    int status =
        data == NULL || closed != 0 ? -1 : decode(data, size, &height, &cases, &count, mode != 0);
    fist_mission_world *world = calloc(1, sizeof(*world));
    fist_mission_world *before = malloc(sizeof(*before));
    uint8_t *pixels = NULL;
    if (world == NULL || before == NULL) {
        status = -1;
    }
    if (status == 0 && mode == MANEUVER) {
        status = maneuver_contracts(world, before);
    }
    if (status == 0) {
        pixels = prepare_probe(world, before, &height, acquisition, promotion);
        if (pixels == NULL) {
            status = -1;
        }
    }
    if (status == 0 && argc == 3) {
        status = load_mission(argv[1], &height, world, acquisition);
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        status =
            run_query(world, before, &height, &cases[index], argc == 3, acquisition, promotion);
        if (memcmp(pixels, height.pixels, (size_t)height.width * height.height) != 0) {
            status = -1;
        }
    }
    free(before);
    free(world);
    free(cases);
    free(data);
    free(pixels);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
