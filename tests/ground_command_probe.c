#include "assets/bytes.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 4,
    TREE = 21,
    DESCRIPTOR = FIST_UNIT_EXTENDED_SIZE,
    PHASE_RANDOM = DESCRIPTOR + (FIST_ORDER_DESCRIPTOR_WORDS * 2),
    RANDOM = PHASE_RANDOM + 2,
    STREAM = RANDOM + (FIST_RANDOM_STREAMS * 2),
    CASE_BYTES = STREAM + 1
};

typedef struct {
    fist_vehicle_state actor;
    fist_order_descriptor descriptor;
    fist_random random;
    uint16_t phase_random;
} command_case;

static command_case *decode(const uint8_t *data, size_t size, size_t *count) {
    if (size < HEADER || fist_read_u32le(data) != (size - HEADER) / CASE_BYTES ||
        (size - HEADER) % CASE_BYTES != 0) {
        return NULL;
    }
    *count = fist_read_u32le(data);
    command_case *cases = calloc(*count == 0 ? 1 : *count, sizeof(*cases));
    if (cases == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < *count; ++index) {
        const uint8_t *raw = data + HEADER + (index * CASE_BYTES);
        const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                                 .map_x = fist_read_i32le(raw + 4),
                                                 .map_y = fist_read_i32le(raw + 8),
                                                 .altitude = fist_read_i32le(raw + 12),
                                                 .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
        if (fist_vehicle_restore(&definition, &cases[index].actor) != 0) {
            free(cases);
            return NULL;
        }
        for (size_t word = 0; word < FIST_ORDER_DESCRIPTOR_WORDS; ++word) {
            cases[index].descriptor.words[word] = fist_read_u16le(raw + DESCRIPTOR + (word * 2));
        }
        for (size_t word = 0; word < FIST_RANDOM_STREAMS; ++word) {
            cases[index].random.words[word] = fist_read_u16le(raw + RANDOM + (word * 2));
        }
        cases[index].random.next_stream = raw[STREAM];
        cases[index].phase_random = fist_read_u16le(raw + PHASE_RANDOM);
    }
    return cases;
}

static int run_case(fist_mission_world *world, fist_mission_world *before,
                    const command_case *input) {
    fist_mission_world_reset(world);
    fist_pool_allocation allocation = {0};
    const fist_pool_import request = {input->actor.type, 0, 0};
    if (fist_object_pool_import(&world->pool, request, &allocation) != 0) {
        return -1;
    }
    const uint16_t slot = allocation.slot;
    world->objects[slot].vehicle = input->actor;
    world->orders_loaded = 1;
    world->random = input->random;
    if (input->actor.platoon < FIST_UNIT_PLATOON_COUNT) {
        world->orders.descriptors[input->actor.platoon] = input->descriptor;
    }
    fist_probe_capture(world, sizeof(*world), before);
    if (fist_mission_world_select_command(
            NULL, (fist_command_selection){slot, input->phase_random}) != -1 ||
        fist_mission_world_select_command(
            world, (fist_command_selection){FIST_POOL_NO_SLOT, input->phase_random}) != -1 ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    const int status = fist_mission_world_select_command(
        world, (fist_command_selection){slot, input->phase_random});
    if (status == 0) {
        before->objects[slot].vehicle.command.mode = world->objects[slot].vehicle.command.mode;
        before->objects[slot].vehicle.control_flags = world->objects[slot].vehicle.control_flags;
        before->random = world->random;
    }
    if (!fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    printf("status %d\n", status);
    fist_probe_write_vehicle_state(&world->objects[slot].vehicle);
    printf("random %u", (unsigned)world->random.next_stream);
    for (size_t word = 0; word < FIST_RANDOM_STREAMS; ++word) {
        printf(" %u", (unsigned)world->random.words[word]);
    }
    puts("");
    return 0;
}

static int rejected(fist_mission_world *world, fist_mission_world *before, uint16_t slot) {
    fist_probe_capture(world, sizeof(*world), before);
    return fist_mission_world_select_command(world, (fist_command_selection){slot, UINT8_MAX}) ==
                       -1 &&
                   fist_probe_unchanged(world, sizeof(*world), before)
               ? 0
               : -1;
}

static int invalid_worlds(fist_mission_world *world, fist_mission_world *before) {
    fist_mission_world_reset(world);
    fist_pool_allocation allocation = {0};
    const fist_pool_import request = {0, 0, 0};
    if (fist_object_pool_import(&world->pool, request, &allocation) != 0) {
        return -1;
    }
    const uint16_t slot = allocation.slot;
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    actor->component_size = fist_vehicle_component_size(0);
    actor->command.mode = UINT8_MAX;
    for (unsigned loaded = 0; loaded <= UINT8_MAX; ++loaded) {
        if (loaded == 1) {
            continue;
        }
        world->orders_loaded = (uint8_t)loaded;
        if (rejected(world, before, slot) != 0) {
            return -1;
        }
    }
    world->orders_loaded = 1;
    if (rejected(world, before, 0) != 0) {
        return -1;
    }
    for (size_t size = 0; size <= FIST_VEHICLE_COMPONENT_BYTES + 1; ++size) {
        if (size == fist_vehicle_component_size(0)) {
            continue;
        }
        actor->component_size = size;
        if (rejected(world, before, slot) != 0) {
            return -1;
        }
    }
    actor->component_size = fist_vehicle_component_size(0);
    actor->type = 1;
    if (rejected(world, before, slot) != 0) {
        return -1;
    }
    actor->type = 0;
    world->pool.slots[slot].type = TREE;
    if (rejected(world, before, slot) != 0) {
        return -1;
    }
    world->pool.slots[slot].type = 0;
    world->pool.registry[0].slot = FIST_UNIT_REGISTRY_COUNT;
    return rejected(world, before, slot);
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
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    size_t count = 0;
    command_case *cases = data == NULL || closed != 0 ? NULL : decode(data, size, &count);
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
        status = invalid_worlds(world, before);
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        status = run_case(world, before, &cases[index]);
    }
    free(before);
    free(world);
    free(cases);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
