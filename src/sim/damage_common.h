#ifndef FIST_SIM_DAMAGE_COMMON_H
#define FIST_SIM_DAMAGE_COMMON_H

#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"

#include <stdint.h>

/* Internal shared contracts for already-reached M1 primary damage. */
int fist_damage_environment_is_valid(const fist_damage_environment *environment);
int fist_damage_source_is_valid(const fist_damage_environment *environment,
                                fist_vehicle_damage_request request);
uint16_t fist_damage_next_random(fist_random *random);
uint16_t fist_damage_base_roll(fist_random *random, const uint8_t record[2]);
uint16_t fist_damage_scale_word(uint16_t value, uint16_t factor);
uint8_t fist_damage_scale_source(uint16_t value, const fist_combat_state *state,
                                 const fist_projectile *source);

#endif
