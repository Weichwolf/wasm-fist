#include "sim/vehicle_state.h"

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
        MOVEMENT_GATE = 93
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
                             .update_phase = snapshot[UPDATE_PHASE]};
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
        FIRST_PHASE = 109,
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
    fist_vehicle_state vehicle = {.type = type,
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
                                  .random_phases = {raw[FIRST_PHASE], raw[SECOND_PHASE]},
                                  .control_mode = raw[CONTROL_MODE],
                                  .turret_view_mode = raw[TURRET_VIEW],
                                  .hull_view_mode = raw[HULL_VIEW],
                                  .behavior = raw[BEHAVIOR],
                                  .behavior_flags = raw[BEHAVIOR_FLAGS],
                                  .reload_countdown = raw[RELOAD],
                                  .component_size = defaults[type].component_size};
    restore_motion(raw, &vehicle);
    restore_weapon_control(raw, &vehicle);
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
