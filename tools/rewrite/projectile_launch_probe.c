#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/driver.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/primary_fire.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "vehicle_probe_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_BYTES = 4,
    CASE_HEADER = 15,
    IMPORT_BYTES = 6,
    ORIGIN_ORDINAL = 2,
    STEPS = 4,
    COARSE = 6,
    FLAGS = 7,
    AMMO = 8,
    RELOAD = 10,
    RECOIL = 11,
    TRIGGER = 12,
    RELEASE_COUNT = 13,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    TARGET = 151,
    MARKER_INDEX = 9,
    MARKER_VALUE = 12
};

typedef struct {
    fist_object_pool pool;
    fist_vehicle_state vehicle;
    fist_launch_request request;
    uint16_t steps;
    fist_fire_history history;
    uint16_t tick;
} launch_case;

static int same_bytes(const void *left, size_t size, const void *right) {
    const unsigned char *first = left;
    const unsigned char *second = right;
    for (size_t index = 0; index < size; ++index) {
        if (first[index] != second[index]) {
            return 0;
        }
    }
    return 1;
}

static void capture(const void *object, uint8_t *out, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        out[index] = bytes[index];
    }
}

static uint8_t *capture_owned(const void *object, size_t size) {
    uint8_t *bytes = malloc(size);
    if (bytes != NULL) {
        capture(object, bytes, size);
    }
    return bytes;
}

static int rejected_fire(fist_mission_world *world, uint16_t slot, fist_fire_request request) {
    enum { MARKER = 123 };
    uint8_t *before = capture_owned(world, sizeof(*world));
    if (before == NULL) {
        return 0;
    }
    fist_fire_history history = {MARKER};
    fist_fire_result result = {.voice_request = MARKER};
    uint8_t output[sizeof(result)];
    capture(&result, output, sizeof(output));
    const int rejected =
        fist_mission_world_fire_untargeted(world, &history, request, &result) == -1 &&
        fist_m1_fire_untargeted(&world->pool, &world->objects[slot].vehicle, &history, request,
                                &result) == -1 &&
        history.failed_at == MARKER && same_bytes(world, sizeof(*world), before) &&
        same_bytes(&result, sizeof(result), output);
    free(before);
    return rejected;
}

static int invalid_fire_state(fist_mission_world *world) {
    fist_pool_allocation allocation = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){0, MARKER_INDEX, MARKER_VALUE},
                                &allocation) != 0) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[allocation.slot].vehicle;
    actor->component_size = fist_vehicle_component_size(0);
    actor->weapons.trigger = 1;
    fist_fire_history history = {0};
    fist_fire_result result = {0};
    const fist_fire_request request = {{allocation.slot, false}, 0};
    uint8_t *before = capture_owned(world, sizeof(*world));
    if (before == NULL) {
        return -1;
    }
    uint8_t output[sizeof(result)];
    capture(&result, output, sizeof(output));
    const int invalid =
        fist_mission_world_fire_untargeted(NULL, &history, request, &result) != -1 ||
        fist_mission_world_fire_untargeted(world, NULL, request, &result) != -1 ||
        fist_mission_world_fire_untargeted(world, &history, request, NULL) != -1 ||
        fist_m1_fire_untargeted(NULL, actor, &history, request, &result) != -1 ||
        fist_m1_fire_untargeted(&world->pool, NULL, &history, request, &result) != -1 ||
        fist_m1_fire_untargeted(&world->pool, actor, NULL, request, &result) != -1 ||
        fist_m1_fire_untargeted(&world->pool, actor, &history, request, NULL) != -1 ||
        !same_bytes(world, sizeof(*world), before) || history.failed_at != 0 ||
        !same_bytes(&result, sizeof(result), output);
    free(before);
    if (invalid) {
        return -1;
    }
    for (unsigned slot = 0; slot <= FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (slot != allocation.slot &&
            !rejected_fire(world, allocation.slot,
                           (fist_fire_request){{(uint16_t)slot, false}, 0})) {
            return -1;
        }
    }
    for (unsigned selected = 1; selected <= UINT8_MAX; ++selected) {
        actor->weapons.selected = (uint8_t)selected;
        if (!rejected_fire(world, allocation.slot, request)) {
            return -1;
        }
    }
    actor->weapons.selected = 0;
    for (unsigned type = 1; type <= FIST_UNIT_TYPE_COUNT; ++type) {
        actor->type = (uint16_t)type;
        if (!rejected_fire(world, allocation.slot, request)) {
            return -1;
        }
    }
    actor->type = 0;
    for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
        if (size == fist_vehicle_component_size(0)) {
            continue;
        }
        actor->component_size = size;
        if (!rejected_fire(world, allocation.slot, request)) {
            return -1;
        }
    }
    actor->component_size = fist_vehicle_component_size(0);
    world->pool.short_count = 1;
    return rejected_fire(world, allocation.slot, request) ? 0 : -1;
}

static int invalid_fire(void) {
    fist_mission_world *world = calloc(1, sizeof(*world));
    if (world == NULL) {
        return -1;
    }
    fist_mission_world_reset(world);
    const int status = invalid_fire_state(world);
    free(world);
    return status;
}

static int rejected(fist_object_pool *pool, fist_vehicle_state *vehicle,
                    fist_launch_request request) {
    const fist_object_pool saved_pool = *pool;
    const fist_vehicle_state saved_vehicle = *vehicle;
    const fist_launch_result marker = {.outcome = UINT8_MAX, .sound_request = UINT8_MAX};
    fist_launch_result output = marker;
    return fist_m1_launch_untargeted(pool, vehicle, request, &output) == -1 &&
           same_bytes(pool, sizeof(*pool), &saved_pool) &&
           same_bytes(vehicle, sizeof(*vehicle), &saved_vehicle) &&
           same_bytes(&output, sizeof(output), &marker);
}

static int invalid_inputs(void) {
    fist_object_pool pool = {0};
    fist_object_pool_reset(&pool);
    fist_pool_allocation actor = {0};
    if (fist_object_pool_import(&pool, (fist_pool_import){0, MARKER_INDEX, MARKER_VALUE}, &actor) !=
        0) {
        return -1;
    }
    fist_vehicle_state vehicle = {.component_size = fist_vehicle_component_size(0)};
    fist_launch_result output = {0};
    const fist_launch_request request = {actor.slot, false};
    if (fist_m1_launch_untargeted(NULL, &vehicle, request, &output) != -1 ||
        fist_m1_launch_untargeted(&pool, NULL, request, &output) != -1 ||
        fist_m1_launch_untargeted(&pool, &vehicle, request, NULL) != -1) {
        return -1;
    }
    for (unsigned slot = 0; slot <= FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (slot != actor.slot &&
            !rejected(&pool, &vehicle, (fist_launch_request){(uint16_t)slot, false})) {
            return -1;
        }
    }
    for (unsigned type = 1; type <= FIST_UNIT_TYPE_COUNT; ++type) {
        vehicle.type = (uint16_t)type;
        if (!rejected(&pool, &vehicle, request)) {
            return -1;
        }
    }
    vehicle.type = 0;
    for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
        if (size == fist_vehicle_component_size(0)) {
            continue;
        }
        vehicle.component_size = size;
        if (!rejected(&pool, &vehicle, request)) {
            return -1;
        }
    }
    vehicle.component_size = fist_vehicle_component_size(0);
    pool.short_count = 1;
    if (!rejected(&pool, &vehicle, request)) {
        return -1;
    }
    pool.short_count = 0;
    pool.registry[0] = pool.registry[MARKER_INDEX];
    return rejected(&pool, &vehicle, request) ? 0 : -1;
}

static int prepare(const uint8_t *data, size_t available, bool fire, launch_case *out,
                   size_t *consumed) {
    if (available < CASE_HEADER + FIST_UNIT_EXTENDED_SIZE) {
        return -1;
    }
    const size_t count = fist_read_u16le(data);
    const size_t ordinal = fist_read_u16le(data + ORIGIN_ORDINAL);
    const size_t releases = fist_read_u16le(data + RELEASE_COUNT);
    const size_t size = CASE_HEADER + FIST_UNIT_EXTENDED_SIZE + (count * IMPORT_BYTES) +
                        (releases * sizeof(uint16_t));
    const uint8_t *raw = data + CASE_HEADER;
    if (count > FIST_UNIT_REGISTRY_COUNT || ordinal >= count || size > available ||
        data[COARSE] > 1 || releases > FIST_UNIT_REGISTRY_COUNT || fist_read_u16le(raw) != 0 ||
        fist_read_u16le(raw + TARGET) != 0 || fist_read_u16le(data + STEPS) == 0) {
        return -1;
    }
    fist_object_pool_reset(&out->pool);
    fist_pool_allocation origin = {0};
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *binding = raw + FIST_UNIT_EXTENDED_SIZE + (index * IMPORT_BYTES);
        fist_pool_allocation allocation = {0};
        const fist_pool_import request = {fist_read_u16le(binding), fist_read_u16le(binding + 2),
                                          fist_read_u16le(binding + 4)};
        if (fist_object_pool_import(&out->pool, request, &allocation) != 0) {
            return -1;
        }
        if (index == ordinal) {
            origin = allocation;
        }
    }
    for (size_t index = 0; index < releases; ++index) {
        const uint8_t *entry =
            raw + FIST_UNIT_EXTENDED_SIZE + (count * IMPORT_BYTES) + (index * sizeof(uint16_t));
        fist_pool_allocation allocation = {0};
        if (fist_object_pool_release(&out->pool, fist_read_u16le(entry), &allocation) != 0 ||
            allocation.slot == origin.slot) {
            return -1;
        }
    }
    if (origin.type != 0) {
        return -1;
    }
    const fist_unit_definition definition = {.type = 0,
                                             .registry_index = origin.registry_index,
                                             .generation = origin.value,
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    fist_random random = {0};
    if (fist_vehicle_initialize(&definition, &random, 0, &out->vehicle) != 0) {
        return -1;
    }
    out->vehicle.weapons.rounds[0] = fist_read_u16le(data + AMMO);
    out->vehicle.reload_countdown = data[RELOAD];
    out->vehicle.weapons.recoil = data[RECOIL];
    out->vehicle.weapons.trigger = data[TRIGGER];
    out->vehicle.object_flags = data[FLAGS];
    if (fire) {
        enum { SECONDARY = 23, BEHAVIOR = 62, BEHAVIOR_FLAGS = 99, SELECTED = 145 };
        if (raw[SELECTED] != 0) {
            return -1;
        }
        out->vehicle.secondary_flags = raw[SECONDARY];
        out->vehicle.behavior = raw[BEHAVIOR];
        out->vehicle.behavior_flags = raw[BEHAVIOR_FLAGS];
    }
    out->request = (fist_launch_request){origin.slot, data[COARSE] != 0};
    out->steps = fist_read_u16le(data + STEPS);
    *consumed = size;
    return 0;
}

static void write_payloads(const fist_launch_result *result) {
    printf("launch %u %u %u\n", (unsigned)result->outcome, (unsigned)result->has_muzzle,
           (unsigned)result->sound_request);
    if (result->outcome == FIST_LAUNCH_FIRED) {
        const fist_projectile *shot = &result->projectile;
        printf("projectile %u %u %u %u %d %d %d %u %d %d %d %d %u %u %u %d %u %u %u\n",
               (unsigned)shot->allocation.type, (unsigned)shot->allocation.slot,
               (unsigned)shot->allocation.registry_index, (unsigned)shot->allocation.value,
               shot->pose.x, shot->pose.y, shot->pose.altitude, (unsigned)shot->pose.heading,
               shot->velocity.x, shot->velocity.y, shot->velocity.z, shot->speed,
               (unsigned)shot->collision_grace, (unsigned)shot->origin_slot,
               (unsigned)shot->target_slot, shot->target_height_offset, (unsigned)shot->flags,
               (unsigned)shot->collision_profile, (unsigned)shot->launch_parameter);
    }
    if (result->has_muzzle) {
        const fist_muzzle_smoke *smoke = &result->muzzle;
        printf("muzzle %u %u %u %u %d %d %d %u %u %u %u %u\n", (unsigned)smoke->allocation.type,
               (unsigned)smoke->allocation.slot, (unsigned)smoke->allocation.registry_index,
               (unsigned)smoke->allocation.value, smoke->pose.x, smoke->pose.y,
               smoke->pose.altitude, (unsigned)smoke->pose.heading,
               (unsigned)smoke->projection_scale, (unsigned)smoke->animation_counter,
               (unsigned)smoke->animation_frame, (unsigned)smoke->flags);
    }
}

static int unrelated_preserved(const fist_mission_world *world, const uint8_t *before,
                               fist_fire_request request, const fist_fire_result *result) {
    if (!same_bytes(&world->random, sizeof(world->random),
                    before + offsetof(fist_mission_world, random)) ||
        !same_bytes(world->roster, sizeof(world->roster),
                    before + offsetof(fist_mission_world, roster))) {
        return 0;
    }
    const bool fired = result->dispatched && result->launch.outcome == FIST_LAUNCH_FIRED;
    if (!fired && !same_bytes(&world->pool, sizeof(world->pool),
                              before + offsetof(fist_mission_world, pool))) {
        return 0;
    }
    for (unsigned slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (slot == request.launch.origin_slot ||
            (fired &&
             (slot == result->launch.projectile.allocation.slot ||
              (result->launch.has_muzzle && slot == result->launch.muzzle.allocation.slot)))) {
            continue;
        }
        const size_t offset =
            offsetof(fist_mission_world, objects) + (slot * sizeof(fist_mission_object));
        if (!same_bytes(&world->objects[slot], sizeof(fist_mission_object), before + offset)) {
            return 0;
        }
    }
    return 1;
}

static int write_fire_payloads(const fist_mission_world *world, fist_launch_result *launch) {
    if (launch->outcome == FIST_LAUNCH_FIRED) {
        const fist_mission_object *object =
            fist_mission_world_object(world, launch->projectile.allocation.slot);
        if (object == NULL || object->projectile.age != 0 || object->projectile.mode != 0 ||
            object->projectile.phase != 0 || object->projectile.ground_height != 0 ||
            object->projectile.secondary_flags != 0) {
            return -1;
        }
        launch->projectile = object->projectile;
        if (launch->has_muzzle) {
            object = fist_mission_world_object(world, launch->muzzle.allocation.slot);
            if (object == NULL) {
                return -1;
            }
            launch->muzzle = object->muzzle;
        }
    }
    write_payloads(launch);
    return 0;
}

static int fire_step(fist_mission_world *world, fist_fire_history *history,
                     fist_fire_request request) {
    uint8_t *before = capture_owned(world, sizeof(*world));
    if (before == NULL) {
        return -1;
    }
    fist_fire_result result = {0};
    const int status = fist_mission_world_fire_untargeted(world, history, request, &result);
    const int preserved = unrelated_preserved(world, before, request, &result);
    free(before);
    if (status != 0 || !preserved) {
        return -1;
    }
    printf("fire %u %u %u %u %u\n", (unsigned)result.requested, (unsigned)result.dispatched,
           (unsigned)result.weapon_panel_refresh, (unsigned)result.voice_request,
           (unsigned)history->failed_at);
    if (write_fire_payloads(world, &result.launch) != 0) {
        return -1;
    }
    const fist_vehicle_state *actor = &world->objects[request.launch.origin_slot].vehicle;
    fist_probe_write_vehicle_state(actor);
    printf("damage %u %u\n", (unsigned)actor->damage, (unsigned)actor->damage_alarm_countdown);
    fist_probe_write_object_pool(&world->pool);
    return 0;
}

static int fire_case(launch_case *value) {
    fist_mission_world *world = calloc(1, sizeof(*world));
    if (world == NULL) {
        return -1;
    }
    fist_mission_world_reset(world);
    world->pool = value->pool;
    world->objects[value->request.origin_slot].vehicle = value->vehicle;
    int status = 0;
    for (unsigned step = 0; step < value->steps; ++step) {
        const fist_fire_request request = {value->request, value->tick};
        status = fire_step(world, &value->history, request);
        if (status != 0) {
            break;
        }
        ++value->tick;
    }
    free(world);
    return status;
}

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

static int mission_fire_case(const char *path) {
    size_t size = 0;
    uint8_t *data = read_path(path, &size);
    fist_scenario scenario = {0};
    fist_units units = {0};
    fist_mission_world *world = calloc(1, sizeof(*world));
    int status = data == NULL || world == NULL ? -1 : fist_scenario_decode(data, size, &scenario);
    if (status == 0) {
        status = fist_units_decode(&scenario, &units);
    }
    enum { SEED_FIRST = 1, SEED_SECOND = 2, SEED_THIRD = 32768, SEED_LAST = 65535 };
    const fist_random random = {.words = {SEED_FIRST, SEED_SECOND, SEED_THIRD, SEED_LAST}};
    if (status == 0) {
        status = fist_mission_world_initialize(&units, &random, 0, world);
    }
    fist_units_destroy(&units);
    free(data);
    if (status != 0 || world->roster[0] == FIST_POOL_NO_SLOT) {
        free(world);
        return -1;
    }
    const uint16_t slot = world->roster[0];
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    fist_weapon_events events = {0};
    if (world->pool.slots[slot].type != 0 || fist_weapon_select(actor, 0, &events) != 0 ||
        fist_driver_take_control(actor) != 0) {
        free(world);
        return -1;
    }
    enum { RELOAD_STAGES = 320, PHASE_STEP = 2 };
    /* Declared selection/reload boundaries, not complete class/world ticks. */
    for (unsigned phase = 0; phase < RELOAD_STAGES; ++phase) {
        actor->drive.update_phase = (uint8_t)(actor->drive.update_phase + PHASE_STEP);
        if (fist_weapon_reload_phase(actor, &events) != 0) {
            free(world);
            return -1;
        }
    }
    if (fist_weapon_request_fire(actor) != 0) {
        free(world);
        return -1;
    }
    fist_fire_history history = {0};
    status = fire_step(world, &history, (fist_fire_request){{slot, false}, RELOAD_STAGES});
    free(world);
    return status;
}

static int run_cases(uint8_t *data, size_t size, bool fire) {
    enum { FIRE_HEADER = 8, FIRE_TICK = 4, FAILED_AT = 6 };
    const size_t header = fire ? FIRE_HEADER : HEADER_BYTES;
    const size_t count = fist_read_u32le(data);
    if (count > (size - header) / (CASE_HEADER + FIST_UNIT_EXTENDED_SIZE)) {
        free(data);
        return -1;
    }
    launch_case *cases = calloc(count + 1, sizeof(*cases));
    if (cases == NULL) {
        free(data);
        return -1;
    }
    size_t offset = header;
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        size_t consumed = 0;
        if (prepare(data + offset, size - offset, fire, &cases[index], &consumed) != 0) {
            status = -1;
            break;
        }
        offset += consumed;
        if (fire) {
            cases[index].tick = fist_read_u16le(data + FIRE_TICK);
            cases[index].history.failed_at = fist_read_u16le(data + FAILED_AT);
        }
    }
    if (offset != size) {
        status = -1;
    }
    free(data); /* No borrowed source/request bytes survive simulation. */
    for (size_t index = 0; index < count && status == 0; ++index) {
        launch_case *value = &cases[index];
        if (fire) {
            status = fire_case(value);
            continue;
        }
        for (unsigned step = 0; step < value->steps; ++step) {
            fist_launch_result result = {0};
            status =
                fist_m1_launch_untargeted(&value->pool, &value->vehicle, value->request, &result);
            if (status != 0) {
                break;
            }
            write_payloads(&result);
            fist_probe_write_vehicle_state(&value->vehicle);
            fist_probe_write_object_pool(&value->pool);
        }
    }
    free(cases);
    return status;
}

int main(int argc, char **argv) {
    const bool fire = argc == 3 && strcmp(argv[1], "fire") == 0;
    const bool mission = argc == 3 && strcmp(argv[1], "mission-fire") == 0;
    if ((argc != 2 && !fire && !mission) || invalid_inputs() != 0 ||
        ((fire || mission) && invalid_fire() != 0)) {
        return EXIT_FAILURE;
    }
    if (mission) {
        return mission_fire_case(argv[2]) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_path(argv[fire ? 2 : 1], &size);
    const size_t header = fire ? 8U : HEADER_BYTES;
    if (data == NULL || size < header) {
        free(data);
        return EXIT_FAILURE;
    }
    return run_cases(data, size, fire) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
