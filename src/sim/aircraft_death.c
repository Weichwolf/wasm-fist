#include "sim/aircraft_death.h"

#include "sim/ground.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/random.h"
#include "sim/rotation.h"
#include "sim/smoke.h"
#include "sim/vehicle_damage.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    APACHE_TYPE = 5,
    HIND_TYPE = 6,
    DEATH_BEHAVIOR = 12,
    TERRAIN_PHASE_MASK = 3,
    SPEED_PHASE_MASK = 15,
    ROTOR_MASK = 7,
    BYTE_BITS = 8,
    BYTE_RANGE = 256,
    MINIMUM_TURN = 45,
    TURN_RANDOM_MASK = 15,
    CALLBACK_MASK = 31,
    DEATH_SPIN = 1456,
    DAMAGE_SMOKE_THRESHOLD = 10,
    SMOKE_RANDOM_MASK = 3,
    SMOKE_EXTENT = 384,
    DESTRUCTION_SOUND = 9,
    DELETED_FLAG = 1
};

static int signed_byte(uint8_t value) {
    return value <= INT8_MAX ? value : (int)value - BYTE_RANGE;
}

static void altitude_step(fist_other_actor *actor, uint8_t height) {
    const uint8_t previous = (uint8_t)((uint32_t)actor->pose.altitude >> BYTE_BITS);
    uint8_t current = previous < height ? height : previous;
    const uint8_t target = (uint8_t)(height + actor->state.pair.altitude_offset);
    if (current < target) {
        ++current;
    } else if (current > target) {
        --current;
    }
    const int32_t change = ((int32_t)current - previous) * FIST_POSITION_SCALE;
    actor->pose.altitude = fist_position_add(actor->pose.altitude, change);
    actor->ground_height = (uint8_t)(current - height);
}

static void speed_step(fist_pair_actor_state *state, uint16_t tick) {
    if (state->speed > state->target_speed) {
        --state->speed;
    } else if (state->speed < state->target_speed && (tick & SPEED_PHASE_MASK) == 0) {
        ++state->speed;
    }
}

static void heading_step(fist_other_actor *actor, uint16_t random) {
    fist_pair_actor_state *state = &actor->state.pair;
    const int target = signed_byte((uint8_t)(actor->pose.heading >> BYTE_BITS));
    const int current = signed_byte(state->motion_heading);
    if (target > current) {
        state->motion_heading = (uint8_t)(state->motion_heading + 1);
    } else if (target < current) {
        state->motion_heading = (uint8_t)(state->motion_heading - 1);
    }
    int32_t difference = (int32_t)state->target_heading - actor->pose.heading;
    if (difference > INT16_MAX) {
        difference -= FIST_TURN_SIZE;
    } else if (difference < INT16_MIN) {
        difference += FIST_TURN_SIZE;
    }
    const int32_t limit = MINIMUM_TURN + (random & TURN_RANDOM_MASK);
    const int32_t magnitude = difference < 0 ? -difference : difference;
    const int32_t step = magnitude < limit ? magnitude : limit;
    actor->pose.heading = (uint16_t)(actor->pose.heading + (difference < 0 ? -step : step));
}

static int callback(fist_other_actor *actor, const fist_aircraft_environment *environment,
                    fist_aircraft_death_step *result) {
    fist_pair_actor_state *state = &actor->state.pair;
    if (actor->ground_height != 0) {
        state->target_heading = (uint16_t)(state->target_heading + DEATH_SPIN);
        state->motion_heading = (uint8_t)(state->target_heading >> BYTE_BITS);
        return 0;
    }
    const int status = fist_explosion_create(environment->pool, &actor->pose,
                                             FIST_EXPLOSION_PAIR_DESTRUCTION, &result->explosion);
    if (status < 0) {
        return -1;
    }
    result->has_explosion = status == FIST_POOL_OK;
    result->sound_request = DESTRUCTION_SOUND;
    fist_pool_allocation removed = {0};
    if (fist_object_pool_release(environment->pool, actor->allocation.registry_index, &removed) !=
        FIST_POOL_OK) {
        return -1;
    }
    actor->flags |= DELETED_FLAG;
    result->released = true;
    return 0;
}

static int emit_damage_smoke(const fist_other_actor *actor,
                             const fist_aircraft_environment *environment,
                             fist_aircraft_death_step *result) {
    if (actor->state.pair.damage <= DAMAGE_SMOKE_THRESHOLD ||
        (environment->tick & TERRAIN_PHASE_MASK) != 0) {
        return 0;
    }
    uint16_t random = 0;
    (void)fist_random_next(environment->random, &random);
    if ((random & SMOKE_RANDOM_MASK) != 0) {
        return 0;
    }
    const fist_smoke_creation request = {SMOKE_EXTENT, environment->smoke_enabled};
    const int status = fist_drifting_smoke_create(environment->pool, environment->random,
                                                  &actor->pose, request, &result->smoke);
    result->has_smoke = status == FIST_POOL_OK;
    return status < 0 ? -1 : 0;
}

static void move_pose(fist_object_pose *pose, fist_rotation rotation) {
    const fist_velocity velocity = fist_rotate(rotation);
    pose->x = fist_position_add(pose->x, velocity.x);
    pose->y = fist_position_add(pose->y, velocity.y);
}

static void movement_tail(fist_other_actor *actor, const fist_aircraft_environment *environment,
                          fist_aircraft_death_step *result) {
    if (result->released && result->has_smoke &&
        result->smoke.allocation.slot == actor->allocation.slot) {
        /* Actual 9eef reads the replacement type-17 constructor at +1b/+30:
         * both motion fields are zero. Preserve the captured emission position. */
        move_pose(&result->smoke.pose, (fist_rotation){0, 0, environment->coarse});
        return;
    }
    const fist_pair_actor_state *state = &actor->state.pair;
    const fist_rotation rotation = {(uint16_t)(state->motion_heading << BYTE_BITS), state->speed,
                                    environment->coarse};
    move_pose(&actor->pose, rotation);
}

int fist_aircraft_death_advance(fist_other_actor *actor,
                                const fist_aircraft_environment *environment,
                                fist_aircraft_death_step *out) {
    if (actor == NULL || environment == NULL || out == NULL ||
        (actor->allocation.type != APACHE_TYPE && actor->allocation.type != HIND_TYPE) ||
        actor->state.pair.behavior != DEATH_BEHAVIOR || environment->random == NULL ||
        environment->random->next_stream >= FIST_RANDOM_STREAMS ||
        !fist_object_pool_is_current(environment->pool, actor->allocation)) {
        return -1;
    }
    const fist_ground_pose pose = {actor->pose.x, actor->pose.y, actor->pose.heading};
    fist_ground_contact ground = {0};
    if (fist_ground_sample(environment->height, &pose, &ground) != 0) {
        return -1;
    }
    fist_other_actor updated = *actor;
    fist_object_pool pool = *environment->pool;
    fist_random random = *environment->random;
    fist_aircraft_environment staged = *environment;
    staged.pool = &pool;
    staged.random = &random;
    fist_aircraft_death_step result = {.sound_request = FIST_DAMAGE_NO_REQUEST};
    uint16_t roll = 0;
    (void)fist_random_next(&random, &roll);
    if ((environment->tick & TERRAIN_PHASE_MASK) == 0) {
        altitude_step(&updated, ground.height);
    }
    fist_pair_actor_state *state = &updated.state.pair;
    state->rotor_frame = (uint8_t)(environment->animation_phase & ROTOR_MASK);
    speed_step(state, environment->tick);
    heading_step(&updated, roll);
    state->behavior_countdown = (uint16_t)(state->behavior_countdown - 1);
    if ((state->behavior_countdown & CALLBACK_MASK) == 0 &&
        callback(&updated, &staged, &result) != 0) {
        return -1;
    }
    if (emit_damage_smoke(&updated, &staged, &result) != 0) {
        return -1;
    }
    movement_tail(&updated, &staged, &result);
    *actor = updated;
    *environment->pool = pool;
    *environment->random = random;
    *out = result;
    return 0;
}
