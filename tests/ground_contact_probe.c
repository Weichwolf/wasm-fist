#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/mission_update.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    BATCH_HEADER = 8,
    HEADER = 48,
    REGISTRY_BYTES = FIST_UNIT_REGISTRY_COUNT * 4,
    H_ACTOR = 0,
    H_SELECTED = 2,
    H_WRAPPER = 4,
    H_COARSE = 5,
    H_SOURCE = 6,
    H_FACTORS = 8,
    H_SIDE = 12,
    H_CURSOR = 13,
    H_SEEDS = 14,
    H_TREES = 22,
    H_SELECTOR = 24,
    H_OBSTACLE_PACKET = 26,
    H_OBSTACLE_ATTENUATION = 28,
    H_TREE_PACKET = 30,
    H_TREE_ATTENUATION = 32,
    H_INVALID = 33,
    H_OBJECTS = 34,
    H_STEPS = 36,
    H_OVERLAP = 38,
    H_INVALID_SLOT = 40,
    TREE = 21,
    WRECK = 23,
    HIGH_SEED = 32768,
    BYTE_VALUES = 256,
    OUTPUT_CANARY = 0x5a,
    COLLIDABLE = 64,
    EXCLUDED = 16,
    CONTACT = 2,
    RAW_CONTACT = 98,
    RAW_COOLDOWN = 147,
    RAW_TREE_DAMAGE = 26,
    POSITION_Y = 8,
    ALTITUDE = 12,
    HEADING = 16,
    FLAGS = 22,
    SECONDARY = 23,
    GROUND = 24,
    VARIANT = 25,
    SCALE = 20,
    MAX_DETAIL = 4096,
    EXPLOSION = 4,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    TARGET = 26,
    ARTILLERY = 27,
    SHELL = 8,
    SMOKE = 17,
    MUZZLE = 18,
    RAW_MOVEMENT = 93,
    RAW_BEHAVIOR = 62,
    HEIGHT_LOW = 63,
    HEIGHT_MID = 128,
    NULL_AUDIO = 1,
    BAD_RANDOM = 2,
    BAD_BODY = 3,
    BAD_TREE = 4,
    BAD_COMPONENT = 5,
    SOURCE_SIDE = 8,
    TREE_DAMAGE = 101
};

typedef struct {
    const uint8_t *header;
    const uint8_t *registry;
    const uint8_t *raw[FIST_UNIT_REGISTRY_COUNT];
} contact_case;

typedef struct {
    fist_mission_world world;
    fist_mission_world before;
    fist_mission_world prepared;
} contact_state;

static uint8_t *read_path(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    uint8_t *data = fist_probe_read_file(file, size);
    if (fclose(file) != 0) {
        free(data);
        return NULL;
    }
    return data;
}

static int decode_case(const uint8_t *data, size_t size, size_t *position, contact_case *value) {
    if (*position > size || size - *position < HEADER + REGISTRY_BYTES) {
        return -1;
    }
    value->header = data + *position;
    value->registry = value->header + HEADER;
    *position += HEADER + REGISTRY_BYTES;
    const uint16_t count = fist_read_u16le(value->header + H_OBJECTS);
    if (count > FIST_UNIT_REGISTRY_COUNT || fist_read_u16le(value->header + H_STEPS) == 0) {
        return -1;
    }
    for (size_t index = 0; index < count; ++index) {
        if (*position > size || size - *position < 2 * sizeof(uint16_t)) {
            return -1;
        }
        const uint16_t slot = fist_read_u16le(data + *position);
        *position += sizeof(uint16_t);
        const size_t length = fist_unit_state_size(fist_read_u16le(data + *position));
        if (slot >= FIST_UNIT_REGISTRY_COUNT || value->raw[slot] != NULL || length == 0 ||
            length > size - *position ||
            (length == FIST_UNIT_EXTENDED_SIZE) != (slot >= FIST_POOL_SHORT_SLOTS)) {
            return -1;
        }
        value->raw[slot] = data + *position;
        *position += length;
    }
    const uint16_t actor = fist_read_u16le(value->header + H_ACTOR);
    return actor < FIST_UNIT_REGISTRY_COUNT && value->raw[actor] != NULL &&
                   fist_read_u16le(value->raw[actor]) < FIST_UNIT_GROUND_VEHICLE_COUNT
               ? 0
               : -1;
}

static fist_unit_definition definition(const uint8_t *raw, fist_pool_allocation allocation) {
    return (fist_unit_definition){.type = allocation.type,
                                  .registry_index = allocation.registry_index,
                                  .generation = allocation.value,
                                  .map_x = fist_read_i32le(raw + 4),
                                  .map_y = fist_read_i32le(raw + POSITION_Y),
                                  .altitude = fist_read_i32le(raw + ALTITUDE),
                                  .heading = fist_read_u16le(raw + HEADING),
                                  .snapshot = {raw, fist_unit_state_size(allocation.type)}};
}

static int restore_object(fist_mission_world *world, uint16_t slot, const uint8_t *raw) {
    const uint16_t type = fist_read_u16le(raw);
    fist_pool_allocation allocation = {.type = type, .slot = slot, .registry_index = slot};
    const int bound = fist_object_pool_find(&world->pool, slot, &allocation);
    if (bound != FIST_POOL_OK && bound != FIST_POOL_UNAVAILABLE) {
        return -1;
    }
    const fist_unit_definition saved = definition(raw, allocation);
    fist_mission_object *object = &world->objects[slot];
    if (type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return fist_vehicle_restore(&saved, &object->vehicle);
    }
    if (type == TREE) {
        return fist_tree_restore(&saved, allocation, &object->tree);
    }
    const fist_object_pose pose = {saved.map_x, saved.map_y, saved.altitude, saved.heading};
    const uint16_t scale = fist_read_u16le(raw + SCALE);
    /* Probe-only common projections, not invented class initialization. The
     * contact boundary reads pose/flags/scale and only mutates type21 payloads. */
    switch (type) {
    case EXPLOSION:
        object->explosion = (fist_explosion){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT:
    case TARGET:
    case ARTILLERY:
        object->other = (fist_other_actor){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    case SHELL:
        object->projectile = (fist_projectile){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    case SMOKE:
        object->smoke = (fist_drifting_smoke){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    case MUZZLE:
        object->muzzle = (fist_muzzle_smoke){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    case WRECK:
        object->wreck = (fist_vehicle_wreck){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    default:
        object->saved_base = (fist_saved_object_base){
            .allocation = allocation, .pose = pose, .projection_scale = scale, .flags = raw[FLAGS]};
        break;
    }
    return 0;
}

static int install_arena(fist_mission_world *world, const contact_case *value, size_t start,
                         size_t end) {
    for (size_t slot = start; slot < end; ++slot) {
        uint16_t type = start == 0 ? SHELL : 0;
        if (value->raw[slot] != NULL) {
            type = fist_read_u16le(value->raw[slot]);
        }
        fist_pool_allocation allocation = {0};
        if (fist_object_pool_import(&world->pool, (fist_pool_import){type, (uint16_t)slot, 1},
                                    &allocation) != FIST_POOL_OK ||
            allocation.slot != slot) {
            return -1;
        }
    }
    return 0;
}

static int install(fist_mission_world *world, const contact_case *value) {
    fist_mission_world_reset(world);
    size_t ends[2] = {0, FIST_POOL_SHORT_SLOTS};
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (value->raw[slot] != NULL) {
            ends[slot >= FIST_POOL_SHORT_SLOTS] = slot + 1;
        }
    }
    if (install_arena(world, value, 0, ends[0]) != 0 ||
        install_arena(world, value, FIST_POOL_SHORT_SLOTS, ends[1]) != 0) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (world->pool.slots[slot].used != 0 && value->raw[slot] == NULL) {
            fist_pool_allocation released = {0};
            if (fist_object_pool_release(&world->pool, (uint16_t)slot, &released) != FIST_POOL_OK) {
                return -1;
            }
        }
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        world->pool.registry[index] =
            (fist_pool_entry){fist_read_u16le(value->registry + (index * 4)),
                              fist_read_u16le(value->registry + (index * 4) + sizeof(uint16_t))};
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (value->raw[slot] != NULL &&
            restore_object(world, (uint16_t)slot, value->raw[slot]) != 0) {
            return -1;
        }
    }
    return 0;
}

static int load_prepared(const char *path, const fist_klc_image *height, fist_mission_world *out) {
    size_t size = 0;
    uint8_t *data = read_path(path, &size);
    fist_scenario scenario = {0};
    fist_units units = {0};
    fist_mission_orders orders = {0};
    const fist_random random = {.words = {1, 2, HIGH_SEED, UINT16_MAX}, .next_stream = 3};
    int status = data == NULL ? -1 : fist_scenario_decode(data, size, &scenario);
    if (status == 0) {
        status = fist_units_decode(&scenario, &units);
    }
    if (status == 0) {
        status = fist_mission_orders_decode(&scenario, &orders);
    }
    if (status == 0) {
        status = fist_mission_world_initialize(&units, &random, 0, out);
        if (status == 0) {
            out->orders = orders;
            out->orders_loaded = 1;
            status = fist_mission_world_prepare(out, height, 0);
        }
    }
    fist_units_destroy(&units);
    free(data);
    return status;
}

static int force_overlap(fist_mission_world *world, uint16_t slot) {
    uint16_t first = FIST_POOL_NO_SLOT;
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const uint16_t candidate = world->pool.registry[index].slot;
        if (candidate == FIST_POOL_NO_SLOT || candidate == slot) {
            continue;
        }
        fist_object_pose storage = {0};
        fist_mission_view view = {0};
        if (fist_mission_world_view(world, candidate, &storage, &view) != 0) {
            return -1;
        }
        if ((view.flags & COLLIDABLE) != 0 && (view.flags & EXCLUDED) == 0) {
            if (first == FIST_POOL_NO_SLOT) {
                first = candidate;
            }
            if (world->pool.slots[candidate].type == TREE) {
                first = candidate;
                break;
            }
        }
    }
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (first != FIST_POOL_NO_SLOT) {
        fist_object_pose storage = {0};
        fist_mission_view view = {0};
        if (fist_mission_world_view(world, first, &storage, &view) != 0) {
            return -1;
        }
        actor->map_x = view.pose->x;
        actor->map_y = view.pose->y;
    }
    actor->contact_flags &= (uint8_t)~CONTACT;
    actor->contact_cooldown = 0;
    return 0;
}

static int canonical_boundary(fist_mission_world *world, const contact_case *value) {
    const uint16_t actor_slot = fist_read_u16le(value->header + H_ACTOR);
    if (value->header[H_OVERLAP] != 0 && force_overlap(world, actor_slot) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const fist_pool_entry entry = world->pool.registry[index];
        if (entry.slot != fist_read_u16le(value->registry + (index * 4)) ||
            entry.value != fist_read_u16le(value->registry + (index * 4) + sizeof(uint16_t)) ||
            (world->pool.slots[index].used != 0) != (value->raw[index] != NULL)) {
            return -1;
        }
        const uint8_t *raw = value->raw[index];
        if (raw == NULL) {
            continue;
        }
        fist_object_pose storage = {0};
        fist_mission_view view = {0};
        const uint16_t type = world->pool.slots[index].type;
        if (type != fist_read_u16le(raw) ||
            fist_mission_world_view(world, (uint16_t)index, &storage, &view) != 0 ||
            view.pose->x != fist_read_i32le(raw + 4) ||
            view.pose->y != fist_read_i32le(raw + POSITION_Y) ||
            view.projection_scale != fist_read_u16le(raw + SCALE) || view.flags != raw[FLAGS]) {
            return -1;
        }
        if (type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
            const fist_vehicle_state *actor = &world->objects[index].vehicle;
            if (actor->contact_flags != raw[RAW_CONTACT] ||
                actor->contact_cooldown != raw[RAW_COOLDOWN] ||
                actor->control_flags != fist_read_u16le(raw + COLLIDABLE)) {
                return -1;
            }
        } else if (type == TREE && world->objects[index].tree.damage != raw[RAW_TREE_DAMAGE]) {
            return -1;
        }
    }
    if (world->random.next_stream != value->header[H_CURSOR] ||
        world->preparation.trees != fist_read_u16le(value->header + H_TREES)) {
        return -1;
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        if (world->random.words[index] != fist_read_u16le(value->header + H_SEEDS + (index * 2))) {
            return -1;
        }
    }
    return 0;
}

static void allow_changes(contact_state *state, uint16_t slot,
                          const fist_ground_contact_result *result) {
    const fist_mission_world *world = &state->world;
    fist_mission_world *allowed = &state->before;
    const fist_vehicle_state *actor = &world->objects[slot].vehicle;
    fist_vehicle_state *before = &allowed->objects[slot].vehicle;
    before->control_flags = actor->control_flags;
    before->contact_flags = actor->contact_flags;
    before->contact_cooldown = actor->contact_cooldown;
    before->map_x = actor->map_x;
    before->map_y = actor->map_y;
    before->drive.speed = actor->drive.speed;
    before->drive.throttle = actor->drive.throttle;
    allowed->sound_selector = world->sound_selector;
    if (result->refresh_damage_display) {
        const uint16_t candidate = result->candidate;
        const fist_tree *tree = &world->objects[candidate].tree;
        allowed->objects[candidate].tree.damage = tree->damage;
        allowed->random = world->random;
        if (result->tree_released) {
            const uint16_t registry = tree->allocation.registry_index;
            allowed->objects[candidate].tree.flags = tree->flags;
            allowed->pool.slots[candidate] = world->pool.slots[candidate];
            allowed->pool.lifetimes[candidate] = world->pool.lifetimes[candidate];
            allowed->pool.registry[registry] = world->pool.registry[registry];
            allowed->pool.short_count = world->pool.short_count;
            allowed->preparation.trees = world->preparation.trees;
        }
    }
}

static int checked_contact(contact_state *state, fist_ground_contact_request request,
                           fist_ground_contact_result *result, int *observed) {
    uint8_t *bytes = (uint8_t *)result;
    for (size_t index = 0; index < sizeof(*result); ++index) {
        bytes[index] = OUTPUT_CANARY;
    }
    fist_ground_contact_result saved;
    fist_probe_capture(result, sizeof(*result), &saved);
    fist_probe_capture(&state->world, sizeof(state->world), &state->before);
    if (fist_mission_world_ground_contacts(NULL, request, result) != -1 ||
        fist_mission_world_ground_contacts(&state->world, request, NULL) != -1 ||
        !fist_probe_unchanged(&state->world, sizeof(state->world), &state->before) ||
        !fist_probe_unchanged(result, sizeof(*result), &saved)) {
        return -1;
    }
    const int status = fist_mission_world_ground_contacts(&state->world, request, result);
    if (status == 0) {
        allow_changes(state, request.slot, result);
    } else {
        if (status != -1 || !fist_probe_unchanged(result, sizeof(*result), &saved)) {
            return -1;
        }
        *result = (fist_ground_contact_result){.candidate = FIST_POOL_NO_SLOT};
    }
    *observed = status;
    return fist_probe_unchanged(&state->world, sizeof(state->world), &state->before) ? 0 : -1;
}

static int write_contact(const contact_state *state, uint16_t slot,
                         const fist_ground_contact_result *result, int status) {
    if (state == NULL || result == NULL || slot >= FIST_UNIT_REGISTRY_COUNT ||
        (result->refresh_damage_display && result->candidate >= FIST_UNIT_REGISTRY_COUNT)) {
        return -1;
    }
    const fist_mission_world *world = &state->world;
    const fist_vehicle_state *actor = &world->objects[slot].vehicle;
    const fist_tree *tree =
        result->refresh_damage_display ? &world->objects[result->candidate].tree : NULL;
    const uint16_t registry = tree != NULL ? tree->allocation.registry_index : FIST_POOL_NO_SLOT;
    printf("contact %d %u %u %u %u %u %u %u %lu %u %u %u %u %ld %ld %d %d %u %u %u", status,
           (unsigned)result->admitted, (unsigned)result->hit, (unsigned)result->candidate,
           (unsigned)result->tree_released, (unsigned)result->refresh_damage_display,
           (unsigned)result->sound.ax, (unsigned)result->sound.dx, (unsigned long)result->sound.ecx,
           (unsigned)result->sound.emitted, (unsigned)actor->contact_flags,
           (unsigned)actor->contact_cooldown, (unsigned)actor->control_flags, (long)actor->map_x,
           (long)actor->map_y, actor->drive.speed, actor->drive.throttle,
           (unsigned)world->sound_selector, (unsigned)world->preparation.trees,
           (unsigned)world->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)world->random.words[index]);
    }
    printf(" %u %u %u %u %u %u\n", (unsigned)(tree != NULL ? tree->damage : 0),
           (unsigned)(tree != NULL ? tree->flags : 0), (unsigned)world->pool.short_count,
           (unsigned)world->pool.extended_count,
           (unsigned)(registry != FIST_POOL_NO_SLOT ? world->pool.registry[registry].slot
                                                    : FIST_POOL_NO_SLOT),
           (unsigned)(registry != FIST_POOL_NO_SLOT ? world->pool.registry[registry].value : 0));
    return 0;
}

static int run_case(contact_state *state, const contact_case *value, bool canonical) {
    const uint8_t *header = value->header;
    if (canonical) {
        state->world = state->prepared;
        if (canonical_boundary(&state->world, value) != 0) {
            return -1;
        }
    } else if (install(&state->world, value) != 0) {
        return -1;
    }
    fist_mission_world *world = &state->world;
    const uint16_t slot = fist_read_u16le(header + H_ACTOR);
    world->combat.selected_slot = fist_read_u16le(header + H_SELECTED);
    world->combat.source_scale[0] = fist_read_u16le(header + H_FACTORS);
    world->combat.source_scale[1] = fist_read_u16le(header + H_FACTORS + sizeof(uint16_t));
    world->combat.damage_source_enemy = header[H_SIDE] != 0;
    world->random.next_stream = header[H_CURSOR];
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        world->random.words[index] = fist_read_u16le(header + H_SEEDS + (index * 2));
    }
    world->preparation.trees = fist_read_u16le(header + H_TREES);
    world->sound_selector = fist_read_u16le(header + H_SELECTOR);
    const fist_contact_audio audio = {
        {fist_read_u16le(header + H_OBSTACLE_PACKET), header[H_OBSTACLE_ATTENUATION]},
        {fist_read_u16le(header + H_TREE_PACKET), header[H_TREE_ATTENUATION]}};
    fist_ground_contact_request request = {slot, fist_read_u16le(header + H_SOURCE),
                                           header[H_WRAPPER] != 0, header[H_COARSE] != 0, &audio};
    if (header[H_INVALID] == NULL_AUDIO) {
        request.audio = NULL;
    } else if (header[H_INVALID] == BAD_RANDOM) {
        world->random.next_stream = FIST_RANDOM_STREAMS;
    } else if (header[H_INVALID] == BAD_BODY || header[H_INVALID] == BAD_TREE) {
        const uint16_t target = fist_read_u16le(header + H_INVALID_SLOT);
        if (target >= FIST_UNIT_REGISTRY_COUNT || world->pool.slots[target].used == 0) {
            return -1;
        }
        if (header[H_INVALID] == BAD_BODY) {
            world->objects[target].other.allocation.type = FIST_UNIT_TYPE_COUNT;
        } else {
            ++world->objects[target].tree.allocation.value;
        }
    } else if (header[H_INVALID] == BAD_COMPONENT) {
        world->objects[slot].vehicle.component_size = 0;
    }
    const uint16_t steps = fist_read_u16le(header + H_STEPS);
    for (uint16_t index = 0; index < steps; ++index) {
        fist_ground_contact_result result;
        int status = -1;
        if (checked_contact(state, request, &result, &status) != 0) {
            return -1;
        }
        if (write_contact(state, slot, &result, status) != 0) {
            return -1;
        }
    }
    return 0;
}

static int source_flight(contact_state *state, fist_pool_allocation source, bool enemy) {
    enum { FLIGHT_STEPS = 3, FLIGHT_SPEED = 852, TARGET_ALTITUDE = 65536, TICK = 300 };
    uint8_t pixels[4] = {0};
    const fist_klc_image height = {.width = 2, .height = 2, .pixels = pixels};
    const fist_mission_update_environment environment = {.height = &height};
    fist_fire_history history = {0};
    fist_fire_result fired = {0};
    if (fist_mission_world_fire_untargeted(&state->world, &history,
                                           (fist_fire_request){{source.slot, false}, TICK},
                                           &fired) != 0 ||
        !fired.dispatched || fired.launch.outcome != FIST_LAUNCH_FIRED) {
        return -1;
    }
    const fist_pool_allocation shell = fired.launch.projectile.allocation;
    for (unsigned step = 1; step <= FLIGHT_STEPS; ++step) {
        fist_mission_update_result result = {0};
        if (fist_mission_world_visit(&state->world, shell, &environment, &result) != 0) {
            return -1;
        }
        const fist_projectile *projectile = &state->world.objects[shell.slot].projectile;
        if (projectile->pose.x != 0 ||
            projectile->pose.y != -(int32_t)(FLIGHT_SPEED * (FLIGHT_STEPS - step)) ||
            projectile->pose.altitude != TARGET_ALTITUDE ||
            result.damaged != (step == FLIGHT_STEPS) ||
            state->world.combat.damage_source_enemy != (step == FLIGHT_STEPS ? enemy : !enemy)) {
            return -1;
        }
    }
    fist_mission_update_environment smoking = {.height = &height, .weather = {.enabled = 1}};
    const fist_pool_allocation wreck = state->world.objects[0].wreck.allocation;
    fist_mission_update_result result = {0};
    if (fist_mission_world_visit(&state->world, wreck, &smoking, &result) != 0 ||
        !result.destruction.has_smoke || result.destruction.smoke.allocation.slot != shell.slot ||
        (result.destruction.smoke.flags & SOURCE_SIDE) != 0 ||
        state->world.combat.damage_source_enemy != enemy) {
        return -1;
    }
    return 0;
}

static int source_lifetime(contact_state *state, unsigned kind, bool enemy) {
    enum {
        SOURCE_INDEX = 179,
        WRECK_INDEX = 180,
        TREE_INDEX = 6,
        ACTOR_INDEX = 181,
        SOURCE_ALTITUDE = 63488,
        WRECK_ALTITUDE = 65536,
        SOURCE_Y = -2556,
        WRECK_SCALE = 1024,
        TREE_SCALE = 256,
        TREE_POSITION = 10000,
        COUNTER = 63,
        SMALL_FACTOR = 3,
        LARGE_FACTOR = 256,
        SIDE = 8
    };
    fist_mission_world *world = &state->world;
    fist_mission_world_reset(world);
    world->random = (fist_random){.words = {1, 2, HIGH_SEED, UINT16_MAX}};
    world->combat.source_scale[0] = SMALL_FACTOR;
    world->combat.source_scale[1] = LARGE_FACTOR;
    world->combat.damage_source_enemy = !enemy;
    fist_pool_allocation source = {0};
    fist_pool_allocation wreck = {0};
    fist_pool_allocation tree = {0};
    fist_pool_allocation actor = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){0, SOURCE_INDEX, 1}, &source) !=
            0 ||
        fist_object_pool_import(&world->pool, (fist_pool_import){WRECK, WRECK_INDEX, 1}, &wreck) !=
            0 ||
        fist_object_pool_import(&world->pool, (fist_pool_import){TREE, TREE_INDEX, 1}, &tree) !=
            0 ||
        fist_object_pool_import(&world->pool, (fist_pool_import){(uint16_t)kind, ACTOR_INDEX, 1},
                                &actor) != 0) {
        return -1;
    }
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE] = {0};
    fist_unit_definition saved = {.type = 0,
                                  .registry_index = source.registry_index,
                                  .generation = source.value,
                                  .snapshot = {raw, sizeof(raw)}};
    if (fist_vehicle_initialize(&saved, &world->random, 0, &world->objects[source.slot].vehicle) !=
        0) {
        return -1;
    }
    fist_vehicle_state *shooter = &world->objects[source.slot].vehicle;
    shooter->object_flags = (uint8_t)(COLLIDABLE | (enemy ? SIDE : 0));
    shooter->map_y = SOURCE_Y;
    shooter->altitude = SOURCE_ALTITUDE;
    shooter->weapons.trigger = 1;
    raw[0] = (uint8_t)kind;
    saved.type = (uint16_t)kind;
    saved.registry_index = actor.registry_index;
    saved.generation = actor.value;
    if (fist_vehicle_initialize(&saved, &world->random, 0, &world->objects[actor.slot].vehicle) !=
        0) {
        return -1;
    }
    world->objects[wreck.slot].wreck = (fist_vehicle_wreck){.allocation = wreck,
                                                            .pose = {.altitude = WRECK_ALTITUDE},
                                                            .projection_scale = WRECK_SCALE,
                                                            .flags = COLLIDABLE,
                                                            .parameter = TREE_SCALE,
                                                            .emission_counter = COUNTER};
    world->objects[tree.slot].tree = (fist_tree){.allocation = tree,
                                                 .pose = {TREE_POSITION, TREE_POSITION, 0, 0},
                                                 .projection_scale = TREE_SCALE,
                                                 .flags = COLLIDABLE};
    world->preparation.trees = 1;
    if (source_flight(state, source, enemy) != 0) {
        return -1;
    }
    fist_vehicle_state *collider = &world->objects[actor.slot].vehicle;
    collider->map_x = TREE_POSITION;
    collider->map_y = TREE_POSITION;
    collider->projection_scale = TREE_SCALE;
    collider->drive.speed = -3;
    collider->drive.throttle = -1;
    fist_ground_contact_result result = {0};
    int status = -1;
    const fist_ground_contact_request request = {.slot = actor.slot,
                                                 .sound_source = FIST_POOL_NO_SLOT};
    if (checked_contact(state, request, &result, &status) != 0 || status != 0 ||
        result.candidate != tree.slot || !result.refresh_damage_display ||
        result.tree_released != enemy ||
        world->objects[tree.slot].tree.damage != (enemy ? TREE_DAMAGE : 1) ||
        world->combat.damage_source_enemy != enemy) {
        return -1;
    }
    return 0;
}

typedef struct {
    unsigned value;
    unsigned kind;
    unsigned variant;
    uint8_t link;
} retention_case;

static int retained_ground(retention_case input) {
    const unsigned kind = input.kind;
    const unsigned value = input.value;
    const uint8_t link = input.link;
    enum { PATTERN = 37 };
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE];
    for (size_t index = 0; index < sizeof(raw); ++index) {
        raw[index] = (uint8_t)((index * PATTERN) + value);
    }
    raw[0] = (uint8_t)kind;
    raw[1] = 0;
    raw[RAW_CONTACT] = (uint8_t)value;
    raw[RAW_COOLDOWN] = (uint8_t)(UINT8_MAX - value);
    const fist_unit_definition saved = {.type = (uint16_t)kind, .snapshot = {raw, sizeof(raw)}};
    fist_vehicle_state actor = {0};
    fist_random random = {.words = {1, 2, HIGH_SEED, UINT16_MAX}};
    if (fist_vehicle_restore(&saved, &actor) != 0 || actor.contact_flags != value ||
        actor.contact_cooldown != UINT8_MAX - value ||
        actor.drive.movement_gate != fist_read_u16le(raw + RAW_MOVEMENT) ||
        actor.behavior != raw[RAW_BEHAVIOR] ||
        fist_vehicle_initialize(&saved, &random, (uint8_t)link, &actor) != 0 ||
        actor.contact_flags != value || actor.contact_cooldown != UINT8_MAX - value ||
        fist_vehicle_prepare(&actor, (uint8_t)link) != 0 || actor.contact_flags != value ||
        actor.contact_cooldown != UINT8_MAX - value) {
        return -1;
    }
    return 0;
}

static int retained_tree(contact_state *state, retention_case input) {
    const unsigned variant = input.variant;
    const unsigned value = input.value;
    const uint8_t link = input.link;
    fist_mission_world_reset(&state->world);
    fist_pool_allocation allocation = {0};
    if (fist_object_pool_import(&state->world.pool, (fist_pool_import){TREE, 0, 1}, &allocation) !=
        0) {
        return -1;
    }
    uint8_t raw[FIST_UNIT_SHORT_SIZE] = {0};
    raw[0] = TREE;
    raw[FLAGS] = COLLIDABLE;
    raw[VARIANT] = (uint8_t)variant;
    raw[RAW_TREE_DAMAGE] = (uint8_t)value;
    const fist_unit_definition saved = definition(raw, allocation);
    uint8_t pixels[4] = {0, HEIGHT_LOW, HEIGHT_MID, UINT8_MAX};
    const fist_klc_image height = {.width = 2, .height = 2, .pixels = pixels};
    state->world.random = (fist_random){.words = {1, 2, HIGH_SEED, UINT16_MAX}};
    fist_tree *tree = &state->world.objects[allocation.slot].tree;
    if (fist_tree_restore(&saved, allocation, tree) != 0 || tree->damage != value ||
        fist_mission_world_prepare(&state->world, &height, (uint8_t)link) != 0 ||
        tree->damage != value ||
        fist_tree_advance(&state->world.pool, tree, (fist_tree_update){1, UINT8_MAX}) != 0 ||
        tree->damage != value) {
        return -1;
    }
    return 0;
}

static int retained_fields(contact_state *state) {
    enum { SECOND_LINK = 2, TREE_VARIANTS = 4 };
    unsigned ground_returns = 0;
    unsigned tree_returns = 0;
    for (unsigned link = 0; link <= SECOND_LINK; link += SECOND_LINK) {
        for (unsigned kind = 0; kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
            for (unsigned value = 0; value < BYTE_VALUES; ++value) {
                if (retained_ground((retention_case){
                        .kind = kind, .link = (uint8_t)link, .value = value}) != 0) {
                    return -1;
                }
                ground_returns += 3;
            }
        }
        for (unsigned variant = 0; variant < TREE_VARIANTS; ++variant) {
            for (unsigned value = 0; value < BYTE_VALUES; ++value) {
                if (retained_tree(state, (retention_case){.variant = variant,
                                                          .link = (uint8_t)link,
                                                          .value = value}) != 0) {
                    return -1;
                }
                tree_returns += 3;
            }
        }
    }
    printf("retention %u %u\n", ground_returns, tree_returns);
    return 0;
}

static int run_batch(int argc, char **argv, contact_state *state) {
    const bool canonical = argc == 4;
    if (argc != 2 && !canonical) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_path(argv[argc - 1], &size);
    int status = data == NULL || size < BATCH_HEADER ? -1 : 0;
    uint32_t count = 0;
    size_t position = BATCH_HEADER;
    fist_klc_image height = {0};
    if (status == 0) {
        const uint32_t side = fist_read_u32le(data);
        count = fist_read_u32le(data + sizeof(uint32_t));
        const size_t pixels = (size_t)side * side;
        if (side == 0 || side > MAX_DETAIL || (side & (side - 1)) != 0 ||
            pixels > size - BATCH_HEADER || count == 0) {
            status = -1;
        } else {
            height = (fist_klc_image){.width = side, .height = side, .pixels = data + BATCH_HEADER};
            position += pixels;
        }
    }
    if (status == 0 && canonical) {
        status = load_prepared(argv[2], &height, &state->prepared);
    }
    for (uint32_t index = 0; index < count && status == 0; ++index) {
        contact_case value = {0};
        status = decode_case(data, size, &position, &value);
        if (status == 0) {
            status = run_case(state, &value, canonical);
        }
    }
    if (position != size) {
        status = -1;
    }
    free(data);
    return status;
}

static int source_lifetime_cases(contact_state *state) {
    for (unsigned kind = 0; kind < FIST_UNIT_GROUND_VEHICLE_COUNT; ++kind) {
        for (unsigned side = 0; side < 2; ++side) {
            if (source_lifetime(state, kind, side != 0) != 0) {
                return -1;
            }
        }
    }
    puts("source-lifetime 8 24 8 8 8");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2 && (argc != 4 || strcmp(argv[1], "--prepared") != 0)) {
        return EXIT_FAILURE;
    }
    contact_state *state = calloc(1, sizeof(*state));
    if (state == NULL) {
        return EXIT_FAILURE;
    }
    int status = 0;
    if (argc == 2 && strcmp(argv[1], "--retention") == 0) {
        status = retained_fields(state);
    } else if (argc == 2 && strcmp(argv[1], "--source-lifetime") == 0) {
        status = source_lifetime_cases(state);
    } else {
        status = run_batch(argc, argv, state);
    }
    free(state);
    return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
