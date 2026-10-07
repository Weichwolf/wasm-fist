#include "sim/vehicle_state.h"
#include "sim/object_pool.h"

#include "assets/bytes.h"
#include "assets/units.h"
#include "sim/random.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t component_size;
    uint16_t camera_height;
    fist_vehicle_weapons weapons;
    uint8_t opposing_side;
    uint8_t components[FIST_VEHICLE_COMPONENT_BYTES];
} vehicle_defaults;

/* Original 7b91/8744/8f9f/973a and their 1e27 component copies. These are
 * recovered gameplay/format constants, not generated engine instructions. */
static const vehicle_defaults defaults[FIST_UNIT_GROUND_VEHICLE_COUNT] = {
    {.weapons = {.rounds = {15, 20, 2000, 5}, .class_parameter = 20},
     .camera_height = 2048,
     .component_size = 57,
     .components = {0, 255, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2,
                    0, 4,   0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 0, 6, 0,
                    0, 0,   2,  0, 4, 0, 6, 0, 0, 1, 0, 2, 0, 6, 0, 0, 0, 0, 0}},
    {.weapons =
         {.rounds = {2, 500, 400, 2000}, .class_parameter = 20, .cycle = {1, 1}, .ready_stock = 10},
     .camera_height = 2560,
     .component_size = 62,
     .components = {0, 255, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    2, 0,   4,  0, 6, 0, 0, 0, 2, 0, 4, 0, 6, 0, 0, 0, 0, 0, 1, 0, 0,
                    0, 1,   0,  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 6, 0, 0, 0, 0}},
    {.weapons = {.rounds = {15, 16, 5, 2000}, .class_parameter = 20},
     .camera_height = 2048,
     .opposing_side = 1,
     .component_size = 59,
     .components = {0, 255, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0,   0,  2, 0, 4, 0, 6, 0, 8, 0, 0, 0, 2, 0, 4, 0, 6, 0, 8,
                    0, 0,   1,  0, 2, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {.weapons =
         {.rounds = {300, 400, 4, 2000}, .class_parameter = 20, .cycle = {1, 1}, .ready_stock = 12},
     .camera_height = 1920,
     .opposing_side = 1,
     .component_size = 61,
     .components = {0, 255, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
                    0, 0,   0,  0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 0, 6, 0, 0, 0, 2, 0, 4,
                    0, 6,   0,  0, 0, 0, 0, 0, 1, 0, 2, 0, 6, 0, 0, 0, 0, 0, 0}}};

static void restore_motion(const uint8_t *snapshot, fist_vehicle_state *vehicle) {
    enum {
        TURRET_HEADING = 16,
        MOTION_FLAGS = 25,
        HULL_HEADING = 38,
        REQUESTED_HEADING = 48,
        TERRAIN_PITCH = 52,
        TERRAIN_ROLL = 50,
        TURRET_ROLL = 34,
        TURRET_PITCH = 36,
        GUN_ELEVATION = 56,
        ELEVATION_FRAME = 167,
        GROUND_HEIGHT = 29,
        PLATOON = 27,
        MEMBER = 28,
        UPDATE_PHASE = 61,
        DAMAGE = 58,
        DAMAGE_ALARM = 149,
        SPEED = 85,
        THROTTLE = 87,
        VELOCITY_X = 89,
        VELOCITY_Y = 91,
        TURRET_OFFSET = 137,
        REQUESTED_OFFSET = 139,
        MOVEMENT_GATE = 93,
        SPEED_COUNTER = 95
    };
    vehicle->drive =
        (fist_vehicle_drive){.speed = fist_read_i16le(snapshot + SPEED),
                             .throttle = fist_read_i16le(snapshot + THROTTLE),
                             .terrain_roll = fist_read_i16le(snapshot + TERRAIN_ROLL),
                             .terrain_pitch = fist_read_i16le(snapshot + TERRAIN_PITCH),
                             .velocity_x = fist_read_i16le(snapshot + VELOCITY_X),
                             .velocity_y = fist_read_i16le(snapshot + VELOCITY_Y),
                             .heading = fist_read_u16le(snapshot + HULL_HEADING),
                             .requested_heading = fist_read_u16le(snapshot + REQUESTED_HEADING),
                             .movement_gate = fist_read_u16le(snapshot + MOVEMENT_GATE),
                             .motion_flags = snapshot[MOTION_FLAGS],
                             .update_phase = snapshot[UPDATE_PHASE],
                             .speed_counter = snapshot[SPEED_COUNTER]};
    vehicle->ground_height = snapshot[GROUND_HEIGHT];
    vehicle->platoon = snapshot[PLATOON];
    vehicle->member = snapshot[MEMBER];
    vehicle->damage = snapshot[DAMAGE];
    vehicle->damage_alarm_countdown = snapshot[DAMAGE_ALARM];
    vehicle->turret =
        (fist_vehicle_turret){.heading = fist_read_u16le(snapshot + TURRET_HEADING),
                              .offset = fist_read_u16le(snapshot + TURRET_OFFSET),
                              .requested_offset = fist_read_u16le(snapshot + REQUESTED_OFFSET),
                              .terrain_roll = fist_read_i16le(snapshot + TURRET_ROLL),
                              .terrain_pitch = fist_read_i16le(snapshot + TURRET_PITCH),
                              .elevation = fist_read_i16le(snapshot + GUN_ELEVATION),
                              .elevation_frame = snapshot[ELEVATION_FRAME]};
}

static void restore_weapon_control(const uint8_t *snapshot, fist_vehicle_state *vehicle) {
    enum { SELECTED = 145, LOADED = 165, TRIGGER = 146, RECOIL = 60 };
    vehicle->weapons.selected = snapshot[SELECTED];
    vehicle->weapons.loaded = snapshot[LOADED];
    vehicle->weapons.trigger = snapshot[TRIGGER];
    vehicle->weapons.recoil = snapshot[RECOIL];
}

int fist_vehicle_restore(const fist_unit_definition *definition, fist_vehicle_state *out) {
    enum {
        EXTENT = 18,
        SCALE = 20,
        OBJECT_FLAGS = 22,
        SECONDARY_FLAGS = 23,
        OPERATING_FLAGS = 26,
        BEHAVIOR = 62,
        BEHAVIOR_FLAGS = 99,
        CONTROL_FLAGS = 64,
        SECOND_PHASE = 66,
        COMMAND_MODE = 67,
        RETREAT_COUNT = 68,
        MANEUVER = 69,
        MANEUVER_COUNT = 70,
        MANEUVER_HEADING = 71,
        BLOCKED_COUNT = 81,
        TARGET_REFERENCE = 151,
        TARGET_RANGE = 153,
        TARGET_HEADING = 155,
        CANDIDATE_REFERENCE = 157,
        SECONDARY_HEADING = 142,
        DISCOVERY_COUNT = 148,
        RESET_STATE = 54,
        GOAL = 73,
        RANGE = 83,
        HEADING_HISTORY = 40,
        HEADING_AVERAGE = 46,
        FIRST_PHASE = 109,
        POSITION_HISTORY = 110,
        TURRET_VIEW = 134,
        CAMERA_HEIGHT = 135,
        HULL_VIEW = 141,
        CONTROL_MODE = 144,
        RELOAD = 168,
        SELECTORS = 169,
        T80_ROUNDS = 172,
        OTHER_ROUNDS = 173,
        CYCLES = 181,
        STOCK = 187,
        CLASS_PARAMETER = 180
    };
    static const size_t component_offsets[] = {191, 188, 190, 188};
    static const size_t parameter_offsets[] = {181, 250, CLASS_PARAMETER, 249};
    if (definition == NULL || out == NULL || definition->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        definition->snapshot.data == NULL || definition->snapshot.size != FIST_UNIT_EXTENDED_SIZE ||
        fist_read_u16le(definition->snapshot.data) != definition->type) {
        return -1;
    }
    const uint16_t type = definition->type;
    const uint8_t *raw = definition->snapshot.data;
    fist_vehicle_state vehicle = {
        .type = type,
        .registry_index = definition->registry_index,
        .generation = definition->generation,
        .map_x = definition->map_x,
        .map_y = definition->map_y,
        .altitude = definition->altitude,
        .projection_extent = fist_read_u16le(raw + EXTENT),
        .projection_scale = fist_read_u16le(raw + SCALE),
        .camera_height = fist_read_u16le(raw + CAMERA_HEIGHT),
        .control_flags = fist_read_u16le(raw + CONTROL_FLAGS),
        .object_flags = raw[OBJECT_FLAGS],
        .secondary_flags = raw[SECONDARY_FLAGS],
        .operating_flags = raw[OPERATING_FLAGS],
        .reset_state = raw[RESET_STATE],
        .random_phases = {raw[FIRST_PHASE], raw[SECOND_PHASE]},
        .command = {.mode = raw[COMMAND_MODE],
                    .maneuver = raw[MANEUVER],
                    .maneuver_count = raw[MANEUVER_COUNT],
                    .blocked_count = raw[BLOCKED_COUNT],
                    .retreat_count = raw[RETREAT_COUNT],
                    .maneuver_heading = fist_read_u16le(raw + MANEUVER_HEADING),
                    .target_reference = fist_read_u16le(raw + TARGET_REFERENCE),
                    .target_range = fist_read_u16le(raw + TARGET_RANGE),
                    .target_heading = fist_read_u16le(raw + TARGET_HEADING),
                    .candidate_reference = fist_read_u16le(raw + CANDIDATE_REFERENCE),
                    .secondary_heading = fist_read_u16le(raw + SECONDARY_HEADING),
                    .discovery_count = raw[DISCOVERY_COUNT],
                    .goal = {fist_read_i32le(raw + GOAL), fist_read_i32le(raw + GOAL + 4)},
                    .range = fist_read_u16le(raw + RANGE),
                    .heading_average = fist_read_u16le(raw + HEADING_AVERAGE)},
        .control_mode = raw[CONTROL_MODE],
        .turret_view_mode = raw[TURRET_VIEW],
        .hull_view_mode = raw[HULL_VIEW],
        .behavior = raw[BEHAVIOR],
        .behavior_flags = raw[BEHAVIOR_FLAGS],
        .reload_countdown = raw[RELOAD],
        .component_size = defaults[type].component_size};
    restore_motion(raw, &vehicle);
    restore_weapon_control(raw, &vehicle);
    for (size_t index = 0; index < FIST_VEHICLE_HEADING_SAMPLES; ++index) {
        vehicle.command.heading_history[index] =
            fist_read_u16le(raw + HEADING_HISTORY + (index * sizeof(uint16_t)));
    }
    for (size_t index = 0; index < FIST_VEHICLE_POSITION_SAMPLES; ++index) {
        const uint8_t *sample = raw + POSITION_HISTORY + (index * 2 * sizeof(uint16_t));
        vehicle.position_history[index] = (fist_vehicle_position_sample){
            fist_read_u16le(sample), fist_read_u16le(sample + sizeof(uint16_t))};
    }
    const size_t rounds = type == 2 ? T80_ROUNDS : OTHER_ROUNDS;
    for (size_t index = 0; index < FIST_VEHICLE_WEAPON_SLOTS; ++index) {
        vehicle.weapons.rounds[index] = fist_read_u16le(raw + rounds + (index * sizeof(uint16_t)));
    }
    vehicle.weapons.class_parameter =
        type == 2 ? fist_read_u16le(raw + CLASS_PARAMETER) : raw[parameter_offsets[type]];
    if (type == 1 || type == 3) {
        vehicle.weapons.cycle[0] = raw[CYCLES];
        vehicle.weapons.cycle[1] = raw[CYCLES + 1];
        vehicle.weapons.ready_stock = raw[STOCK];
    }
    for (size_t index = 0; index < vehicle.component_size; ++index) {
        vehicle.components[index] = raw[component_offsets[type] + index];
    }
    for (size_t index = 0; index < FIST_VEHICLE_ANIMATION_SELECTORS; ++index) {
        vehicle.animation_selectors[index] = raw[SELECTORS + index];
    }
    *out = vehicle;
    return 0;
}

int fist_vehicle_initialize(const fist_unit_definition *definition, fist_random *random,
                            uint8_t link_mode, fist_vehicle_state *out) {
    enum {
        DEFAULT_OBJECT_FLAGS = 0x66,
        DEFAULT_SECONDARY_FLAGS = 0x34,
        SIDE_FLAG = 8,
        LINK_OPERATING_FLAG = 16,
        LINK_MODE = 2,
        PROJECTION_EXTENT = 40,
        PROJECTION_SCALE = 1024
    };
    if (definition == NULL || random == NULL || out == NULL ||
        definition->type >= FIST_UNIT_GROUND_VEHICLE_COUNT || definition->snapshot.data == NULL ||
        definition->snapshot.size != FIST_UNIT_EXTENDED_SIZE ||
        fist_read_u16le(definition->snapshot.data) != definition->type ||
        random->next_stream >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    const vehicle_defaults *parameters = &defaults[definition->type];
    const uint8_t *snapshot = definition->snapshot.data;
    fist_vehicle_state vehicle = {0};
    if (fist_vehicle_restore(definition, &vehicle) != 0) {
        return -1;
    }
    vehicle.weapons = parameters->weapons;
    vehicle.projection_extent = PROJECTION_EXTENT;
    vehicle.projection_scale = PROJECTION_SCALE;
    vehicle.camera_height = parameters->camera_height;
    vehicle.control_flags |= 1;
    vehicle.object_flags |= DEFAULT_OBJECT_FLAGS;
    vehicle.secondary_flags |= DEFAULT_SECONDARY_FLAGS;
    vehicle.operating_flags = 1;
    vehicle.turret_view_mode = 1;
    vehicle.hull_view_mode = 1;
    vehicle.control_mode = 0;
    vehicle.behavior = 0;
    vehicle.reload_countdown = 0;
    vehicle.drive.movement_gate = UINT16_MAX;
    vehicle.turret.requested_offset = 0;
    restore_weapon_control(snapshot, &vehicle);
    vehicle.object_flags = parameters->opposing_side != 0
                               ? (uint8_t)(vehicle.object_flags | SIDE_FLAG)
                               : (uint8_t)(vehicle.object_flags & (uint8_t)~SIDE_FLAG);
    if (link_mode == LINK_MODE) {
        vehicle.operating_flags |= LINK_OPERATING_FLAG;
    }
    fist_random advanced = *random;
    uint16_t value = 0;
    if (fist_random_next(&advanced, &value) != 0) {
        return -1;
    }
    vehicle.random_phases[0] = (uint8_t)value;
    if (fist_random_next(&advanced, &value) != 0) {
        return -1;
    }
    vehicle.random_phases[1] = (uint8_t)value;
    for (size_t index = 0; index < parameters->component_size; ++index) {
        vehicle.components[index] = parameters->components[index];
    }
    *out = vehicle;
    *random = advanced;
    return 0;
}

size_t fist_vehicle_component_size(uint16_t type) {
    return type < FIST_UNIT_GROUND_VEHICLE_COUNT ? defaults[type].component_size : 0;
}

int fist_vehicle_set_drive_profile(fist_vehicle_state *vehicle, uint8_t profile,
                                   fist_drive_control_events *out) {
    /* Original component destinations +d6/+c8/+ca/+d2, relative to each
     * class's owned component start +bf/+bc/+be/+bc. */
    static const size_t components[FIST_UNIT_GROUND_VEHICLE_COUNT] = {23, 12, 12, 22};
    enum { COMPONENT_REFRESH = 3 };
    if (vehicle == NULL || out == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    vehicle->control_mode = profile;
    vehicle->components[components[vehicle->type]] = COMPONENT_REFRESH;
    *out = (fist_drive_control_events){.refresh_drive_display = true};
    return 0;
}

int fist_vehicle_update_drive_profile(fist_vehicle_state *vehicle, fist_drive_control_events *out) {
    enum { AUTOMATIC_PROFILES = 2, SHIFT_PITCH = 3584 };
    if (vehicle == NULL || out == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    if (vehicle->control_mode < AUTOMATIC_PROFILES) {
        const uint8_t profile = vehicle->drive.terrain_pitch >= SHIFT_PITCH ? 1 : 0;
        if (profile != vehicle->control_mode) {
            return fist_vehicle_set_drive_profile(vehicle, profile, out);
        }
    }
    *out = (fist_drive_control_events){0};
    return 0;
}

int fist_vehicle_prepare(fist_vehicle_state *vehicle, uint8_t link_mode) {
    enum { LINK_FLAG = 16, LINK_MODE = 2, OPERATING_FLAG = 1, AUTOMATIC_FLAG = 1 };
    if (vehicle == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    const vehicle_defaults *parameters = &defaults[vehicle->type];
    vehicle->operating_flags = (uint8_t)((vehicle->operating_flags & ~LINK_FLAG) | OPERATING_FLAG |
                                         (link_mode == LINK_MODE ? LINK_FLAG : 0));
    vehicle->camera_height = parameters->camera_height;
    vehicle->command.target_reference = 0;
    vehicle->command.candidate_reference = 0;
    vehicle->command.target = (fist_object_reference){0};
    vehicle->command.candidate = (fist_object_reference){0};
    vehicle->control_flags |= AUTOMATIC_FLAG;
    vehicle->reset_state = 0;
    for (size_t index = 0; index < parameters->component_size; ++index) {
        vehicle->components[index] = parameters->components[index];
    }
    return 0;
}

int fist_vehicle_history_phase(fist_vehicle_state *vehicle) {
    enum { HISTORY_PHASE = 6, SAMPLE_INTERVAL = 12, COORDINATE_SHIFT = 8 };
    if (vehicle == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    if ((vehicle->drive.update_phase & FIST_VEHICLE_PHASE_MASK) != HISTORY_PHASE) {
        return 0;
    }
    /* aa37 -> f69:b038 (raw 1a6c8..1a721): unsigned byte INC/CMP, then
     * descending word copies. The initialized random byte is the same counter. */
    vehicle->random_phases[0] = (uint8_t)(vehicle->random_phases[0] + 1);
    if (vehicle->random_phases[0] < SAMPLE_INTERVAL) {
        return 0;
    }
    vehicle->random_phases[0] = 0;
    for (size_t index = FIST_VEHICLE_POSITION_SAMPLES - 1; index > 0; --index) {
        vehicle->position_history[index] = vehicle->position_history[index - 1];
    }
    vehicle->position_history[0] = (fist_vehicle_position_sample){
        .x = (uint16_t)((uint32_t)vehicle->map_x >> COORDINATE_SHIFT),
        .y = (uint16_t)((uint32_t)vehicle->map_y >> COORDINATE_SHIFT)};
    return 0;
}

int fist_vehicle_maintenance_phase(fist_vehicle_state *vehicle) {
    enum {
        SPEED_SHIFT = 4,
        HIGH_SPEED = 60,
        COUNTER_LIMIT = 248,
        COMPONENT_PHASE_MASK = 0xe0,
        COMPONENT_REFRESH = 3
    };
    /* Complete 7cbf/88ce/9160/98fb wrappers around f69:a96c. Component
     * indices follow each original class's owned template, in write order. */
    static const struct {
        uint8_t phase;
        uint8_t components[2];
    } profiles[FIST_UNIT_GROUND_VEHICLE_COUNT] = {
        {10, {13, 14}}, {10, {10, 11}}, {16, {51, 50}}, {20, {60, 59}}};
    if (vehicle == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    if ((vehicle->drive.update_phase & FIST_VEHICLE_PHASE_MASK) != profiles[vehicle->type].phase) {
        return 0;
    }
    int32_t magnitude = vehicle->drive.speed;
    if (magnitude < 0) {
        magnitude = -magnitude;
    }
    /* 19ffc..1a02c: NEG keeps the 8000h magnitude, SHR is unsigned,
     * and SUB carry saturates the movement word to zero. */
    const uint16_t consumed = (uint16_t)((uint32_t)magnitude >> SPEED_SHIFT);
    vehicle->drive.movement_gate = consumed > vehicle->drive.movement_gate
                                       ? 0
                                       : (uint16_t)(vehicle->drive.movement_gate - consumed);
    if (vehicle->drive.speed >= HIGH_SPEED) {
        if (vehicle->drive.speed_counter < COUNTER_LIMIT) {
            ++vehicle->drive.speed_counter;
        }
    } else if (vehicle->drive.speed_counter != 0) {
        --vehicle->drive.speed_counter;
    }
    if ((vehicle->drive.update_phase & COMPONENT_PHASE_MASK) == 0) {
        for (size_t index = 0; index < sizeof(profiles[vehicle->type].components); ++index) {
            vehicle->components[profiles[vehicle->type].components[index]] = COMPONENT_REFRESH;
        }
    }
    return 0;
}
