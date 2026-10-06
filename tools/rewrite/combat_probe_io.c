#include "combat_probe_io.h"

#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/smoke.h"

#include <stdint.h>
#include <stdio.h>

enum { PAIR_FIRST = 5, PAIR_SECOND = 6, TYPE26_TYPE = 26, LAST_TYPE = 27 };

void fist_probe_write_other_actor(const fist_other_actor *actor) {
    const fist_pool_allocation allocation = actor->allocation;
    printf("actor %u %u %u %u %ld %ld %ld %u %u %u %u %u %u %u\n", (unsigned)allocation.type,
           (unsigned)allocation.slot, (unsigned)allocation.registry_index,
           (unsigned)allocation.value, (long)actor->pose.x, (long)actor->pose.y,
           (long)actor->pose.altitude, (unsigned)actor->pose.heading,
           (unsigned)actor->projection_extent, (unsigned)actor->projection_scale,
           (unsigned)actor->flags, (unsigned)actor->secondary_flags, (unsigned)actor->ground_height,
           (unsigned)actor->mode);
    if (allocation.type == PAIR_FIRST || allocation.type == PAIR_SECOND) {
        const fist_pair_actor_state *state = &actor->state.pair;
        printf("pair %u %u %u %u %d %u %u %u %u\n", (unsigned)(uint16_t)state->target_speed,
               (unsigned)state->behavior, (unsigned)state->altitude_offset, (unsigned)state->damage,
               (int)state->speed, (unsigned)state->behavior_countdown,
               (unsigned)state->target_heading, (unsigned)state->motion_heading,
               (unsigned)state->rotor_frame);
    } else if (allocation.type == TYPE26_TYPE) {
        const fist_type26_state *state = &actor->state.type26;
        printf("type26 %u %u %u %u\n", (unsigned)state->damage, (unsigned)state->limit,
               (unsigned)state->destruction_parameter, (unsigned)state->emission_counter);
    } else if (allocation.type == LAST_TYPE) {
        const fist_type27_state *state = &actor->state.type27;
        printf("type27 %u %u %u %u\n", (unsigned)state->damage, (unsigned)state->debris_parameter,
               (unsigned)state->animation_counter, (unsigned)state->emission_counter);
    }
}

void fist_probe_write_explosion(const fist_explosion *effect) {
    printf("effect %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)effect->allocation.slot, (unsigned)effect->allocation.registry_index,
           (unsigned)effect->allocation.value, (long)effect->pose.x, (long)effect->pose.y,
           (long)effect->pose.altitude, (unsigned)effect->model_code, (unsigned)effect->extent,
           (unsigned)effect->projection_scale, (unsigned)effect->callback_selector,
           (unsigned)effect->height_offset, (unsigned)effect->frame, (unsigned)effect->last_frame,
           (unsigned)effect->period, (unsigned)effect->countdown, (unsigned)effect->flags);
}

void fist_probe_write_smoke(const fist_drifting_smoke *smoke) {
    printf("smoke %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u\n", (unsigned)smoke->allocation.slot,
           (unsigned)smoke->allocation.registry_index, (unsigned)smoke->allocation.value,
           (long)smoke->pose.x, (long)smoke->pose.y, (long)smoke->pose.altitude,
           (unsigned)smoke->pose.heading, (unsigned)smoke->extent,
           (unsigned)smoke->projection_scale, (unsigned)smoke->flags,
           (unsigned)smoke->secondary_flags, (unsigned)smoke->ground_height,
           (unsigned)smoke->animation_frame, (unsigned)smoke->animation_counter);
}
