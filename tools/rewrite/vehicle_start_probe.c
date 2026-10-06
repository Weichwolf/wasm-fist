#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "probe_io.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { REQUEST_BYTES = 10, STATE_MARKER = 123, RANDOM_SAMPLES = 65536 };

static int same_random(const fist_random *first, const fist_random *second) {
    if (first->next_stream != second->next_stream) {
        return 0;
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        if (first->words[index] != second->words[index]) {
            return 0;
        }
    }
    return 1;
}

static int unchanged(const fist_vehicle_state *state) {
    const unsigned char *bytes = (const unsigned char *)state;
    for (size_t index = 0; index < sizeof(*state); ++index) {
        if (bytes[index] != STATE_MARKER) {
            return 0;
        }
    }
    return 1;
}

static int invalid_inputs(void) {
    uint8_t snapshot[FIST_UNIT_EXTENDED_SIZE] = {0};
    fist_unit_definition definition = {.snapshot = {snapshot, sizeof(snapshot)}};
    fist_random random = {.words = {1, 2, 3, 4}};
    const fist_random previous = random;
    fist_vehicle_state state = {0};
    unsigned char *state_bytes = (unsigned char *)&state;
    for (size_t index = 0; index < sizeof(state); ++index) {
        state_bytes[index] = STATE_MARKER;
    }
    uint16_t value = STATE_MARKER;
    if (fist_vehicle_initialize(NULL, &random, 0, &state) != -1 ||
        fist_vehicle_initialize(&definition, NULL, 0, &state) != -1 ||
        fist_vehicle_initialize(&definition, &random, 0, NULL) != -1 ||
        fist_random_next(NULL, &value) != -1 || fist_random_next(&random, NULL) != -1) {
        return -1;
    }
    for (size_t size = 0; size <= sizeof(snapshot) + 1; ++size) {
        if (size == sizeof(snapshot)) {
            continue;
        }
        definition.snapshot.size = size;
        if (fist_vehicle_initialize(&definition, &random, 0, &state) != -1 || !unchanged(&state) ||
            !same_random(&random, &previous)) {
            return -1;
        }
    }
    definition.snapshot.size = sizeof(snapshot);
    for (unsigned type = FIST_UNIT_GROUND_VEHICLE_COUNT; type <= UINT16_MAX; ++type) {
        definition.type = (uint16_t)type;
        if (fist_vehicle_initialize(&definition, &random, 0, &state) != -1 || !unchanged(&state) ||
            !same_random(&random, &previous)) {
            return -1;
        }
    }
    definition.type = 0;
    snapshot[0] = 1;
    if (fist_vehicle_initialize(&definition, &random, 0, &state) != -1) {
        return -1;
    }
    snapshot[0] = 0;
    definition.snapshot.data = NULL;
    if (fist_vehicle_initialize(&definition, &random, 0, &state) != -1 ||
        !same_random(&random, &previous) || !unchanged(&state)) {
        return -1;
    }
    definition.snapshot.data = snapshot;
    random.next_stream = FIST_RANDOM_STREAMS;
    const fist_random invalid = random;
    return fist_random_next(&random, &value) == -1 && value == STATE_MARKER &&
                   fist_vehicle_initialize(&definition, &random, 0, &state) == -1 &&
                   same_random(&random, &invalid) && unchanged(&state)
               ? 0
               : -1;
}

static int read_request(const char *path, fist_random *random, uint8_t *link) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return -1;
    }
    uint8_t data[REQUEST_BYTES] = {0};
    const size_t count = fread(data, 1, sizeof(data), file);
    if (count != sizeof(data) || ferror(file) != 0) {
        (void)fclose(file);
        return -1;
    }
    const int extra = fgetc(file);
    const int error = ferror(file);
    const int closed = fclose(file);
    if (count != sizeof(data) || extra != EOF || error != 0 || closed != 0 ||
        data[REQUEST_BYTES - 2] >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        random->words[index] = fist_read_u16le(data + (index * 2));
    }
    random->next_stream = data[REQUEST_BYTES - 2];
    *link = data[REQUEST_BYTES - 1];
    return 0;
}

static void write_random(const fist_random *random) {
    printf("random %u", (unsigned)random->next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)random->words[index]);
    }
    puts("");
}

static int sample_random(fist_random *random) {
    for (unsigned index = 0; index < RANDOM_SAMPLES; ++index) {
        uint16_t value = 0;
        if (fist_random_next(random, &value) != 0) {
            return -1;
        }
        printf("%u\n", (unsigned)value);
    }
    write_random(random);
    return 0;
}

static int sweep_random(fist_random *random) {
    const uint8_t selected = random->next_stream;
    for (unsigned previous = 0; previous < RANDOM_SAMPLES; ++previous) {
        random->next_stream = selected;
        random->words[selected] = (uint16_t)previous;
        uint16_t value = 0;
        if (fist_random_next(random, &value) != 0) {
            return -1;
        }
        printf("%u\n", (unsigned)value);
    }
    write_random(random);
    return 0;
}

static void write_state(const fist_vehicle_state *state) {
    const fist_vehicle_drive *drive = &state->drive;
    const fist_vehicle_turret *turret = &state->turret;
    printf("state %u %u %u %d %d %d %d %d %d %d %d %u %u %u %u %u %u %u %u\n",
           (unsigned)state->type, (unsigned)state->registry_index, (unsigned)state->generation,
           state->map_x, state->map_y, state->altitude, drive->speed, drive->throttle,
           drive->terrain_pitch, drive->velocity_x, drive->velocity_y, (unsigned)drive->heading,
           (unsigned)drive->requested_heading, (unsigned)drive->movement_gate,
           (unsigned)drive->motion_flags, (unsigned)drive->update_phase, (unsigned)turret->heading,
           (unsigned)turret->offset, (unsigned)turret->requested_offset);
    printf("control %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)state->projection_extent, (unsigned)state->projection_scale,
           (unsigned)state->camera_height, (unsigned)state->control_flags,
           (unsigned)state->object_flags, (unsigned)state->secondary_flags,
           (unsigned)state->operating_flags, (unsigned)state->random_phases[0],
           (unsigned)state->random_phases[1], (unsigned)state->control_mode,
           (unsigned)state->turret_view_mode, (unsigned)state->hull_view_mode,
           (unsigned)state->behavior, (unsigned)state->reload_countdown,
           (unsigned)state->component_size);
    printf("weapons");
    for (size_t slot = 0; slot < FIST_VEHICLE_WEAPON_SLOTS; ++slot) {
        printf(" %u", (unsigned)state->weapons.rounds[slot]);
    }
    printf(" %u %u %u %u\n", (unsigned)state->weapons.class_parameter,
           (unsigned)state->weapons.cycle[0], (unsigned)state->weapons.cycle[1],
           (unsigned)state->weapons.ready_stock);
    printf("components");
    for (size_t index = 0; index < state->component_size; ++index) {
        printf(" %02x", (unsigned)state->components[index]);
    }
    puts("");
}

static int initialize_units(const char *path, fist_random *random, uint8_t link) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return -1;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    fist_units units = {0};
    if (data == NULL || closed != 0 || fist_scenario_decode(data, size, &scenario) != 0 ||
        fist_units_decode(&scenario, &units) != 0) {
        free(data);
        return -1;
    }
    free(data);
    fist_vehicle_state *states = calloc(units.count + 1, sizeof(*states));
    if (states == NULL) {
        fist_units_destroy(&units);
        return -1;
    }
    size_t count = 0;
    int result = 0;
    for (size_t index = 0; index < units.count; ++index) {
        const fist_unit_definition *definition = &units.definitions[index];
        if (definition->type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
            if (fist_vehicle_initialize(definition, random, link, &states[count]) != 0) {
                result = -1;
                break;
            }
            ++count;
        }
    }
    fist_units_destroy(&units);
    /* All FSG/source/snapshot storage is gone before observing start state. */
    for (size_t index = 0; index < count && result == 0; ++index) {
        write_state(&states[index]);
    }
    if (result == 0) {
        write_random(random);
    }
    free(states);
    return result;
}

int main(int argc, char **argv) {
    if (invalid_inputs() != 0 || argc < 3) {
        return EXIT_FAILURE;
    }
    fist_random random = {0};
    uint8_t link = 0;
    if (read_request(argv[2], &random, &link) != 0) {
        return EXIT_FAILURE;
    }
    int result = -1;
    if (argc == 3 && strcmp(argv[1], "random") == 0) {
        result = sample_random(&random);
    } else if (argc == 3 && strcmp(argv[1], "sweep") == 0) {
        result = sweep_random(&random);
    } else if (argc == 4 && strcmp(argv[1], "units") == 0) {
        result = initialize_units(argv[3], &random, link);
    }
    return result == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
