#include "vehicle_probe_io.h"

#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdio.h>

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
}
