#include "app/driving.h"
#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/units.h"
#include "mission_probe_io.h"
#include "probe_io.h"
#include "probe_source.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "vehicle_probe_io.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    ARGUMENT_COUNT = 4,
    HEADER_BYTES = 8,
    MISSION_HEADER_BYTES = 17,
    RANDOM_OFFSET = 8,
    CURSOR_OFFSET = 16,
    INTERVAL_BYTES = 6,
    KEYS_OFFSET = 4
};

static uint8_t *read_path(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    uint8_t *bytes = fist_probe_read_file(file, size);
    if (fclose(file) != 0) {
        free(bytes);
        return NULL;
    }
    return bytes;
}

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

static int checked_load(const fist_scenario *scenario, const fist_asset_source *source,
                        const fist_driving_options *options, fist_driving *out, int *status) {
    const fist_driving prior = *out;
    const fist_driving_options random_detail = *options;
    if (fist_driving_load_mission(NULL, source, options, out) != -1 ||
        fist_driving_load_mission(scenario, NULL, options, out) != -1 ||
        fist_driving_load_mission(scenario, source, NULL, out) != -1 ||
        fist_driving_load_mission(scenario, source, options, NULL) != -1 ||
        !same_bytes(out, sizeof(prior), &prior)) {
        return -1;
    }
    *status = fist_driving_load_mission(scenario, source, options, out);
    return same_bytes(options, sizeof(random_detail), &random_detail) &&
                   (*status == 0 || same_bytes(out, sizeof(prior), &prior))
               ? 0
               : -1;
}

static int checked_advance(fist_driving *driving, fist_driving_interval interval, int *status) {
    if (driving->world == NULL) {
        *status = fist_driving_advance(driving, interval);
        return 0;
    }
    fist_mission_world *before = malloc(sizeof(*before));
    if (before == NULL) {
        return -1;
    }
    *before = *driving->world;
    const fist_driving session = *driving;
    *status = fist_driving_advance(driving, interval);
    int preserved = 0;
    if (*status != 0) {
        preserved = same_bytes(driving, sizeof(session), &session);
    } else {
        const uint16_t slot = driving->world->combat.selected_slot;
        const fist_vehicle_state changed = driving->world->objects[slot].vehicle;
        driving->world->objects[slot].vehicle = before->objects[slot].vehicle;
        preserved = same_bytes(driving->world, sizeof(*before), before);
        driving->world->objects[slot].vehicle = changed;
    }
    if (*status != 0) {
        preserved = preserved && same_bytes(driving->world, sizeof(*before), before);
    }
    free(before);
    return preserved ? 0 : -1;
}

static int rejects_invalid_selection(fist_driving *driving) {
    int status = 0;
    return fist_driving_player(driving) == NULL &&
                   checked_advance(driving, (fist_driving_interval){1, FIST_DRIVE_FASTER},
                                   &status) == 0 &&
                   status == -1
               ? 0
               : -1;
}

static int mission_contract(fist_driving *driving) {
    fist_mission_world *world = driving->world;
    const uint16_t selected = world->combat.selected_slot;
    if (driving->preview_player != NULL ||
        fist_driving_player(driving) != &world->objects[selected].vehicle ||
        fist_driving_player(NULL) != NULL ||
        fist_driving_advance(NULL, (fist_driving_interval){0, 0}) != -1) {
        return -1;
    }
    for (size_t slot = 0; slot <= FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (slot < FIST_UNIT_REGISTRY_COUNT && world->pool.slots[slot].used != 0 &&
            world->pool.slots[slot].type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
            continue;
        }
        world->combat.selected_slot = (uint16_t)slot;
        const int rejected = rejects_invalid_selection(driving);
        world->combat.selected_slot = selected;
        if (rejected != 0) {
            return -1;
        }
    }
    world->combat.selected_slot = FIST_POOL_NO_SLOT;
    const int absent = rejects_invalid_selection(driving);
    world->combat.selected_slot = selected;
    driving->preview_player = &world->objects[selected].vehicle;
    const int duplicate = rejects_invalid_selection(driving);
    driving->preview_player = NULL;
    world->pending_player_impact = selected;
    int status = 0;
    const int suspended = checked_advance(driving, (fist_driving_interval){1, 0}, &status);
    world->pending_player_impact = FIST_POOL_NO_SLOT;
    return absent == 0 && duplicate == 0 && suspended == 0 && status == -1 ? 0 : -1;
}

static int write_state(const fist_driving *driving) {
    const fist_vehicle_state *player = fist_driving_player(driving);
    fist_weapon_status weapon = {0};
    if (player == NULL || fist_weapon_inspect(player, &weapon) != 0) {
        return -1;
    }
    printf("clock %" PRIu64 " %" PRIu64 " %u %u\n", driving->ticks, driving->clock_phase,
           (unsigned)driving->keys, (unsigned)driving->paused);
    fist_probe_write_vehicle_state(player);
    const fist_driving_feedback *feedback = &driving->feedback;
    printf("feedback %" PRIu64 " %" PRIu64 " %" PRIu64 " %u %u %" PRIu64 "\n", feedback->selections,
           feedback->reloads, feedback->voice_requests, (unsigned)feedback->voice_request,
           (unsigned)feedback->notice, feedback->notice_deadline);
    printf("weapon_status %u %u %u %u %u %u %u\n", (unsigned)weapon.ammunition,
           (unsigned)weapon.station_count, (unsigned)weapon.selected, (unsigned)weapon.countdown,
           (unsigned)weapon.continuous, (unsigned)weapon.reserve, (unsigned)weapon.has_reserve);
    if (driving->world != NULL) {
        printf("selection %u %u\n", (unsigned)driving->world->combat.selected_slot,
               (unsigned)driving->world->pending_player_impact);
        return fist_probe_write_mission_world(driving->world);
    }
    return 0;
}

int main(int argc, char **argv) {
    const int mission = argc == ARGUMENT_COUNT + 1 && strcmp(argv[ARGUMENT_COUNT], "mission") == 0;
    if (argc != ARGUMENT_COUNT && mission == 0) {
        return EXIT_FAILURE;
    }
    const size_t header_bytes = mission != 0 ? MISSION_HEADER_BYTES : HEADER_BYTES;
    size_t scenario_size = 0;
    size_t request_size = 0;
    uint8_t *bytes = read_path(argv[1], &scenario_size);
    uint8_t *request = read_path(argv[3], &request_size);
    fist_scenario scenario = {0};
    if (bytes == NULL || request == NULL || request_size < header_bytes ||
        (request_size - header_bytes) % INTERVAL_BYTES != 0 ||
        fist_read_u32le(request + KEYS_OFFSET) != (request_size - header_bytes) / INTERVAL_BYTES ||
        fist_scenario_decode(bytes, scenario_size, &scenario) != 0) {
        free(bytes);
        free(request);
        return EXIT_FAILURE;
    }
    fist_probe_source storage = {.directory = argv[2]};
    const fist_asset_source source = {fist_probe_source_read, &storage};
    fist_driving_options options = {.height_side = fist_read_u32le(request)};
    if (mission != 0) {
        for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
            options.random.words[index] =
                fist_read_u16le(request + RANDOM_OFFSET + (index * sizeof(uint16_t)));
        }
        options.random.next_stream = request[CURSOR_OFFSET];
    }
    fist_driving driving = {.ticks = UINT64_MAX};
    int loaded = 0;
    if (mission != 0) {
        if (checked_load(&scenario, &source, &options, &driving, &loaded) != 0) {
            fist_driving_destroy(&driving);
            fist_probe_source_close(&storage);
            free(bytes);
            free(request);
            return EXIT_FAILURE;
        }
    } else {
        loaded = fist_driving_load(&scenario, &source, &options, &driving);
    }
    fist_probe_source_close(&storage);
    free(bytes);
    if (loaded != 0) {
        if (mission != 0) {
            printf("load %d\n", loaded);
        }
        free(request);
        return EXIT_FAILURE;
    }
    int result = mission != 0 ? mission_contract(&driving) : 0;
    if (result == 0) {
        result = write_state(&driving);
    }
    for (size_t offset = header_bytes; offset < request_size && result == 0;
         offset += INTERVAL_BYTES) {
        const fist_driving_interval interval = {fist_read_u32le(request + offset),
                                                fist_read_u16le(request + offset + KEYS_OFFSET)};
        int advanced = 0;
        if (checked_advance(&driving, interval, &advanced) != 0) {
            result = -1;
            break;
        }
        printf("advance %d\n", advanced);
        result = write_state(&driving);
    }
    free(request);
    fist_driving_destroy(&driving);
    fist_driving_destroy(&driving);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
