#include "assets/model.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "assets/vehicle.h"
#include "probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CONTRACT_FAILURE = 2, ANGLE_COUNT = 65536, MARKER_CODE = 255 };

typedef struct {
    size_t ordinal;
    fist_vehicle_visual visual;
} vehicle_observation;

static int visual_marker_intact(const fist_vehicle_visual *visual) {
    return visual->model_code == MARKER_CODE && visual->model_name == NULL && visual->scale == 0 &&
           visual->headings[0] == 0 && visual->headings[1] == 0 && visual->parts[0] == 0 &&
           visual->parts[1] == 0;
}

static int check_invalid(void) {
    fist_vehicle_visual visual = {.model_code = MARKER_CODE};
    fist_model_pose pose = {.facing = MARKER_CODE};
    uint8_t snapshot[FIST_UNIT_EXTENDED_SIZE] = {0};
    fist_unit_definition definition = {.snapshot = {snapshot, sizeof(snapshot)}};
    if (fist_vehicle_visual_decode(NULL, &visual) != -1 ||
        fist_vehicle_visual_decode(&definition, NULL) != -1 || visual_marker_intact(&visual) == 0 ||
        fist_vehicle_part_pose(0, NULL, 0, &pose) != -1 ||
        fist_vehicle_part_pose(FIST_VEHICLE_MODEL_PARTS, &visual, 0, &pose) != -1 ||
        fist_vehicle_part_pose(0, &visual, 0, NULL) != -1 || pose.facing != MARKER_CODE ||
        pose.part != 0 || pose.variant != 0) {
        return -1;
    }
    for (unsigned type = FIST_UNIT_GROUND_VEHICLE_COUNT; type <= UINT16_MAX; ++type) {
        definition.type = (uint16_t)type;
        if (fist_vehicle_visual_decode(&definition, &visual) != -1 ||
            visual_marker_intact(&visual) == 0) {
            return -1;
        }
    }
    definition.type = 0;
    for (size_t size = 0; size < sizeof(snapshot); ++size) {
        definition.snapshot.size = size;
        if (fist_vehicle_visual_decode(&definition, &visual) != -1 ||
            visual_marker_intact(&visual) == 0) {
            return -1;
        }
    }
    definition.snapshot.size = sizeof(snapshot) + 1;
    if (fist_vehicle_visual_decode(&definition, &visual) != -1 ||
        visual_marker_intact(&visual) == 0) {
        return -1;
    }
    definition.snapshot.size = sizeof(snapshot);
    snapshot[0] = 1;
    if (fist_vehicle_visual_decode(&definition, &visual) != -1 ||
        visual_marker_intact(&visual) == 0) {
        return -1;
    }
    definition.snapshot.data = NULL;
    return fist_vehicle_visual_decode(&definition, &visual) == -1 &&
                   visual_marker_intact(&visual) != 0
               ? 0
               : -1;
}

static int write_catalog(void) {
    for (unsigned code = 0; code <= UINT16_MAX; ++code) {
        const char *name = fist_model_name_get((uint16_t)code);
        const int exists = code % 2 == 0 && code / 2 < FIST_MODEL_FAMILY_COUNT;
        if ((name != NULL) != exists) {
            return CONTRACT_FAILURE;
        }
        if (name != NULL) {
            printf("model %u %s\n", code, name);
        }
    }
    return EXIT_SUCCESS;
}

static int write_angles(uint16_t bearing) {
    for (unsigned heading = 0; heading < ANGLE_COUNT; ++heading) {
        const fist_model_angle angle = {.heading = (uint16_t)heading, .bearing = bearing};
        printf("%u\n", (unsigned)fist_model_facing(angle));
    }
    return EXIT_SUCCESS;
}

static int write_visual(const vehicle_observation *observation, uint16_t bearing) {
    const fist_vehicle_visual *visual = &observation->visual;
    printf("vehicle %zu %u %s %u %u %u %u %u", observation->ordinal, (unsigned)visual->model_code,
           visual->model_name, (unsigned)visual->scale, (unsigned)visual->headings[0],
           (unsigned)visual->headings[1], (unsigned)visual->parts[0], (unsigned)visual->parts[1]);
    for (size_t part = 0; part < FIST_VEHICLE_MODEL_PARTS; ++part) {
        fist_model_pose pose = {0};
        if (fist_vehicle_part_pose(part, visual, bearing, &pose) != 0) {
            return CONTRACT_FAILURE;
        }
        printf(" %u %zu %zu", (unsigned)pose.facing, pose.part, pose.variant);
    }
    printf("\n");
    return EXIT_SUCCESS;
}

static int write_units(const char *path, uint16_t bearing) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    fist_units units = {0};
    if (data == NULL || closed != 0 || fist_scenario_decode(data, size, &scenario) != 0 ||
        fist_units_decode(&scenario, &units) != 0) {
        free(data);
        return EXIT_FAILURE;
    }
    for (size_t offset = 0; offset < size; ++offset) {
        data[offset] = 0;
    }
    free(data);
    vehicle_observation *observations = calloc(units.count + 1, sizeof(*observations));
    if (observations == NULL) {
        fist_units_destroy(&units);
        return EXIT_FAILURE;
    }
    size_t count = 0;
    int result = EXIT_SUCCESS;
    for (size_t ordinal = 0; ordinal < units.count; ++ordinal) {
        const fist_unit_definition *definition = &units.definitions[ordinal];
        if (definition->type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
            continue;
        }
        observations[count].ordinal = ordinal;
        if (fist_vehicle_visual_decode(definition, &observations[count].visual) != 0) {
            result = CONTRACT_FAILURE;
            break;
        }
        ++count;
    }
    fist_units_destroy(&units);
    /* All snapshots/definitions are gone before observing any visual/pose. */
    for (size_t index = 0; index < count && result == EXIT_SUCCESS; ++index) {
        result = write_visual(&observations[index], bearing);
    }
    free(observations);
    return result;
}

static int parse_bearing(const char *text, uint16_t *out) {
    char *end = NULL;
    const unsigned long value = strtoul(text, &end, 10);
    if (text[0] == '\0' || end == NULL || *end != '\0' || value > UINT16_MAX) {
        return -1;
    }
    *out = (uint16_t)value;
    return 0;
}

int main(int argc, char **argv) {
    if (check_invalid() != 0) {
        return CONTRACT_FAILURE;
    }
    int result = EXIT_FAILURE;
    uint16_t bearing = 0;
    if (argc == 2 && strcmp(argv[1], "catalog") == 0) {
        result = write_catalog();
    } else if (argc == 3 && strcmp(argv[1], "angles") == 0 &&
               parse_bearing(argv[2], &bearing) == 0) {
        result = write_angles(bearing);
    } else if (argc == 4 && strcmp(argv[1], "units") == 0 &&
               parse_bearing(argv[3], &bearing) == 0) {
        result = write_units(argv[2], bearing);
    }
    return ferror(stdout) == 0 ? result : EXIT_FAILURE;
}
