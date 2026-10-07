#include "assets/bytes.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_motion.h"
#include "sim/vehicle_state.h"
#include "vehicle_probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 4,
    DESCRIPTOR = FIST_UNIT_EXTENDED_SIZE,
    RANDOM = DESCRIPTOR + (FIST_ORDER_DESCRIPTOR_WORDS * 2),
    STREAM = RANDOM + (FIST_RANDOM_STREAMS * 2),
    OPERATION = STREAM + 1,
    CASE_BYTES = OPERATION + 2,
    THROTTLE = 0,
    PROFILE_UPDATE = 1,
    DIRECT_PROFILE = 2,
    THROTTLE_MOTION = DIRECT_PROFILE + UINT8_MAX + 1,
    LAST_OPERATION = THROTTLE_MOTION
};

typedef struct {
    fist_vehicle_state actor;
    fist_order_descriptor descriptor;
    fist_random random;
    uint16_t operation;
} throttle_case;

static int invalid_outputs(fist_mission_world *world, fist_mission_world *before, uint16_t slot);

static int decode_case(const uint8_t *raw, throttle_case *out) {
    enum { MAP_X = 4, MAP_Y = 8, ALTITUDE = 12, HEADING = 16 };
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .map_x = fist_read_i32le(raw + MAP_X),
                                             .map_y = fist_read_i32le(raw + MAP_Y),
                                             .altitude = fist_read_i32le(raw + ALTITUDE),
                                             .heading = fist_read_u16le(raw + HEADING),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    out->operation = fist_read_u16le(raw + OPERATION);
    if (out->operation > LAST_OPERATION || fist_vehicle_restore(&definition, &out->actor) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_ORDER_DESCRIPTOR_WORDS; ++index) {
        out->descriptor.words[index] = fist_read_u16le(raw + DESCRIPTOR + (index * 2));
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        out->random.words[index] = fist_read_u16le(raw + RANDOM + (index * 2));
    }
    out->random.next_stream = raw[STREAM];
    return 0;
}

static int retain_target_range(const throttle_case *input) {
    enum { TARGET_RANGE = 153, HIGH_BYTE_SHIFT = 8 };
    const fist_vehicle_state before = input->actor;
    uint8_t raw[FIST_UNIT_EXTENDED_SIZE] = {0};
    raw[0] = (uint8_t)before.type;
    raw[TARGET_RANGE] = (uint8_t)before.command.target_range;
    raw[TARGET_RANGE + 1] = (uint8_t)(before.command.target_range >> HIGH_BYTE_SHIFT);
    const fist_unit_definition definition = {.type = before.type, .snapshot = {raw, sizeof(raw)}};
    for (uint8_t link = 0; link <= 2; ++link) {
        fist_random random = input->random;
        fist_vehicle_state retained = {0};
        if (fist_vehicle_initialize(&definition, &random, link, &retained) != 0 ||
            retained.command.target_range != before.command.target_range ||
            fist_vehicle_prepare(&retained, link) != 0 ||
            retained.command.target_range != before.command.target_range) {
            return -1;
        }
    }
    return 0;
}

static throttle_case *decode(const uint8_t *raw, size_t size, size_t *count) {
    if (raw == NULL || size < HEADER || (size - HEADER) % CASE_BYTES != 0 ||
        fist_read_u32le(raw) != (size - HEADER) / CASE_BYTES) {
        return NULL;
    }
    *count = fist_read_u32le(raw);
    throttle_case *cases = calloc(*count == 0 ? 1 : *count, sizeof(*cases));
    if (cases == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < *count; ++index) {
        if (decode_case(raw + HEADER + (index * CASE_BYTES), &cases[index]) != 0 ||
            (cases[index].random.next_stream < FIST_RANDOM_STREAMS &&
             retain_target_range(&cases[index]) != 0)) {
            free(cases);
            return NULL;
        }
    }
    return cases;
}

static int apply(fist_mission_world *world, uint16_t slot, uint16_t operation,
                 fist_drive_control_events *events) {
    if (operation == THROTTLE || operation == THROTTLE_MOTION) {
        const int status = fist_mission_world_throttle_command(world, slot, events);
        if (status != 0 || operation == THROTTLE) {
            return status;
        }
        fist_vehicle_motion_events motion = {0};
        return fist_vehicle_motion_step(&world->objects[slot].vehicle, &motion);
    }
    if (operation == PROFILE_UPDATE) {
        return fist_mission_world_update_command_profile(world, slot, events);
    }
    return fist_vehicle_set_drive_profile(&world->objects[slot].vehicle,
                                          (uint8_t)(operation - DIRECT_PROFILE), events);
}

static void allow_destinations(fist_mission_world *before, uint16_t slot,
                               const fist_mission_world *world, uint16_t operation) {
    static const size_t components[FIST_UNIT_GROUND_VEHICLE_COUNT] = {23, 12, 12, 22};
    fist_vehicle_state *expected = &before->objects[slot].vehicle;
    const fist_vehicle_state *actual = &world->objects[slot].vehicle;
    expected->drive.throttle = actual->drive.throttle;
    expected->control_mode = actual->control_mode;
    const size_t component = components[expected->type];
    expected->components[component] = actual->components[component];
    if (operation == THROTTLE_MOTION) {
        expected->drive = actual->drive;
        expected->turret = actual->turret;
        expected->map_x = actual->map_x;
        expected->map_y = actual->map_y;
        expected->control_flags = actual->control_flags;
        expected->operating_flags = actual->operating_flags;
        for (size_t index = 0; index < expected->component_size; ++index) {
            expected->components[index] = actual->components[index];
        }
        for (size_t index = 0; index < FIST_VEHICLE_ANIMATION_SELECTORS; ++index) {
            expected->animation_selectors[index] = actual->animation_selectors[index];
        }
    }
}

static int run(const throttle_case *input, fist_mission_world *world, bool verify_outputs,
               fist_mission_world *before) {
    fist_mission_world_reset(world);
    fist_pool_allocation allocation = {0};
    if (fist_object_pool_import(&world->pool, (fist_pool_import){input->actor.type, 0, 0},
                                &allocation) != 0) {
        return -1;
    }
    const uint16_t slot = allocation.slot;
    world->objects[slot].vehicle = input->actor;
    world->orders_loaded = 1;
    world->preparation.prepared = 1;
    world->random = input->random;
    if (input->actor.platoon < FIST_UNIT_PLATOON_COUNT) {
        world->orders.descriptors[input->actor.platoon] = input->descriptor;
    }
    if (verify_outputs && invalid_outputs(world, before, slot) != 0) {
        return -1;
    }
    fist_probe_capture(world, sizeof(*world), before);
    fist_drive_control_events events = {.refresh_drive_display = true};
    const int status = apply(world, slot, input->operation, &events);
    if (status == 0) {
        allow_destinations(before, slot, world, input->operation);
    }
    if (!fist_probe_unchanged(world, sizeof(*world), before) ||
        (status != 0 && !events.refresh_drive_display)) {
        return -1;
    }
    printf("status %d\n", status);
    fist_probe_write_vehicle_state(&world->objects[slot].vehicle);
    printf("throttle %u %u\n", (unsigned)world->objects[slot].vehicle.command.target_range,
           (unsigned)events.refresh_drive_display);
    printf("random %u", (unsigned)world->random.next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)world->random.words[index]);
    }
    puts("");
    return 0;
}

static int invalid_outputs(fist_mission_world *world, fist_mission_world *before, uint16_t slot) {
    fist_probe_capture(world, sizeof(*world), before);
    fist_drive_control_events events = {.refresh_drive_display = true};
    fist_vehicle_state *actor = &world->objects[slot].vehicle;
    if (fist_mission_world_throttle_command(NULL, slot, &events) != -1 ||
        fist_mission_world_update_command_profile(NULL, slot, &events) != -1 ||
        fist_mission_world_throttle_command(world, FIST_POOL_NO_SLOT, &events) != -1 ||
        fist_mission_world_update_command_profile(world, FIST_POOL_NO_SLOT, &events) != -1 ||
        fist_mission_world_throttle_command(world, slot, NULL) != -1 ||
        fist_mission_world_update_command_profile(world, slot, NULL) != -1 ||
        fist_vehicle_set_drive_profile(NULL, 0, &events) != -1 ||
        fist_vehicle_update_drive_profile(NULL, &events) != -1 ||
        fist_vehicle_set_drive_profile(actor, 0, NULL) != -1 ||
        fist_vehicle_update_drive_profile(actor, NULL) != -1 || !events.refresh_drive_display ||
        !fist_probe_unchanged(world, sizeof(*world), before)) {
        return -1;
    }
    return 0;
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
    throttle_case *cases = closed != 0 ? NULL : decode(data, size, &count);
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
        for (size_t index = 0; index < count; ++index) {
            if (run(&cases[index], world, index == 0, before) != 0) {
                status = -1;
                break;
            }
        }
    }
    free(cases);
    free(world);
    free(before);
    return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
