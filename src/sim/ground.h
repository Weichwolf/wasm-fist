#ifndef FIST_SIM_GROUND_H
#define FIST_SIM_GROUND_H

#include "assets/klc.h"
#include "sim/vehicle_state.h"

#include <stdint.h>

typedef struct {
    int32_t map_x;
    int32_t map_y;
    uint16_t heading;
} fist_ground_pose;

typedef struct {
    uint8_t height;
    int16_t roll;
    int16_t pitch;
} fist_ground_contact;

/* Original installed height/7fa0 slope query. Borrows a complete positive
 * square power-of-two height plane, with at most 16 index bits per axis.
 * Coordinates wrap at the original 524288-unit map period. Heading selects
 * the original fixed four-sample footprint; returned slopes are signed turns.
 * Return 0 on success, -1 on invalid/overflow input, preserving out. */
int fist_ground_sample(const fist_klc_image *height, const fist_ground_pose *pose,
                       fist_ground_contact *out);

/* Original op 1c contact publication for one ground vehicle. Updates ground
 * height, hull roll/pitch and independent absolute-turret roll/pitch only.
 * The subsequent class tick owns transfer to altitude; no timing is advanced.
 * Caller chooses which registry/roster vehicles participate in this pass.
 * Return 0 on success, -1 on invalid field/class/components, preserving state. */
int fist_vehicle_ground_update(fist_vehicle_state *vehicle, const fist_klc_image *height);

#endif
