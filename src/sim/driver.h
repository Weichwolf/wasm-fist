#ifndef FIST_SIM_DRIVER_H
#define FIST_SIM_DRIVER_H

#include "assets/klc.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"

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

/* Manual driving stage: actions, altitude-byte transfer, gun/recoil prefix,
 * movement/manual turret, phase +2, reload dispatch and final ground contact.
 * Caller owns elapsed time and roster participation. Firing/AI/damage remain
 * separate. Invalid input preserves both vehicle and weapon events. */
int fist_driver_step(fist_vehicle_state *vehicle, const fist_klc_image *height,
                     const fist_driver_controls *controls, fist_weapon_events *events);

/* Original aae8 control refresh, also used after a player weapon selection. */
int fist_driver_take_control(fist_vehicle_state *vehicle);

#endif
