#include "assets/bytes.h"
#include "assets/units.h"
#include "combat_probe_io.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/destruction_updates.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"
#include "sim/world_step.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 37,
    COMMAND_SIZE = 8,
    TICK = 0,
    AUXILIARY_FIRST = 2,
    AUXILIARY_SECOND = 4,
    VOICE_AT = 6,
    LAST_VOICE = 8,
    TIME = 10,
    PENDING_VOICE = 13,
    VOICE_MODE = 14,
    SMOKE_SETTING = 15,
    WIND_X = 16,
    WIND_Y = 20,
    SEEDS = 24,
    STREAM = 32,
    COMMAND_COUNT = 33,
    EXPLOSION = 4,
    SMOKE = 17,
    MUZZLE = 18,
    WRECK = 23,
    TARGET = 26,
    ARTILLERY = 27,
    MAP_X = 4,
    MAP_Y = 8,
    ALTITUDE = 12,
    HEADING = 16,
    EXTENT = 18,
    SCALE = 20,
    FLAGS = 22,
    FRAME = 25,
    CALLBACK = 26,
    HEIGHT = 28,
    LAST_FRAME = 30,
    PERIOD = 31,
    COUNTDOWN = 32,
    RESET_COMMAND = 0,
    ALLOCATE_COMMAND = 1,
    IMPORT_COMMAND = 2,
    RELEASE_COMMAND = 3,
    PASS_COMMAND = 4,
    NEXT_COMMAND = 5,
    TICK_COMMAND = 6,
    RESTORE_COMMAND = 7,
    UPDATE_COMMAND = 8,
    RETYPE_COMMAND = 9,
    COMMAND_VALUE = 6,
    MARKER = 123
};

typedef struct {
    uint8_t operation;
    uint8_t low;
    uint16_t type;
    uint16_t index;
    uint16_t value;
    uint8_t raw[FIST_UNIT_SHORT_SIZE];
} command;

typedef struct {
    fist_world_clock clock;
    fist_random random;
    fist_smoke_weather weather;
    size_t count;
    command *commands;
} program;

typedef union {
    fist_vehicle_wreck wreck;
    fist_other_actor actor;
    fist_drifting_smoke smoke;
    fist_muzzle_smoke muzzle;
    fist_explosion explosion;
} payload;

typedef struct {
    fist_object_pool pool;
    fist_world_pass pass;
    fist_pool_allocation visit;
    fist_world_clock clock;
    fist_random random;
    fist_smoke_weather weather;
    payload objects[FIST_POOL_SHORT_SLOTS];
    bool present[FIST_POOL_SHORT_SLOTS];
} runtime;

static void capture(const void *object, uint8_t *out, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        out[index] = bytes[index];
    }
}

static int unchanged(const void *object, const uint8_t *before, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        if (bytes[index] != before[index]) {
            return 0;
        }
    }
    return 1;
}

static int invalid(runtime *world) {
    uint8_t before[sizeof(*world)];
    capture(world, before, sizeof(before));
    fist_world_tick tick = {.voice_due = true, .voice_command = MARKER};
    uint8_t marker[sizeof(tick)];
    capture(&tick, marker, sizeof(marker));
    if (fist_world_begin_tick(NULL, 0, &tick) != -1 ||
        fist_world_begin_tick(&world->clock, 0, NULL) != -1 ||
        fist_world_next(NULL, &world->pass, &world->visit) != -1 ||
        fist_world_next(&world->pool, NULL, &world->visit) != -1 ||
        fist_world_next(&world->pool, &world->pass, NULL) != -1 ||
        !unchanged(world, before, sizeof(before)) || !unchanged(&tick, marker, sizeof(marker))) {
        return -1;
    }
    const uint16_t saved = world->pass.next_entry;
    world->pass.next_entry = FIST_UNIT_REGISTRY_COUNT + 1;
    capture(world, before, sizeof(before));
    const int valid = fist_world_next(&world->pool, &world->pass, &world->visit) == -1 &&
                      unchanged(world, before, sizeof(before));
    world->pass.next_entry = saved;
    if (!valid) {
        return -1;
    }
    const uint16_t count = world->pool.short_count;
    world->pool.short_count = (uint16_t)(count + 1);
    capture(world, before, sizeof(before));
    const int valid_pool = fist_world_next(&world->pool, &world->pass, &world->visit) == -1 &&
                           unchanged(world, before, sizeof(before));
    world->pool.short_count = count;
    return valid_pool ? 0 : -1;
}

static void write_state(const runtime *world) {
    const fist_world_clock *clock = &world->clock;
    printf("clock %u %u %u %u %u %u %u %u %u %u\nrandom %u", (unsigned)clock->tick,
           (unsigned)clock->auxiliary_countdown[0], (unsigned)clock->auxiliary_countdown[1],
           (unsigned)clock->voice_at, (unsigned)clock->last_voice_timer,
           (unsigned)clock->mission_countdown[0], (unsigned)clock->mission_countdown[1],
           (unsigned)clock->mission_countdown[2], (unsigned)clock->pending_voice,
           (unsigned)clock->voice_mode, (unsigned)world->random.next_stream);
    for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
        printf(" %u", (unsigned)world->random.words[stream]);
    }
    printf("\n");
    fist_probe_write_object_pool(&world->pool);
}

static int restore(runtime *world, const command *input) {
    if (input->index >= FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    const uint16_t slot = world->pool.registry[input->index].slot;
    fist_pool_allocation allocation = {0};
    if (slot >= FIST_POOL_SHORT_SLOTS ||
        fist_object_pool_find(&world->pool, slot, &allocation) != FIST_POOL_OK ||
        fist_read_u16le(input->raw) != allocation.type) {
        return -1;
    }
    const uint8_t *raw = input->raw;
    const fist_object_pose pose = {fist_read_i32le(raw + MAP_X), fist_read_i32le(raw + MAP_Y),
                                   fist_read_i32le(raw + ALTITUDE), fist_read_u16le(raw + HEADING)};
    const fist_unit_definition definition = {.type = allocation.type,
                                             .registry_index = allocation.registry_index,
                                             .generation = allocation.value,
                                             .map_x = pose.x,
                                             .map_y = pose.y,
                                             .altitude = pose.altitude,
                                             .heading = pose.heading,
                                             .snapshot = {raw, FIST_UNIT_SHORT_SIZE}};
    payload value = {0};
    int status = 0;
    switch (allocation.type) {
    case WRECK:
        status = fist_vehicle_wreck_restore(&definition, allocation, &value.wreck);
        break;
    case TARGET:
    case ARTILLERY:
        status = fist_other_actor_restore(&definition, allocation, &value.actor);
        if (allocation.type == TARGET && value.actor.mode < 4) {
            return -1;
        }
        break;
    case SMOKE:
        status = fist_drifting_smoke_restore(&definition, allocation, &value.smoke);
        break;
    case EXPLOSION:
        value.explosion = (fist_explosion){.allocation = allocation,
                                           .pose = {pose.x, pose.y, pose.altitude, 0},
                                           .model_code = pose.heading,
                                           .extent = fist_read_u16le(raw + EXTENT),
                                           .projection_scale = fist_read_u16le(raw + SCALE),
                                           .callback_selector = fist_read_u16le(raw + CALLBACK),
                                           .height_offset = fist_read_u16le(raw + HEIGHT),
                                           .frame = raw[FRAME],
                                           .last_frame = raw[LAST_FRAME],
                                           .period = raw[PERIOD],
                                           .countdown = raw[COUNTDOWN],
                                           .flags = raw[FLAGS]};
        break;
    case MUZZLE:
        value.muzzle = (fist_muzzle_smoke){.allocation = allocation,
                                           .pose = pose,
                                           .projection_scale = fist_read_u16le(raw + SCALE),
                                           .animation_counter = fist_read_u16le(raw + CALLBACK),
                                           .animation_frame = raw[FRAME],
                                           .flags = raw[FLAGS]};
        break;
    default:
        return -1;
    }
    if (status == 0) {
        world->objects[slot] = value;
        world->present[slot] = true;
    }
    return status;
}

static int update(runtime *world) {
    const fist_pool_allocation allocation = world->visit;
    if (allocation.slot >= FIST_POOL_SHORT_SLOTS || !world->present[allocation.slot] ||
        !fist_object_pool_is_current(&world->pool, allocation)) {
        return -1;
    }
    payload *object = &world->objects[allocation.slot];
    const fist_destruction_environment environment = {&world->pool, &world->random,
                                                      world->weather.enabled};
    fist_destruction_step result = {0};
    int status = 0;
    switch (allocation.type) {
    case WRECK:
        status = fist_vehicle_wreck_advance(&object->wreck, &environment, &result);
        if (status == 0) {
            fist_probe_write_wreck(&object->wreck);
        }
        break;
    case TARGET:
    case ARTILLERY:
        status = fist_destroyed_target_advance(&object->actor, &environment, &result);
        if (status == 0) {
            fist_probe_write_other_actor(&object->actor);
        }
        break;
    case SMOKE:
        status = fist_drifting_smoke_advance(&world->pool, &object->smoke, world->weather);
        if (status == 0) {
            fist_probe_write_smoke(&object->smoke);
        }
        break;
    case EXPLOSION:
        status = fist_explosion_advance(&world->pool, &object->explosion);
        if (status == 0) {
            fist_probe_write_explosion(&object->explosion);
        }
        break;
    case MUZZLE:
        status = fist_muzzle_smoke_advance(&world->pool, &object->muzzle);
        if (status == 0) {
            const fist_muzzle_smoke *muzzle = &object->muzzle;
            printf("muzzle %u %u %u %ld %ld %ld %u %u %u %u %u\n", (unsigned)allocation.slot,
                   (unsigned)allocation.registry_index, (unsigned)allocation.value,
                   (long)muzzle->pose.x, (long)muzzle->pose.y, (long)muzzle->pose.altitude,
                   (unsigned)muzzle->pose.heading, (unsigned)muzzle->projection_scale,
                   (unsigned)muzzle->animation_counter, (unsigned)muzzle->animation_frame,
                   (unsigned)muzzle->flags);
        }
        break;
    default:
        return -1;
    }
    if (status == 0 && result.has_smoke) {
        const size_t slot = result.smoke.allocation.slot;
        world->objects[slot].smoke = result.smoke;
        world->present[slot] = true;
        fist_probe_write_smoke(&result.smoke);
    }
    return status;
}

static int execute_program(const program *input) {
    runtime *world = calloc(1, sizeof(*world));
    if (world == NULL) {
        return -1;
    }
    world->clock = input->clock;
    world->random = input->random;
    world->weather = input->weather;
    world->visit = (fist_pool_allocation){FIST_UNIT_TYPE_COUNT, FIST_POOL_NO_SLOT,
                                          FIST_UNIT_REGISTRY_COUNT, UINT16_MAX};
    fist_object_pool_reset(&world->pool);
    write_state(world);
    for (size_t index = 0; index < input->count; ++index) {
        const command *request = &input->commands[index];
        fist_pool_allocation allocation = {0};
        fist_world_tick tick = {.voice_command = FIST_WORLD_NO_COMMAND};
        int status = 0;
        if (invalid(world) != 0) {
            free(world);
            return -1;
        }
        switch (request->operation) {
        case RESET_COMMAND:
            fist_object_pool_reset(&world->pool);
            for (size_t slot = 0; slot < FIST_POOL_SHORT_SLOTS; ++slot) {
                world->present[slot] = false;
            }
            break;
        case ALLOCATE_COMMAND:
            status = fist_object_pool_allocate(
                &world->pool, (fist_pool_request){request->type, request->low}, &allocation);
            break;
        case IMPORT_COMMAND:
            status = fist_object_pool_import(
                &world->pool, (fist_pool_import){request->type, request->index, request->value},
                &allocation);
            break;
        case RELEASE_COMMAND:
            status = fist_object_pool_release(&world->pool, request->index, &allocation);
            break;
        case PASS_COMMAND:
            world->pass = (fist_world_pass){0};
            break;
        case NEXT_COMMAND:
            status = fist_world_next(&world->pool, &world->pass, &world->visit);
            break;
        case TICK_COMMAND:
            status = fist_world_begin_tick(&world->clock, request->index, &tick);
            break;
        case RESTORE_COMMAND:
            status = restore(world, request);
            break;
        case UPDATE_COMMAND:
            status = update(world);
            break;
        case RETYPE_COMMAND:
            status = fist_object_pool_find(&world->pool,
                                           request->index < FIST_UNIT_REGISTRY_COUNT
                                               ? world->pool.registry[request->index].slot
                                               : FIST_POOL_NO_SLOT,
                                           &allocation);
            if (status == FIST_POOL_OK) {
                status =
                    fist_object_pool_retype(&world->pool, allocation, request->type, &allocation);
            }
            break;
        default:
            status = -1;
            break;
        }
        if (status == FIST_POOL_OK &&
            (request->operation == ALLOCATE_COMMAND || request->operation == IMPORT_COMMAND ||
             request->operation == RETYPE_COMMAND) &&
            allocation.slot < FIST_POOL_SHORT_SLOTS) {
            world->present[allocation.slot] = false;
        }
        printf("result %u %d %u %u %u %u %u %u %u\n", (unsigned)request->operation, status,
               (unsigned)world->visit.type, (unsigned)world->visit.slot,
               (unsigned)world->visit.registry_index, (unsigned)world->visit.value,
               (unsigned)world->pass.next_entry, (unsigned)tick.voice_due,
               (unsigned)tick.voice_command);
        write_state(world);
    }
    free(world);
    return 0;
}

static int read_program(program *out, const uint8_t *input, size_t size, size_t *used) {
    if (size < HEADER || input[STREAM] >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    const size_t count = fist_read_u32le(input + COMMAND_COUNT);
    if (count > (size - HEADER) / COMMAND_SIZE) {
        return -1;
    }
    out->clock =
        (fist_world_clock){.tick = fist_read_u16le(input + TICK),
                           .auxiliary_countdown = {fist_read_u16le(input + AUXILIARY_FIRST),
                                                   fist_read_u16le(input + AUXILIARY_SECOND)},
                           .voice_at = fist_read_u16le(input + VOICE_AT),
                           .last_voice_timer = fist_read_u16le(input + LAST_VOICE),
                           .mission_countdown = {input[TIME], input[TIME + 1], input[TIME + 2]},
                           .pending_voice = input[PENDING_VOICE],
                           .voice_mode = input[VOICE_MODE]};
    out->weather = (fist_smoke_weather){fist_read_i32le(input + WIND_X),
                                        fist_read_i32le(input + WIND_Y), input[SMOKE_SETTING]};
    out->random.next_stream = input[STREAM];
    for (size_t stream = 0; stream < FIST_RANDOM_STREAMS; ++stream) {
        out->random.words[stream] = fist_read_u16le(input + SEEDS + (stream * sizeof(uint16_t)));
    }
    out->count = count;
    out->commands = calloc(count == 0 ? 1 : count, sizeof(*out->commands));
    if (out->commands == NULL) {
        return -1;
    }
    size_t offset = HEADER;
    for (size_t index = 0; index < count; ++index) {
        if (size - offset < COMMAND_SIZE) {
            return -1;
        }
        const uint8_t *bytes = input + offset;
        command *value = &out->commands[index];
        *value = (command){.operation = bytes[0],
                           .low = bytes[1],
                           .type = fist_read_u16le(bytes + 2),
                           .index = fist_read_u16le(bytes + 4),
                           .value = fist_read_u16le(bytes + COMMAND_VALUE)};
        offset += COMMAND_SIZE;
        if (value->operation == RESTORE_COMMAND) {
            if (size - offset < FIST_UNIT_SHORT_SIZE) {
                return -1;
            }
            for (size_t byte = 0; byte < FIST_UNIT_SHORT_SIZE; ++byte) {
                value->raw[byte] = input[offset + byte];
            }
            offset += FIST_UNIT_SHORT_SIZE;
        }
    }
    *used = offset;
    return 0;
}

static int run(uint8_t *input, size_t size) {
    if (size < sizeof(uint32_t)) {
        free(input);
        return -1;
    }
    const size_t count = fist_read_u32le(input);
    if (count == 0 || count > (size - sizeof(uint32_t)) / HEADER) {
        free(input);
        return -1;
    }
    program *programs = calloc(count, sizeof(*programs));
    if (programs == NULL) {
        free(input);
        return -1;
    }
    size_t offset = sizeof(uint32_t);
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        size_t used = 0;
        if (read_program(&programs[index], input + offset, size - offset, &used) != 0) {
            status = -1;
            break;
        }
        offset += used;
    }
    free(input);
    if (offset != size) {
        status = -1;
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        status = execute_program(&programs[index]);
    }
    for (size_t index = 0; index < count; ++index) {
        free(programs[index].commands);
    }
    free(programs);
    return status;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *input = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (input == NULL || closed) {
        free(input);
        return EXIT_FAILURE;
    }
    return run(input, size) == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
