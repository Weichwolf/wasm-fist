#include "vehicle_probe_io.h"

#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

size_t fist_probe_component_offset(uint16_t type) {
    static const size_t offsets[] = {191, 188, 190, 188};
    return type < sizeof(offsets) / sizeof(offsets[0]) ? offsets[type] : 0;
}

void fist_probe_write_vehicle_state(const fist_vehicle_state *state) {
    const fist_vehicle_drive *drive = &state->drive;
    const fist_vehicle_turret *turret = &state->turret;
    printf("state %u %u %u %d %d %d %d %d %d %d %d %u %u %u %u %u %u %u %u\n",
           (unsigned)state->type, (unsigned)state->registry_index, (unsigned)state->generation,
           state->map_x, state->map_y, state->altitude, drive->speed, drive->throttle,
           drive->terrain_pitch, drive->velocity_x, drive->velocity_y, (unsigned)drive->heading,
           (unsigned)drive->requested_heading, (unsigned)drive->movement_gate,
           (unsigned)drive->motion_flags, (unsigned)drive->update_phase, (unsigned)turret->heading,
           (unsigned)turret->offset, (unsigned)turret->requested_offset);
    printf("ground %u %d %d %d\n", (unsigned)state->ground_height, drive->terrain_roll,
           turret->terrain_roll, turret->terrain_pitch);
    printf("control %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)state->projection_extent, (unsigned)state->projection_scale,
           (unsigned)state->camera_height, (unsigned)state->control_flags,
           (unsigned)state->object_flags, (unsigned)state->secondary_flags,
           (unsigned)state->operating_flags, (unsigned)state->random_phases[0],
           (unsigned)state->random_phases[1], (unsigned)state->control_mode,
           (unsigned)state->turret_view_mode, (unsigned)state->hull_view_mode,
           (unsigned)state->behavior, (unsigned)state->reload_countdown,
           (unsigned)state->component_size);
    printf("weapons");
    for (size_t slot = 0; slot < FIST_VEHICLE_WEAPON_SLOTS; ++slot) {
        printf(" %u", (unsigned)state->weapons.rounds[slot]);
    }
    printf(" %u %u %u %u\n", (unsigned)state->weapons.class_parameter,
           (unsigned)state->weapons.cycle[0], (unsigned)state->weapons.cycle[1],
           (unsigned)state->weapons.ready_stock);
    printf("weapon_control %u %u %u %u %d %u\n", (unsigned)state->weapons.selected,
           (unsigned)state->weapons.loaded, (unsigned)state->weapons.trigger,
           (unsigned)state->weapons.recoil, turret->elevation, (unsigned)turret->elevation_frame);
    printf("components");
    for (size_t index = 0; index < state->component_size; ++index) {
        printf(" %02x", (unsigned)state->components[index]);
    }
    puts("");
    printf("selectors");
    for (size_t index = 0; index < FIST_VEHICLE_ANIMATION_SELECTORS; ++index) {
        printf(" %u", (unsigned)state->animation_selectors[index]);
    }
    puts("");
    printf("behavior_flags %u\n", (unsigned)state->behavior_flags);
    printf("speed_counter %u\n", (unsigned)drive->speed_counter);
    printf("position_history");
    for (size_t index = 0; index < FIST_VEHICLE_POSITION_SAMPLES; ++index) {
        printf(" %u %u", (unsigned)state->position_history[index].x,
               (unsigned)state->position_history[index].y);
    }
    puts("");
}
