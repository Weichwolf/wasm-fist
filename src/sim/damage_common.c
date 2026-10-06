#include "sim/damage_common.h"

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"

#include <stddef.h>
#include <stdint.h>

enum {
    PRIMARY_PROJECTILE = 8,
    PRIMARY_PARAMETER = 5,
    SIDE_FLAG = 8,
    DELETED_FLAG = 1,
    ASPECTS = 16,
    FIXED_SCALE_SHIFT = 8
};

int fist_damage_environment_is_valid(const fist_damage_environment *environment) {
    if (environment == NULL || !fist_object_pool_is_valid(environment->pool) ||
        environment->random == NULL || environment->random->next_stream >= FIST_RANDOM_STREAMS ||
        environment->state == NULL) {
        return 0;
    }
    const fist_combat_state *state = environment->state;
    if (state->selected_slot != FIST_POOL_NO_SLOT &&
        (state->selected_slot >= FIST_UNIT_REGISTRY_COUNT ||
         environment->pool->slots[state->selected_slot].used == 0)) {
        return 0;
    }
    for (size_t index = 0; index < FIST_UNIT_ROSTER_COUNT; ++index) {
        const uint16_t slot = state->roster[index];
        if (slot != FIST_POOL_NO_SLOT &&
            (slot >= FIST_UNIT_REGISTRY_COUNT || environment->pool->slots[slot].used == 0)) {
            return 0;
        }
    }
    return 1;
}

int fist_damage_source_is_valid(const fist_damage_environment *environment,
                                fist_vehicle_damage_request request) {
    const fist_projectile *source = request.projectile;
    return fist_damage_environment_is_valid(environment) && source != NULL &&
           request.hit.aspect < ASPECTS && source->allocation.type == PRIMARY_PROJECTILE &&
           source->collision_profile == 0 && source->launch_parameter == PRIMARY_PARAMETER &&
           source->phase == FIST_PROJECTILE_UNIT_IMPACT && (source->flags & DELETED_FLAG) == 0 &&
           fist_object_pool_is_current(environment->pool, source->allocation);
}

uint16_t fist_damage_next_random(fist_random *random) {
    uint16_t value = 0;
    (void)fist_random_next(random, &value);
    return value;
}

uint16_t fist_damage_base_roll(fist_random *random, const uint8_t record[2]) {
    const uint16_t value = (uint8_t)fist_damage_next_random(random);
    return (uint16_t)(((value * record[1]) >> FIXED_SCALE_SHIFT) + record[0] + 1);
}

uint16_t fist_damage_scale_word(uint16_t value, uint16_t factor) {
    return (uint16_t)((uint16_t)((uint32_t)value * factor) >> FIXED_SCALE_SHIFT);
}

uint8_t fist_damage_scale_source(uint16_t value, const fist_combat_state *state,
                                 const fist_projectile *source) {
    const size_t side = (source->flags & SIDE_FLAG) != 0;
    return (uint8_t)fist_damage_scale_word(value, state->source_scale[side]);
}
