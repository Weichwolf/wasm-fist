#include "assets/vehicle.h"

#include "assets/bytes.h"
#include "assets/model.h"
#include "assets/units.h"

#include <stddef.h>
#include <stdint.h>

static const char *const model_names[FIST_MODEL_FAMILY_COUNT] = {
    "M1_A",  "M1_B",   "M1_C",    "M1_D",   "M1_E",    "M1_DEAD",  "M3_A",    "M3_B",   "M3_C",
    "M3_D",  "M3_E",   "M3_DEAD", "T80_A",  "T80_B",   "T80_C",    "T80_D",   "T80_E",  "T80_DEAD",
    "BMP_A", "BMP_B",  "BMP_C",   "BMP_D",  "BMP_E",   "BMP_DEAD", "EXPLODE", "APACHE", "HIND",
    "SMOKE", "MUZZLE", "TREES",   "GSMOKE", "TARGETS", "SHOT",     "ARTILL"};

const char *fist_model_name_get(uint16_t code) {
    if (code % 2 != 0 || code / 2 >= FIST_MODEL_FAMILY_COUNT) {
        return NULL;
    }
    return model_names[code / 2];
}

int fist_vehicle_visual_decode(const fist_unit_definition *definition, fist_vehicle_visual *out) {
    enum { SCALE_OFFSET = 20, TURRET_HEADING_OFFSET = 38, PARTS_OFFSET = 169, BYTE_BITS = 8 };
    static const uint8_t default_codes[FIST_UNIT_GROUND_VEHICLE_COUNT] = {4, 16, 28, 40};
    if (definition == NULL || out == NULL || definition->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        definition->snapshot.data == NULL || definition->snapshot.size != FIST_UNIT_EXTENDED_SIZE ||
        fist_read_u16le(definition->snapshot.data) != definition->type) {
        return -1;
    }
    const uint8_t *snapshot = definition->snapshot.data;
    fist_vehicle_visual visual = {
        .model_code = default_codes[definition->type],
        .scale = (uint16_t)(fist_read_u16le(snapshot + SCALE_OFFSET) >> BYTE_BITS),
        .headings = {fist_read_u16le(snapshot + TURRET_HEADING_OFFSET), definition->heading}};
    visual.model_name = fist_model_name_get(visual.model_code);
    for (size_t part = 0; part < FIST_VEHICLE_MODEL_PARTS; ++part) {
        visual.parts[part] = (uint8_t)(snapshot[PARTS_OFFSET + part] ^ FIST_MODEL_PART_VARIANTS);
    }
    *out = visual;
    return 0;
}

uint8_t fist_model_facing(fist_model_angle angle) {
    enum { DIRECTION_SHIFT = 10, DIRECTION_OFFSET_MASK = 62 };
    const uint16_t difference = (uint16_t)(angle.heading - angle.bearing);
    const unsigned rounded = (unsigned)(difference >> DIRECTION_SHIFT) + 1;
    return (uint8_t)((rounded & DIRECTION_OFFSET_MASK) / 2);
}

int fist_vehicle_part_pose(size_t part, const fist_vehicle_visual *visual, uint16_t bearing,
                           fist_model_pose *out) {
    if (visual == NULL || out == NULL || part >= FIST_VEHICLE_MODEL_PARTS) {
        return -1;
    }
    const uint8_t flags = visual->parts[part];
    const size_t orientation = flags / FIST_MODEL_PART_VARIANTS;
    const fist_model_angle angle = {.heading = visual->headings[orientation], .bearing = bearing};
    *out = (fist_model_pose){.facing = fist_model_facing(angle),
                             .part = part,
                             .variant = flags % FIST_MODEL_PART_VARIANTS};
    return 0;
}
