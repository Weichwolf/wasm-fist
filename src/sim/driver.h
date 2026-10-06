#ifndef FIST_SIM_DRIVER_H
#define FIST_SIM_DRIVER_H

#include "assets/klc.h"
#include "sim/vehicle_state.h"

#include <stdint.h>

enum {
    FIST_DRIVER_TURN_INCREMENT = 182,
    FIST_DRIVER_TURRET_INCREMENT = 364,
    FIST_DRIVER_THROTTLE_LIMIT = 254,
    FIST_DRIVER_PIT_RATE = 1193182,
    FIST_DRIVER_TICK_COUNTS = 19886
};

typedef struct {
    int8_t throttle_change;
    int8_t steering;
    int16_t turret_change;
    uint8_t throttle_off;
} fist_driver_controls;

/* Manual driving stage: recovered action increments, class altitude-byte
 * transfer, shared movement/manual turret, phase +2 and final ground contact.
 * Caller owns elapsed time and roster participation; full class weapon/AI/HUD/
 * damage methods are separate. Return -1 on invalid input, preserving vehicle. */
int fist_driver_step(fist_vehicle_state *vehicle, const fist_klc_image *height,
                     const fist_driver_controls *controls);

#endif
