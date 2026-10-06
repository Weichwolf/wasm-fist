#include "sim/vehicle_motion.h"

#include "assets/units.h"
#include "sim/rotation.h"
#include "sim/vehicle_state.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    DRIVE_PROFILES = 4,
    SLOPE_KNOTS = 32,
    SLOPE_INTERVAL = 512,
    SLOPE_CENTER = 16,
    THROTTLE_DIVISOR = 4,
    PHASE_GATE = 3,
    DIRECTION_PHASE_GATE = 12,
    DIRECTION_FLAGS = 6,
    IMMOBILE_FLAG = 16,
    RECENTER_FLAG = 128,
    STOPPED_CONTROL_FLAG = 16,
    HULL_SLEW = 182,
    TURRET_SLEW = 364,
    RECENTER_SLEW = 910,
    WORD_RANGE = 65536,
    MOTION_TURN_FACTOR = 8,
    RIGHT_MOTION_FLAG = 4,
    COMPONENT_REFRESH = 3,
    DEFAULT_PART_SELECTOR = 128
};

/* Original STR:2b84 points to these four complete 32-byte slope profiles. */
static const uint8_t speed_limits[DRIVE_PROFILES][SLOPE_KNOTS] = {
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     64, 61, 58, 54, 50, 45, 32, 24, 16, 8,  0,  0,  0,  0,  0,  0},
    {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
     32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 28, 24, 20, 16, 12, 8},
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
    {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
     32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32}};

typedef struct {
    size_t speed;
    size_t heading;
    size_t heading_companion;
} refresh_offsets;

/* Component-local indices from 7cd5/88e4/912d/98c3 and their turret twins.
 * The fourth class marks two components. Its companion is otherwise unused. */
static const refresh_offsets refresh[FIST_UNIT_GROUND_VEHICLE_COUNT] = {
    {.speed = 12, .heading = 11},
    {.speed = 9, .heading = 48},
    {.speed = 11, .heading = 10},
    {.speed = 8, .heading = 18, .heading_companion = 21}};

static int32_t signed_word(uint16_t value) {
    return value <= INT16_MAX ? value : (int32_t)value - WORD_RANGE;
}

static int32_t floor_divide(int32_t value, int32_t divisor) {
    const int32_t quotient = value / divisor;
    return quotient - (value < 0 && value % divisor != 0 ? 1 : 0);
}

static int32_t limit_step(int32_t value, int32_t maximum) {
    if (value > maximum) {
        return maximum;
    }
    return value < -maximum ? -maximum : value;
}

static bool speed_step(fist_vehicle_state *vehicle) {
    fist_vehicle_drive *drive = &vehicle->drive;
    if ((drive->update_phase & PHASE_GATE) != 0) {
        return false;
    }
    const unsigned direction = drive->motion_flags & DIRECTION_FLAGS;
    if ((drive->motion_flags & IMMOBILE_FLAG) != 0 || direction == DIRECTION_FLAGS ||
        drive->movement_gate == 0) {
        drive->throttle = 0;
    }
    int32_t slope = floor_divide(drive->terrain_pitch, SLOPE_INTERVAL) + SLOPE_CENTER;
    if (slope < 0) {
        slope = 0;
    } else if (slope >= SLOPE_KNOTS) {
        slope = SLOPE_KNOTS - 1;
    }
    const int32_t limit = speed_limits[vehicle->control_mode % DRIVE_PROFILES][slope];
    int32_t target = floor_divide(drive->throttle, THROTTLE_DIVISOR);
    if (target > limit) {
        target = limit;
    }
    const int32_t speed = drive->speed;
    if (target == speed) {
        return false;
    }
    const int32_t step = target > speed ? 1 : -1;
    if (direction != 0 && (drive->update_phase & DIRECTION_PHASE_GATE) != 0 &&
        ((step > 0 && speed >= 0) || (step < 0 && speed < 0))) {
        return false;
    }
    drive->speed = (int16_t)(speed + step);
    if (drive->speed != 0) {
        vehicle->control_flags &= (uint16_t)~STOPPED_CONTROL_FLAG;
    }
    return true;
}

static bool hull_step(fist_vehicle_state *vehicle) {
    fist_vehicle_drive *drive = &vehicle->drive;
    fist_vehicle_turret *turret = &vehicle->turret;
    if ((drive->motion_flags & (DIRECTION_FLAGS | IMMOBILE_FLAG)) != 0 ||
        drive->movement_gate == 0) {
        return false;
    }
    if ((vehicle->operating_flags & RECENTER_FLAG) == 0) {
        const int32_t difference =
            signed_word((uint16_t)(drive->requested_heading - drive->heading));
        const int32_t step = limit_step(difference, HULL_SLEW);
        drive->heading = (uint16_t)(drive->heading + step);
        return step != 0;
    }
    const int32_t offset = signed_word(turret->offset);
    const int32_t step = limit_step(offset, RECENTER_SLEW);
    if (step == offset) {
        vehicle->operating_flags &= (uint8_t)~RECENTER_FLAG;
    }
    drive->heading = (uint16_t)(drive->heading + step);
    drive->requested_heading = (uint16_t)(drive->requested_heading + step);
    turret->offset = (uint16_t)(turret->offset - step);
    turret->requested_offset = (uint16_t)(turret->requested_offset - step);
    return true;
}

static int32_t add_velocity(int32_t position, int16_t velocity) {
    const int64_t sum = (int64_t)position + velocity;
    const int64_t range = (int64_t)UINT32_MAX + 1;
    if (sum > INT32_MAX) {
        return (int32_t)(sum - range);
    }
    return sum < INT32_MIN ? (int32_t)(sum + range) : (int32_t)sum;
}

static void integrate(fist_vehicle_state *vehicle) {
    fist_vehicle_drive *drive = &vehicle->drive;
    const int16_t magnitude = (int16_t)floor_divide(drive->speed, 2);
    const unsigned direction = drive->motion_flags & DIRECTION_FLAGS;
    if (direction != 0 && direction != DIRECTION_FLAGS) {
        const int32_t sign = (drive->motion_flags & RIGHT_MOTION_FLAG) != 0 ? 1 : -1;
        drive->heading = (uint16_t)(drive->heading + (magnitude * MOTION_TURN_FACTOR * sign));
        drive->requested_heading = drive->heading;
    }
    const fist_velocity velocity =
        fist_rotate((fist_rotation){.heading = drive->heading, .magnitude = magnitude});
    drive->velocity_x = velocity.x;
    drive->velocity_y = velocity.y;
    vehicle->map_x = add_velocity(vehicle->map_x, velocity.x);
    vehicle->map_y = add_velocity(vehicle->map_y, velocity.y);
}

static bool turret_step(fist_vehicle_state *vehicle) {
    fist_vehicle_turret *turret = &vehicle->turret;
    const int32_t difference = signed_word((uint16_t)(turret->requested_offset - turret->offset));
    const int32_t step = limit_step(difference, TURRET_SLEW);
    turret->offset = (uint16_t)(turret->offset + step);
    turret->heading = (uint16_t)(vehicle->drive.heading + turret->offset);
    vehicle->animation_selectors[0] = DEFAULT_PART_SELECTOR;
    vehicle->animation_selectors[1] = 0;
    vehicle->animation_selectors[2] = 0;
    return step != 0;
}

static void mark_heading(fist_vehicle_state *vehicle) {
    const refresh_offsets *offsets = &refresh[vehicle->type];
    vehicle->components[offsets->heading] = COMPONENT_REFRESH;
    if (vehicle->type == FIST_UNIT_GROUND_VEHICLE_COUNT - 1) {
        vehicle->components[offsets->heading_companion] = COMPONENT_REFRESH;
    }
}

int fist_vehicle_motion_step(fist_vehicle_state *vehicle, fist_vehicle_motion_events *out) {
    if (vehicle == NULL || out == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    fist_vehicle_motion_events events = {0};
    events.speed_changed = (uint8_t)speed_step(vehicle);
    if (events.speed_changed != 0) {
        vehicle->components[refresh[vehicle->type].speed] = COMPONENT_REFRESH;
    }
    events.hull_refreshed = (uint8_t)hull_step(vehicle);
    if (events.hull_refreshed != 0) {
        mark_heading(vehicle);
    }
    integrate(vehicle);
    events.turret_changed = (uint8_t)turret_step(vehicle);
    if (events.turret_changed != 0) {
        mark_heading(vehicle);
    }
    *out = events;
    return 0;
}
