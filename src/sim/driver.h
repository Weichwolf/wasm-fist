#ifndef FIST_SIM_DRIVER_H
#define FIST_SIM_DRIVER_H

#include "assets/klc.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"

#include <stdbool.h>
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

typedef struct {
    /* Retained original shared 9600/9602/9604 and 9746 owners. */
    fist_weapon_elevation_controls elevation;
    fist_weapon_turret_controls turret;
} fist_manual_controls;

typedef struct {
    uint16_t drive_mode;
    /* Borrowed original 0452 clock, independent of simulation ticks. */
    uint16_t clock;
    bool selected;
} fist_manual_input;

typedef struct {
    fist_drive_control_events drive;
    /* Complete f69:798b refresh of all three original view-display controls;
     * a producer notification, independent of platform drawing/caching. */
    bool refresh_view_display;
} fist_manual_events;

/* Complete selected a57a composition. Inhibition only skips the drive bank.
 * Modes 0..5 and original wrapping aliases 32768..32773 form that full bank;
 * 1..4 apply decoded axes, 2 then selects/refreshed view before weapon control.
 * Saved weapon byte 0/2/4/6/8 selects no-op/left/right/raise/lower. Existing
 * child owners retain selector, adaptive controls and target cancellation.
 * All pointers are required. Unselected calls ignore actor fields/mode/clock
 * and preserve actor/controls; every success writes explicit display events.
 * Invalid used state preserves complete actor, controls and events, even when
 * axes/view would otherwise succeed before a failing weapon operation.
 * No devices, RNG, allocation, phase advancement or class/world scheduling. */
int fist_driver_apply_manual(fist_vehicle_state *vehicle, const fist_manual_input *input,
                             fist_manual_controls *controls, fist_manual_events *events);

/* Manual driving stage: actions, altitude-byte transfer, gun/recoil prefix,
 * movement/manual turret, phase +2, reload/history/maintenance dispatch and ground contact.
 * Caller owns elapsed time and roster participation. Firing/AI/damage remain
 * separate. Invalid input preserves both vehicle and weapon events. */
int fist_driver_step(fist_vehicle_state *vehicle, const fist_klc_image *height,
                     const fist_driver_controls *controls, fist_weapon_events *events);

/* Complete f69:aae4 decoded-axis consumer, without input-device reads.
 * Steering outside (-24,24) sets requested heading from current heading with
 * word wrap; inside it retains the request. Throttle admits >=24 or <-24,
 * doubles the negated axis and clamps to [-240,284]. Negative current speed
 * always applies profile 3, otherwise modes above 1 apply profile 0; reuse the
 * existing component/display owner. No clock, target, RNG, allocation or phase
 * advancement. Every success writes the refresh event, including no refresh.
 * Invalid used actor/output preserves both completely. */
int fist_driver_apply_axes(fist_vehicle_state *vehicle, fist_drive_control_events *events);

/* Original aae8 control refresh, also used after a player weapon selection. */
int fist_driver_take_control(fist_vehicle_state *vehicle);

#endif
