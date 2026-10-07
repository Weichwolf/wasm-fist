#ifndef FIST_SIM_VEHICLE_STATE_H
#define FIST_SIM_VEHICLE_STATE_H

#include "assets/orders.h"
#include "assets/units.h"
#include "sim/random.h"

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_VEHICLE_WEAPON_SLOTS = 4,
    FIST_VEHICLE_COMPONENT_BYTES = 62,
    FIST_VEHICLE_ANIMATION_SELECTORS = 3,
    FIST_VEHICLE_POSITION_SAMPLES = 6,
    FIST_VEHICLE_HEADING_SAMPLES = 3,
    FIST_VEHICLE_PHASE_MASK = 0x1e
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
    /* Original +5d: consumed movement word, with zero gating motion. */
    uint16_t movement_gate;
    uint8_t motion_flags;
    uint8_t update_phase;
    /* Original +5f speed-threshold counter, without inferred physical units. */
    uint8_t speed_counter;
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
    /* Original middle coordinate words, retaining bits 8..23. */
    uint16_t x;
    uint16_t y;
} fist_vehicle_position_sample;

typedef struct {
    /* Original +43/+45 selectors, retained across class initialization. */
    uint8_t mode;
    uint8_t maneuver;
    /* Original +97 saved reference. Mode selection tests presence only;
     * target discovery/resolution must not treat this word as a C pointer. */
    uint16_t target_reference;
    /* Original +49/+4d navigation goal. Validity remains control bit 2. */
    fist_order_waypoint goal;
    /* Original +53 unsigned navigation range; the bearing callback produces it
     * and route/throttle callbacks consume it. No inferred physical unit. */
    uint16_t range;
    /* Original +28/+2a/+2c newest-first samples and +2e average. The parent
     * heading phase owns future sampling; restoration retains all words. */
    uint16_t heading_history[FIST_VEHICLE_HEADING_SAMPLES];
    uint16_t heading_average;
} fist_vehicle_command;

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
    fist_vehicle_command command;
    uint16_t projection_extent;
    uint16_t projection_scale;
    uint16_t camera_height;
    uint16_t control_flags;
    uint8_t object_flags;
    uint8_t secondary_flags;
    uint8_t operating_flags;
    uint8_t random_phases[2];
    /* Newest first; +6d in random_phases[0] is the shared sampling counter. */
    fist_vehicle_position_sample position_history[FIST_VEHICLE_POSITION_SAMPLES];
    uint8_t control_mode;
    uint8_t turret_view_mode;
    uint8_t hull_view_mode;
    uint8_t behavior;
    /* Original +63, separate from the behavior selector at +3e. */
    uint8_t behavior_flags;
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

/* Restore modeled ground fields from a complete saved snapshot without class
 * initialization or RNG consumption. No input views survive; failure preserves
 * out. The mission loader calls initialization only for participating actors. */
int fist_vehicle_restore(const fist_unit_definition *definition, fist_vehicle_state *out);

/* Execute c296 + the complete original ground-class start defaults in typed C.
 * Borrows the definition during the call; no pointers/source bytes survive.
 * The caller supplies original random state and the 6dae link-mode byte.
 * This is the initialization stage, not terrain/suspension installation or a
 * full mission tick. Return 0 on success, -1 on invalid input, preserving out
 * and random on failure. No allocation is needed. */
int fist_vehicle_initialize(const fist_unit_definition *definition, fist_random *random,
                            uint8_t link_mode, fist_vehicle_state *out);

/* Execute the position-history callback when the current class phase selects
 * it. Does not advance the class phase or execute other callbacks. The original
 * byte counter wraps, and every twelfth call inserts the current position.
 * Invalid ground type/component size preserves the complete vehicle. */
int fist_vehicle_history_phase(fist_vehicle_state *vehicle);

/* Execute the current class-selected movement maintenance phase: consume the
 * movement word from absolute speed, update the saved speed counter and refresh
 * the class components. No phase advancement or other callbacks. Invalid
 * ground type/component size preserves the complete vehicle. */
int fist_vehicle_maintenance_phase(fist_vehicle_state *vehicle);

/* Complete original component payload size; 0 for unsupported classes. */
size_t fist_vehicle_component_size(uint16_t type);

#endif
