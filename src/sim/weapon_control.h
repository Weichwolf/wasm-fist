#ifndef FIST_SIM_WEAPON_CONTROL_H
#define FIST_SIM_WEAPON_CONTROL_H

#include "sim/vehicle_state.h"

#include <stdint.h>

enum { FIST_WEAPON_NO_REQUEST = 255 };

typedef struct {
    uint8_t station_changed;
    uint8_t timer_expired;
    /* Requests at the class-method boundary. Voice playback has separate
     * player/side/mute/time gates; these IDs do not promise audible output. */
    uint8_t voice_request;
    uint8_t notice_request;
    uint16_t notice_ticks;
} fist_weapon_events;

/* Recover complete original station setters for all four ground classes.
 * station is the original even code, including T80 code 8. Re-selecting the
 * current station is a complete no-op. Invalid input preserves state/events. */
int fist_weapon_select(fist_vehicle_state *vehicle, uint8_t station, fist_weapon_events *events);

/* Original class cycle traverses codes 0,2,4,6 on every class. */
int fist_weapon_cycle(fist_vehicle_state *vehicle, fist_weapon_events *events);

/* The original fire command a286 sets the pending request to 48. This function
 * does not run eligibility/spawn/flight or consume ammunition. */
int fist_weapon_request_fire(fist_vehicle_state *vehicle);

/* Class-entry stage, before motion/phase advancement: decay recoil and copy
 * the gun elevation's high byte into the authored pose selector. */
int fist_weapon_begin_tick(fist_vehicle_state *vehicle);

/* Original reload method, dispatched only when current phase & 1e is zero.
 * Caller owns phase advancement. No ammunition is created/consumed here. */
int fist_weapon_reload_phase(fist_vehicle_state *vehicle, fist_weapon_events *events);

#endif
