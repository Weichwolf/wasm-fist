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
    TARGET_COMMAND = 6
};

typedef struct {
    const uint8_t *header;
    const uint8_t *registry;
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
    if (actor >= FIST_UNIT_REGISTRY_COUNT || value->raw[actor] == NULL ||
        fist_read_u16le(value->raw[actor]) >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        (target != FIST_POOL_NO_SLOT && target >= FIST_UNIT_REGISTRY_COUNT)) {
        return -1;
    }
    return 0;
}

static int decode_case(uint8_t *data, size_t size, size_t *cursor, query_case *value) {
    size_t position = *cursor;
    if (position > size || size - position < HEADER + REGISTRY_BYTES) {
        return -1;
    }
    value->header = data + position;
    const uint16_t records = fist_read_u16le(value->header + OBJECTS);
    if (records > FIST_UNIT_REGISTRY_COUNT || value->header[CURSOR] >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    position += HEADER;
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
                  uint32_t *count) {
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
        if (decode_case(data, size, &position, &values[index]) != 0) {
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
        object->vehicle = (fist_vehicle_state){.type = type,
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

static int install_pool(fist_mission_world *world, const query_case *value,
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
    if (target != FIST_POOL_NO_SLOT && ends[target >= FIST_POOL_SHORT_SLOTS] <= target) {
        ends[target >= FIST_POOL_SHORT_SLOTS] = (size_t)target + 1;
    }
    if (allocate_arena(world, value, 0, ends[0], SHELL) != 0 ||
        allocate_arena(world, value, FIST_POOL_SHORT_SLOTS, ends[1], 0) != 0) {
        return -1;
    }
    if (target != FIST_POOL_NO_SLOT &&
        fist_object_pool_reference(&world->pool, target, old_target) != 0) {
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

static int install(fist_mission_world *world, const query_case *value) {
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
                 const fist_klc_image *height, const query_case *value, bool canonical) {
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

static int load_mission(const char *path, const fist_klc_image *height, fist_mission_world *world) {
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
    const fist_random random = {.words = {1, 2, 32768, 65535}, .next_stream = 3};
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

int main(int argc, char **argv) {
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
    int status = data == NULL || closed != 0 ? -1 : decode(data, size, &height, &cases, &count);
    fist_mission_world *world = calloc(1, sizeof(*world));
    fist_mission_world *before = malloc(sizeof(*before));
    uint8_t *pixels = NULL;
    if (world == NULL || before == NULL) {
        status = -1;
    }
    if (status == 0) {
        status = lifetime_contracts(world, before);
        const size_t bytes = (size_t)height.width * height.height;
        pixels = malloc(bytes);
        if (pixels == NULL) {
            status = -1;
        } else {
            for (size_t index = 0; index < bytes; ++index) {
                pixels[index] = height.pixels[index];
            }
        }
    }
    if (status == 0 && argc == 3) {
        status = load_mission(argv[1], &height, world);
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        status = query(world, before, &height, &cases[index], argc == 3);
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
