#ifndef FIST_ASSETS_VEHICLE_H
#define FIST_ASSETS_VEHICLE_H

#include "assets/model.h"
#include "assets/units.h"

#include <stddef.h>
#include <stdint.h>

enum { FIST_MODEL_FAMILY_COUNT = 34, FIST_VEHICLE_MODEL_PARTS = 2 };

typedef struct {
    uint8_t model_code;
    const char *model_name;
    uint16_t scale;
    /* Original primary heading is the hull; secondary heading is the turret. */
    uint16_t headings[FIST_MODEL_ORIENTATIONS];
    /* Bit 7 selects the secondary heading, low 7 bits select the variant. */
    uint8_t parts[FIST_VEHICLE_MODEL_PARTS];
} fist_vehicle_visual;

typedef struct {
    uint16_t heading;
    uint16_t bearing;
} fist_model_angle;

/* Original even byte-offset code, 0..66. Returns NULL for absent codes. */
const char *fist_model_name_get(uint16_t code);
/* Decode the original default class render selection for ground types 0..3.
 * Copies the required immutable visual fields; no snapshot views survive.
 * Does not run vehicle initialization or select editor wrecks/other classes.
 * Returns 0 on success, -1 on invalid arguments; failure preserves out. */
int fist_vehicle_visual_decode(const fist_unit_definition *definition, fist_vehicle_visual *out);
/* Round a supplied original heading/bearing difference to direction 0..31. */
uint8_t fist_model_facing(fist_model_angle angle);
/* Return a part's facing/variant for the supplied view bearing. Returns 0 on
 * success, -1 for invalid arguments, preserving out. Model variant presence
 * is validated separately by fist_model_variant_get; it is never clamped. */
int fist_vehicle_part_pose(size_t part, const fist_vehicle_visual *visual, uint16_t bearing,
                           fist_model_pose *out);

#endif
