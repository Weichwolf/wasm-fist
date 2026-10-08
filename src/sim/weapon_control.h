#ifndef FIST_SIM_WEAPON_CONTROL_H
#define FIST_SIM_WEAPON_CONTROL_H

#include "sim/vehicle_state.h"

#include <stdint.h>

enum { FIST_WEAPON_NO_REQUEST = 255, FIST_WEAPON_MAX_STATIONS = 5 };

typedef struct {
    uint8_t station_changed;
    uint8_t timer_expired;
    /* Requests at the class-method boundary. Voice playback has separate
     * player/side/mute/time gates; these IDs do not promise audible output. */
    uint8_t voice_request;
    uint8_t notice_request;
    uint16_t notice_ticks;
} fist_weapon_events;

typedef struct {
    uint16_t ammunition;
    uint8_t station_count;
    uint8_t selected;
    uint8_t countdown;
    uint8_t continuous;
    uint8_t reserve;
    uint8_t has_reserve;
} fist_weapon_status;

typedef struct {
    /* Shared original 9600/9602/9604 controls, not per-actor counters.
     * The canonical caller supplies the wrapping original 0452 clock word. */
    uint16_t previous_clock;
    uint16_t step;
    uint16_t held_step;
} fist_weapon_elevation_controls;

typedef enum {
    FIST_WEAPON_ELEVATION_PHASE,
    FIST_WEAPON_ELEVATION_RAISE,
    FIST_WEAPON_ELEVATION_LOWER,
    FIST_WEAPON_ELEVATION_QUICK_LOWER,
    FIST_WEAPON_ELEVATION_CENTER
} fist_weapon_elevation_action;

typedef struct {
    /* Shared original 9746 selector, retained across controlled actors. */
    uint16_t selector;
} fist_weapon_turret_controls;

typedef enum {
    FIST_WEAPON_TURRET_INVALID = -1,
    FIST_WEAPON_TURRET_LEFT,
    FIST_WEAPON_TURRET_RIGHT,
    FIST_WEAPON_TURRET_DIRECTION_COUNT
} fist_weapon_turret_direction;

/* Complete a376/a3a8. Reuse control refresh before target/elevation cancellation.
 * The wrapped selector chooses the complete original curve; view 1 uses the
 * full step, other views halve it and views above 3 halve it again. Adjust the
 * requested relative turret offset with word wrap and no clamp. The selector
 * is unchanged. No target resolution, clock, RNG, allocation or phase advance.
 * Only LEFT/RIGHT are accepted; INVALID/COUNT are rejection markers.
 * Invalid used actor/direction/control/target preserves the complete actor. */
int fist_weapon_turn_turret(fist_vehicle_state *vehicle, fist_weapon_turret_direction direction,
                            const fist_weapon_turret_controls *controls);

/* Complete a59c/a5a6 callers set the shared selector to 88/232 respectively
 * before the same helper. Failure preserves both actor and shared controls. */
int fist_weapon_turn_turret_input(fist_vehicle_state *vehicle,
                                  fist_weapon_turret_direction direction,
                                  fist_weapon_turret_controls *controls);

/* Complete a202/a1e9/a26d/a265/a25b. Phase admission uses motion bits 20/40,
 * with raise taking precedence, and restores the shared adaptive step after
 * temporarily using held_step. Manual raise/lower retain step acceleration;
 * quick lower sets 728 without updating the clock; center clears the requested
 * relative turret offset. An existing target is cleared before adjustment,
 * resetting elevation only when a target was present. Word wrap precedes the
 * directional signed clamp. No phase advancement, allocation, RNG or devices.
 * Malformed used runtime targets fail; an inhibited phase does not use them.
 * Failure preserves the complete actor and shared controls. */
int fist_weapon_adjust_elevation(fist_vehicle_state *vehicle, fist_weapon_elevation_action action,
                                 fist_weapon_elevation_controls *controls, uint16_t clock);

/* Read the selected store and mechanical timer, not firing eligibility. */
int fist_weapon_inspect(const fist_vehicle_state *vehicle, fist_weapon_status *out);

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
