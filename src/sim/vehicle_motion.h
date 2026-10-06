#ifndef FIST_SIM_VEHICLE_MOTION_H
#define FIST_SIM_VEHICLE_MOTION_H

#include "sim/vehicle_state.h"

#include <stdint.h>

typedef struct {
    uint8_t speed_changed;
    /* Original hull servo refresh includes a zero-distance recenter update. */
    uint8_t hull_refreshed;
    uint8_t turret_changed;
} fist_vehicle_motion_events;

/* Complete ground motion + manual turret stage, in original call order.
 * Uses current throttle, requested headings, cadence phase and component data.
 * Does not advance phase or install terrain; the full vehicle tick owns those.
 * A resolved target/aim stage must run separately before targeted turret updates.
 * Return 0 on success, -1 on invalid pointers/class/components with state and
 * event output unchanged. No allocation, platform input or host time is used. */
int fist_vehicle_motion_step(fist_vehicle_state *vehicle, fist_vehicle_motion_events *out);

#endif
