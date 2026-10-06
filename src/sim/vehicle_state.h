#ifndef FIST_SIM_VEHICLE_STATE_H
#define FIST_SIM_VEHICLE_STATE_H

#include "assets/units.h"
#include "sim/random.h"

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_VEHICLE_WEAPON_SLOTS = 4,
    FIST_VEHICLE_COMPONENT_BYTES = 62,
    FIST_VEHICLE_ANIMATION_SELECTORS = 3
};

typedef struct {
    int16_t speed;
    int16_t throttle;
    int16_t terrain_pitch;
    int16_t terrain_roll;
    int16_t velocity_x;
    int16_t velocity_y;
    uint16_t heading;
    uint16_t requested_heading;
    uint16_t movement_gate;
    uint8_t motion_flags;
    uint8_t update_phase;
} fist_vehicle_drive;

typedef struct {
    uint16_t heading;
    uint16_t offset;
    uint16_t requested_offset;
    int16_t terrain_roll;
    int16_t terrain_pitch;
    int16_t elevation;
    uint8_t elevation_frame;
} fist_vehicle_turret;

typedef struct {
    /* Original class weapon-slot order; identities and fire/reload behavior
     * are decoded with the firing rules, not inferred from ammunition size. */
    uint16_t rounds[FIST_VEHICLE_WEAPON_SLOTS];
    /* T80's fifth station ammunition is a word. Other classes retain their
     * distinct trailing parameter until its additional weapon rules are owned. */
    uint16_t class_parameter;
    uint8_t cycle[2];
    uint8_t ready_stock;
    /* Original station codes are 0, 2, 4, 6; T80 also has station 8. */
    uint8_t selected;
    uint8_t loaded;
    uint8_t trigger;
    uint8_t recoil;
} fist_vehicle_weapons;

typedef struct {
    uint16_t type;
    uint16_t registry_index;
    uint16_t generation;
    int32_t map_x;
    int32_t map_y;
    int32_t altitude;
    uint8_t ground_height;
    uint8_t platoon;
    uint8_t member;
    fist_vehicle_drive drive;
    fist_vehicle_turret turret;
    fist_vehicle_weapons weapons;
    uint16_t projection_extent;
    uint16_t projection_scale;
    uint16_t camera_height;
    uint16_t control_flags;
    uint8_t object_flags;
    uint8_t secondary_flags;
    uint8_t operating_flags;
    uint8_t random_phases[2];
    uint8_t control_mode;
    uint8_t turret_view_mode;
    uint8_t hull_view_mode;
    uint8_t behavior;
    uint8_t reload_countdown;
    uint8_t damage;
    uint8_t damage_alarm_countdown;
    /* Original +a9/+aa part selectors and +ab companion animation byte. */
    uint8_t animation_selectors[FIST_VEHICLE_ANIMATION_SELECTORS];
    /* Complete original initialized component payload. Owned format data;
     * individual component damage/animation semantics remain to be decoded. */
    size_t component_size;
    uint8_t components[FIST_VEHICLE_COMPONENT_BYTES];
} fist_vehicle_state;

/* Execute c296 + the complete original ground-class start defaults in typed C.
 * Borrows the definition during the call; no pointers/source bytes survive.
 * The caller supplies original random state and the 6dae link-mode byte.
 * This is the initialization stage, not terrain/suspension installation or a
 * full mission tick. Return 0 on success, -1 on invalid input, preserving out
 * and random on failure. No allocation is needed. */
int fist_vehicle_initialize(const fist_unit_definition *definition, fist_random *random,
                            uint8_t link_mode, fist_vehicle_state *out);

/* Complete original component payload size; 0 for unsupported classes. */
size_t fist_vehicle_component_size(uint16_t type);

#endif
