#include "sim/driver.h"

#include "assets/klc.h"
#include "assets/units.h"
#include "sim/ground.h"
#include "sim/vehicle_motion.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum { CONTROL_REFRESH_FLAG = 1, COMPONENT_REFRESH = 3, CLASS_PHASE_STEP = 2 };

static void take_control(fist_vehicle_state *vehicle) {
    /* Complete 784d/7faf/8cc8/9407 methods selected by aae8/995a. */
    static const uint8_t component_indices[FIST_UNIT_GROUND_VEHICLE_COUNT] = {8, 8, 40, 27};
    if ((vehicle->control_flags & CONTROL_REFRESH_FLAG) != 0) {
        vehicle->control_flags &= (uint16_t)~CONTROL_REFRESH_FLAG;
        vehicle->components[component_indices[vehicle->type]] = COMPONENT_REFRESH;
    }
}

int fist_driver_take_control(fist_vehicle_state *vehicle) {
    if (vehicle == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    take_control(vehicle);
    return 0;
}

int fist_driver_apply_axes(fist_vehicle_state *vehicle, fist_drive_control_events *events) {
    enum {
        DEAD_ZONE = 24,
        DEMAND_SCALE = 2,
        MINIMUM_DEMAND = -240,
        MAXIMUM_DEMAND = 284,
        AUTOMATIC_PROFILES = 2,
        REVERSE_PROFILE = 3
    };
    if (vehicle == NULL || events == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    fist_vehicle_state next = *vehicle;
    const int32_t steering = (int32_t)next.axes.steering;
    if (steering >= DEAD_ZONE || steering <= -DEAD_ZONE) {
        next.drive.requested_heading = (uint16_t)((int32_t)next.drive.heading + steering);
    }
    const int32_t pedal = (int32_t)next.axes.throttle;
    int32_t demand = pedal >= DEAD_ZONE || pedal < -DEAD_ZONE ? -(DEMAND_SCALE * pedal) : 0;
    if (demand < MINIMUM_DEMAND) {
        demand = MINIMUM_DEMAND;
    } else if (demand > MAXIMUM_DEMAND) {
        demand = MAXIMUM_DEMAND;
    }
    next.drive.throttle = (int16_t)demand;
    fist_drive_control_events emitted = {0};
    if (next.drive.speed < 0 || next.control_mode >= AUTOMATIC_PROFILES) {
        const uint8_t profile = next.drive.speed < 0 ? REVERSE_PROFILE : 0;
        if (fist_vehicle_set_drive_profile(&next, profile, &emitted) != 0) {
            return -1;
        }
    }
    *vehicle = next;
    *events = emitted;
    return 0;
}

static void apply_controls(fist_vehicle_state *vehicle, const fist_driver_controls *controls) {
    if (controls->throttle_change != 0 || controls->steering != 0 || controls->turret_change != 0) {
        take_control(vehicle);
    }
    if (controls->throttle_change > 0 && vehicle->drive.throttle < FIST_DRIVER_THROTTLE_LIMIT) {
        ++vehicle->drive.throttle;
    } else if (controls->throttle_change < 0 &&
               vehicle->drive.throttle > -FIST_DRIVER_THROTTLE_LIMIT) {
        --vehicle->drive.throttle;
    }
    if (controls->throttle_off != 0) {
        vehicle->drive.throttle = 0;
    }
    vehicle->drive.requested_heading =
        (uint16_t)((int)vehicle->drive.requested_heading +
                   (controls->steering * FIST_DRIVER_TURN_INCREMENT));
    vehicle->turret.requested_offset =
        (uint16_t)((int)vehicle->turret.requested_offset + controls->turret_change);
}

static void transfer_altitude(fist_vehicle_state *vehicle) {
    /* Original class entry MOV byte +1dh -> byte +0dh, preserving other lanes. */
    vehicle->altitude = fist_altitude_set_height(vehicle->altitude, vehicle->ground_height);
}

int fist_driver_step(fist_vehicle_state *vehicle, const fist_klc_image *height,
                     const fist_driver_controls *controls, fist_weapon_events *events) {
    if (vehicle == NULL || controls == NULL || events == NULL || controls->throttle_change < -1 ||
        controls->throttle_change > 1 || controls->steering < -1 || controls->steering > 1 ||
        controls->throttle_off > 1 || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    fist_vehicle_state next = *vehicle;
    apply_controls(&next, controls);
    transfer_altitude(&next);
    fist_vehicle_motion_events motion = {0};
    if (fist_weapon_begin_tick(&next) != 0 || fist_vehicle_motion_step(&next, &motion) != 0) {
        return -1;
    }
    next.drive.update_phase = (uint8_t)(next.drive.update_phase + CLASS_PHASE_STEP);
    fist_weapon_events emitted = {0};
    if (fist_weapon_reload_phase(&next, &emitted) != 0 || fist_vehicle_history_phase(&next) != 0 ||
        fist_vehicle_maintenance_phase(&next) != 0 ||
        fist_vehicle_ground_update(&next, height) != 0) {
        return -1;
    }
    *vehicle = next;
    *events = emitted;
    return 0;
}
