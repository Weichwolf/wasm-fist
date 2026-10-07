#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "mission_probe_io.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 16,
    WORDS = 4,
    STREAM = 12,
    LINK = 13,
    COUNT = 14,
    COMMAND = 4,
    PREPARE = 0,
    INJECT_EFFECT = 1,
    INJECT_SHELL = 2,
    INJECT_RETIRING = 3,
    EFFECT = 4,
    SHELL = 8,
    RETIRING = 19,
    DELETED = 1,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    FIRST_TEMPORARY = 11,
    SECOND_TEMPORARY = 13,
    SMOKE = 17,
    MUZZLE = 18,
    TARGET = 26,
    ARTILLERY = 27
};

static uint8_t *read_file(const char *path, size_t *size) {
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

static int contracts(fist_mission_world *world, const fist_klc_image *height,
                     fist_mission_world *before, uint8_t link) {
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_prepare(NULL, height, link) != -1 ||
        fist_mission_world_prepare(world, NULL, link) != -1 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    const uint8_t cursor = world->random.next_stream;
    world->random.next_stream = FIST_RANDOM_STREAMS;
    fist_probe_capture(world, sizeof(*world), before);
    const int unchanged = fist_mission_world_prepare(world, height, link) == -1 &&
                          fist_probe_unchanged(world, sizeof(*world), before);
    world->random.next_stream = cursor;
    if (!unchanged) {
        return -1;
    }
    return 0;
}

static int inject(fist_mission_world *world, const uint8_t *command) {
    static const uint16_t types[] = {0, EFFECT, SHELL, RETIRING};
    const uint16_t type = types[fist_read_u16le(command)];
    const uint8_t flags = (uint8_t)fist_read_u16le(command + sizeof(uint16_t));
    fist_pool_allocation allocation = {0};
    const int status =
        fist_object_pool_allocate(&world->pool, (fist_pool_request){type, 0}, &allocation);
    if (status == FIST_POOL_OK) {
        fist_mission_object *object = &world->objects[allocation.slot];
        if (type == EFFECT) {
            *object =
                (fist_mission_object){.explosion = {.allocation = allocation, .flags = flags}};
        } else if (type == SHELL) {
            *object =
                (fist_mission_object){.projectile = {.allocation = allocation, .flags = flags}};
        } else {
            *object = (fist_mission_object){.vehicle = {.type = type,
                                                        .registry_index = allocation.registry_index,
                                                        .generation = allocation.value,
                                                        .object_flags = flags}};
        }
    }
    printf("inject %u %d\n", (unsigned)type, status);
    return status < 0 ? -1 : 0;
}

static int deletion(fist_mission_object *object, uint16_t type) {
    switch (type) {
    case EFFECT:
        object->explosion.flags |= DELETED;
        break;
    case SHELL:
        object->projectile.flags |= DELETED;
        break;
    case RETIRING:
        object->vehicle.object_flags |= DELETED;
        break;
    case FIRST_AIRCRAFT:
    case SECOND_AIRCRAFT:
    case TARGET:
    case ARTILLERY:
        object->other.flags |= DELETED;
        break;
    case FIRST_TEMPORARY:
    case SECOND_TEMPORARY:
        object->saved_base.flags |= DELETED;
        break;
    case SMOKE:
        object->smoke.flags |= DELETED;
        break;
    case MUZZLE:
        object->muzzle.flags |= DELETED;
        break;
    default:
        return -1;
    }
    return 0;
}

static int preserve_released_and_orphans(const fist_mission_world *world,
                                         fist_mission_world *before) {
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (before->pool.slots[slot].used == 0) {
            continue;
        }
        fist_pool_allocation binding = {0};
        if (world->pool.slots[slot].used == 0) {
            if (deletion(&before->objects[slot], before->pool.slots[slot].type) != 0 ||
                !fist_probe_unchanged(&world->objects[slot], sizeof(world->objects[slot]),
                                      &before->objects[slot])) {
                return -1;
            }
        } else if (fist_object_pool_find(&before->pool, (uint16_t)slot, &binding) ==
                       FIST_POOL_UNAVAILABLE &&
                   !fist_probe_unchanged(&world->objects[slot], sizeof(world->objects[slot]),
                                         &before->objects[slot])) {
            return -1;
        }
    }
    return 0;
}

static int validate_request(const uint8_t *request, size_t size, fist_klc_image *height,
                            const uint8_t **commands, uint16_t *count) {
    if (size < HEADER || request[STREAM] >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    const uint32_t side = fist_read_u32le(request);
    if (side != 0 && (size_t)side > (SIZE_MAX - HEADER) / side) {
        return -1;
    }
    const size_t pixels = (size_t)side * side;
    *count = fist_read_u16le(request + COUNT);
    if (pixels > size - HEADER || size - HEADER - pixels != (size_t)*count * COMMAND) {
        return -1;
    }
    *height =
        (fist_klc_image){.width = side, .height = side, .pixels = (uint8_t *)(request + HEADER)};
    *commands = request + HEADER + pixels;
    for (size_t index = 0; index < *count; ++index) {
        const uint16_t operation = fist_read_u16le(*commands + (index * COMMAND));
        const uint16_t value = fist_read_u16le(*commands + (index * COMMAND) + sizeof(uint16_t));
        if (operation > INJECT_RETIRING || value > UINT8_MAX ||
            (operation == PREPARE && value != 0)) {
            return -1;
        }
    }
    return 0;
}

static int run(fist_units *units, const fist_mission_orders *orders, const uint8_t *request,
               size_t size) {
    fist_klc_image height = {0};
    const uint8_t *commands = NULL;
    uint16_t count = 0;
    if (validate_request(request, size, &height, &commands, &count) != 0) {
        return -1;
    }
    fist_random random = {.next_stream = request[STREAM]};
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        random.words[index] = fist_read_u16le(request + WORDS + (index * sizeof(uint16_t)));
    }
    fist_mission_world *world = malloc(sizeof(*world));
    fist_mission_world *before = malloc(sizeof(*before));
    if (world == NULL || before == NULL) {
        free(world);
        free(before);
        return -1;
    }
    fist_mission_world_reset(world);
    fist_probe_capture(world, sizeof(*world), before);
    const int status = fist_mission_world_initialize(units, &random, request[LINK], world);
    if (status != 0 && !fist_probe_unchanged(world, sizeof(*world), before)) {
        free(before);
        free(world);
        return -1;
    }
    printf("status %d\n", status);
    /* Every saved source view is gone before preparation or observations. */
    fist_units_destroy(units);
    int result = 0;
    if (status == 0) {
        world->orders = *orders;
        world->orders_loaded = 1;
        result = contracts(world, &height, before, request[LINK]);
        if (result == 0) {
            result = fist_probe_write_mission_world(world);
        }
        for (size_t index = 0; index < count && result == 0; ++index) {
            const uint8_t *command = commands + (index * COMMAND);
            const uint16_t operation = fist_read_u16le(command);
            if (operation != PREPARE) {
                result = inject(world, command);
                continue;
            }
            fist_probe_capture(world, sizeof(*world), before);
            const int prepared = fist_mission_world_prepare(world, &height, request[LINK]);
            if ((prepared != 0 && !fist_probe_unchanged(world, sizeof(*world), before)) ||
                (prepared == 0 && preserve_released_and_orphans(world, before) != 0)) {
                result = -1;
            } else {
                printf("prepare %d\n", prepared);
                result = fist_probe_write_mission_world(world);
            }
        }
    }
    free(before);
    free(world);
    return result;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_file(argv[1], &size);
    fist_scenario scenario = {0};
    fist_units units = {0};
    fist_mission_orders orders = {0};
    int status = data == NULL ? -1 : fist_scenario_decode(data, size, &scenario);
    if (status == 0) {
        status = fist_units_decode(&scenario, &units);
    }
    if (status == 0) {
        status = fist_mission_orders_decode(&scenario, &orders);
    }
    free(data);
    if (status == 0) {
        data = read_file(argv[2], &size);
        status = data == NULL ? -1 : run(&units, &orders, data, size);
        free(data);
    }
    fist_units_destroy(&units);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
