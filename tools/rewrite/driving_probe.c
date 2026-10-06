#include "app/driving.h"
#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "probe_io.h"
#include "probe_source.h"
#include "sim/weapon_control.h"
#include "vehicle_probe_io.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { ARGUMENT_COUNT = 4, HEADER_BYTES = 8, INTERVAL_BYTES = 6, KEYS_OFFSET = 4 };

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

static int write_state(const fist_driving *driving) {
    fist_weapon_status weapon = {0};
    if (fist_weapon_inspect(&driving->player, &weapon) != 0) {
        return -1;
    }
    printf("clock %" PRIu64 " %" PRIu64 " %u %u\n", driving->ticks, driving->clock_phase,
           (unsigned)driving->keys, (unsigned)driving->paused);
    fist_probe_write_vehicle_state(&driving->player);
    const fist_driving_feedback *feedback = &driving->feedback;
    printf("feedback %" PRIu64 " %" PRIu64 " %" PRIu64 " %u %u %" PRIu64 "\n", feedback->selections,
           feedback->reloads, feedback->voice_requests, (unsigned)feedback->voice_request,
           (unsigned)feedback->notice, feedback->notice_deadline);
    printf("weapon_status %u %u %u %u %u %u %u\n", (unsigned)weapon.ammunition,
           (unsigned)weapon.station_count, (unsigned)weapon.selected, (unsigned)weapon.countdown,
           (unsigned)weapon.continuous, (unsigned)weapon.reserve, (unsigned)weapon.has_reserve);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != ARGUMENT_COUNT) {
        return EXIT_FAILURE;
    }
    size_t scenario_size = 0;
    size_t request_size = 0;
    uint8_t *bytes = read_path(argv[1], &scenario_size);
    uint8_t *request = read_path(argv[3], &request_size);
    fist_scenario scenario = {0};
    if (bytes == NULL || request == NULL || request_size < HEADER_BYTES ||
        (request_size - HEADER_BYTES) % INTERVAL_BYTES != 0 ||
        fist_read_u32le(request + KEYS_OFFSET) != (request_size - HEADER_BYTES) / INTERVAL_BYTES ||
        fist_scenario_decode(bytes, scenario_size, &scenario) != 0) {
        free(bytes);
        free(request);
        return EXIT_FAILURE;
    }
    fist_probe_source storage = {.directory = argv[2]};
    const fist_asset_source source = {fist_probe_source_read, &storage};
    const fist_driving_options options = {.height_side = fist_read_u32le(request)};
    fist_driving driving = {0};
    const int loaded = fist_driving_load(&scenario, &source, &options, &driving);
    fist_probe_source_close(&storage);
    free(bytes);
    if (loaded != 0) {
        free(request);
        return EXIT_FAILURE;
    }
    int result = write_state(&driving);
    for (size_t offset = HEADER_BYTES; offset < request_size && result == 0;
         offset += INTERVAL_BYTES) {
        const fist_driving_interval interval = {fist_read_u32le(request + offset),
                                                fist_read_u16le(request + offset + KEYS_OFFSET)};
        printf("advance %d\n", fist_driving_advance(&driving, interval));
        result = write_state(&driving);
    }
    free(request);
    fist_driving_destroy(&driving);
    fist_driving_destroy(&driving);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
