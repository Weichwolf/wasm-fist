#include "sim/projectile_flight.h"

#include "assets/klc.h"
#include "assets/units.h"
#include "sim/collision.h"
#include "sim/ground.h"
#include "sim/object_pool.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    PROJECTILE_TYPE = 8,
    EXPLOSION_TYPE = 4,
    MUZZLE_TYPE = 18,
    DELETED_FLAG = 1,
    PROJECTILE_LIFETIME = 480,
    HEIGHT_SHIFT = 8,
    TERRAIN_GATE = 128,
    SUBTRACTION_SIGN = 128,
    IMPACT_SOUND = 15,
    EXPLOSION_SCALE = 2048,
    UNIT_CALLBACK = 4,
    CALLBACK_MASK = 6,
    GROUND_RISE_FRAME = 13,
    GROUND_MIDDLE_FRAME = 16,
    GROUND_FALL_FRAME = 19,
    UNIT_RISE_FRAME = 6,
    UNIT_FALL_FRAME = 8,
    HIGH_OFFSET = 768,
    MIDDLE_OFFSET = 512,
    LOW_OFFSET = 256,
    MUZZLE_PERIOD = 8,
    MUZZLE_LAST_FRAME = 7
};

static int live_binding(const fist_object_pool *pool, fist_pool_allocation allocation,
                        uint16_t type) {
    return allocation.type == type && fist_object_pool_is_current(pool, allocation);
}

static int retire(fist_object_pool *pool, fist_pool_allocation allocation) {
    fist_pool_allocation removed = {0};
    return fist_object_pool_release(pool, allocation.registry_index, &removed) == FIST_POOL_OK ? 0
                                                                                               : -1;
}

static int ground_impact(fist_projectile *projectile, const fist_klc_image *height) {
    if ((uint16_t)((uint32_t)projectile->pose.altitude >> HEIGHT_SHIFT) >= TERRAIN_GATE) {
        return 0;
    }
    const fist_ground_pose pose = {projectile->pose.x, projectile->pose.y,
                                   projectile->pose.heading};
    fist_ground_contact contact = {0};
    if (fist_ground_sample(height, &pose, &contact) != 0) {
        return -1;
    }
    projectile->ground_height = contact.height;
    const uint8_t difference =
        (uint8_t)(((uint32_t)projectile->pose.altitude >> HEIGHT_SHIFT) - contact.height);
    return (difference & SUBTRACTION_SIGN) != 0;
}

static int unit_impact(fist_projectile *projectile, const fist_collision_world *world,
                       fist_random *random, fist_projectile_step *result) {
    if (projectile->collision_grace != 0) {
        --projectile->collision_grace;
        return 0;
    }
    fist_collision_body bodies[FIST_UNIT_REGISTRY_COUNT] = {0};
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        bodies[index] = world->bodies[index];
    }
    bodies[projectile->allocation.slot].pose = &projectile->pose;
    const fist_collision_world updated = {world->pool, bodies, FIST_UNIT_REGISTRY_COUNT};
    fist_collision_hit hit = {0};
    if (fist_collision_find(&updated, projectile->allocation.slot, random, &hit) != 0) {
        return -1;
    }
    if (hit.slot != FIST_POOL_NO_SLOT && hit.slot != projectile->origin_slot) {
        projectile->phase = FIST_PROJECTILE_UNIT_IMPACT;
        result->hit = hit;
    }
    return 0;
}

static int valid_environment(const fist_projectile *projectile,
                             const fist_projectile_environment *environment) {
    if (projectile == NULL || environment == NULL ||
        !fist_collision_world_is_valid(&environment->world) ||
        environment->pool != environment->world.pool || environment->random == NULL ||
        environment->random->next_stream >= FIST_RANDOM_STREAMS ||
        projectile->phase != FIST_PROJECTILE_FLYING || (projectile->flags & DELETED_FLAG) != 0 ||
        projectile->target_slot != FIST_POOL_NO_SLOT ||
        projectile->origin_slot >= FIST_UNIT_REGISTRY_COUNT ||
        !live_binding(environment->pool, projectile->allocation, PROJECTILE_TYPE)) {
        return 0;
    }
    const fist_collision_body *body = &environment->world.bodies[projectile->allocation.slot];
    return body->pose == &projectile->pose && body->flags == projectile->flags &&
           body->mode == projectile->mode;
}

int fist_projectile_advance(fist_projectile *projectile,
                            const fist_projectile_environment *environment,
                            fist_projectile_step *out) {
    if (out == NULL || !valid_environment(projectile, environment)) {
        return -1;
    }
    fist_projectile updated = *projectile;
    fist_random random = *environment->random;
    fist_projectile_step result = {.hit = {FIST_POOL_NO_SLOT, FIST_POOL_NO_SLOT, 0, 0}};
    updated.age = (uint16_t)(updated.age + 1);
    if (updated.age >= PROJECTILE_LIFETIME) {
        if (retire(environment->pool, updated.allocation) != 0) {
            return -1;
        }
        updated.flags |= DELETED_FLAG;
        updated.phase = FIST_PROJECTILE_RETIRED;
    } else {
        updated.pose.x = fist_position_add(updated.pose.x, updated.velocity.x);
        updated.pose.y = fist_position_add(updated.pose.y, updated.velocity.y);
        updated.pose.altitude = fist_position_add(updated.pose.altitude, updated.velocity.z);
        const int ground = ground_impact(&updated, environment->height);
        if (ground < 0) {
            return -1;
        }
        if (ground != 0) {
            updated.phase = FIST_PROJECTILE_GROUND_IMPACT;
        } else if (unit_impact(&updated, &environment->world, &random, &result) != 0) {
            return -1;
        }
    }
    result.phase = updated.phase;
    *environment->random = random;
    *projectile = updated;
    *out = result;
    return 0;
}

typedef struct {
    uint16_t model_code;
    uint16_t extent;
    uint16_t callback_selector;
    uint8_t last_frame;
    uint8_t period;
} explosion_template;

/* Exact 9c1d/9c4d/9c5d/9c3d/9c65/9c6d/9c2d authored templates. */
static const explosion_template explosion_templates[FIST_EXPLOSION_TEMPLATE_COUNT] = {
    {16, 768, 0, 22, 6}, {20, 256, 4, 10, 5},   {20, 448, 4, 10, 7},  {19, 768, 2, 21, 6},
    {20, 768, 4, 10, 9}, {20, 1280, 4, 10, 11}, {16, 2048, 0, 22, 10}};

int fist_explosion_create(fist_object_pool *pool, const fist_object_pose *pose, uint8_t template_id,
                          fist_explosion *out) {
    if (pose == NULL || out == NULL || template_id >= FIST_EXPLOSION_TEMPLATE_COUNT) {
        return -1;
    }
    fist_pool_allocation allocation = {0};
    const int status =
        fist_object_pool_allocate(pool, (fist_pool_request){EXPLOSION_TYPE, 0}, &allocation);
    if (status != FIST_POOL_OK) {
        return status;
    }
    const explosion_template *parameters = &explosion_templates[template_id];
    *out = (fist_explosion){.allocation = allocation,
                            .pose = {pose->x, pose->y, pose->altitude, 0},
                            .model_code = parameters->model_code,
                            .extent = parameters->extent,
                            .projection_scale = EXPLOSION_SCALE,
                            .callback_selector = parameters->callback_selector,
                            .last_frame = parameters->last_frame,
                            .period = parameters->period,
                            .countdown = parameters->period};
    return 0;
}

int fist_projectile_finish_impact(fist_object_pool *pool, fist_projectile *projectile,
                                  fist_projectile_impact *out) {
    if (projectile == NULL || out == NULL ||
        (projectile->phase != FIST_PROJECTILE_UNIT_IMPACT &&
         projectile->phase != FIST_PROJECTILE_GROUND_IMPACT) ||
        (projectile->flags & DELETED_FLAG) != 0 ||
        !live_binding(pool, projectile->allocation, PROJECTILE_TYPE)) {
        return -1;
    }
    fist_object_pool updated = *pool;
    fist_projectile_impact result = {.notice = projectile->phase,
                                     .sound_request = IMPACT_SOUND,
                                     .hit_voice = projectile->phase == FIST_PROJECTILE_UNIT_IMPACT};
    const uint8_t template_id = projectile->phase == FIST_PROJECTILE_UNIT_IMPACT
                                    ? FIST_EXPLOSION_SHELL_UNIT
                                    : FIST_EXPLOSION_SHELL_GROUND;
    const int status =
        fist_explosion_create(&updated, &projectile->pose, template_id, &result.explosion);
    if (status < 0 || retire(&updated, projectile->allocation) != 0) {
        return -1;
    }
    if (status == FIST_POOL_OK) {
        result.has_explosion = true;
    }
    projectile->flags |= DELETED_FLAG;
    projectile->phase = FIST_PROJECTILE_RETIRED;
    *pool = updated;
    *out = result;
    return 0;
}

static void explosion_height(fist_explosion *explosion) {
    const uint16_t callback = explosion->callback_selector & CALLBACK_MASK;
    if (callback == 0) {
        switch (explosion->frame) {
        case GROUND_RISE_FRAME:
            explosion->height_offset = HIGH_OFFSET;
            break;
        case GROUND_MIDDLE_FRAME:
            explosion->height_offset = MIDDLE_OFFSET;
            break;
        case GROUND_FALL_FRAME:
            explosion->height_offset = LOW_OFFSET;
            break;
        default:
            break;
        }
    } else if (callback == UNIT_CALLBACK) {
        if (explosion->frame == UNIT_RISE_FRAME) {
            explosion->height_offset = MIDDLE_OFFSET;
        } else if (explosion->frame == UNIT_FALL_FRAME) {
            explosion->height_offset = LOW_OFFSET;
        }
    }
}

int fist_explosion_advance(fist_object_pool *pool, fist_explosion *explosion) {
    if (explosion == NULL || (explosion->flags & DELETED_FLAG) != 0 ||
        !live_binding(pool, explosion->allocation, EXPLOSION_TYPE)) {
        return -1;
    }
    fist_explosion updated = *explosion;
    updated.countdown = (uint8_t)(updated.countdown - 1);
    if (updated.countdown == 0) {
        updated.countdown = updated.period;
        if (updated.frame == updated.last_frame) {
            if (retire(pool, updated.allocation) != 0) {
                return -1;
            }
            updated.flags |= DELETED_FLAG;
        } else {
            updated.frame = (uint8_t)(updated.frame + 1);
            explosion_height(&updated);
        }
    }
    *explosion = updated;
    return 0;
}

int fist_muzzle_smoke_advance(fist_object_pool *pool, fist_muzzle_smoke *muzzle) {
    if (muzzle == NULL || (muzzle->flags & DELETED_FLAG) != 0 ||
        !live_binding(pool, muzzle->allocation, MUZZLE_TYPE)) {
        return -1;
    }
    fist_muzzle_smoke updated = *muzzle;
    updated.animation_counter = (uint16_t)(updated.animation_counter + 1);
    if (updated.animation_counter >= MUZZLE_PERIOD) {
        updated.animation_counter = 0;
        updated.animation_frame = (uint8_t)(updated.animation_frame + 1);
        if (updated.animation_frame >= MUZZLE_LAST_FRAME) {
            if (retire(pool, updated.allocation) != 0) {
                return -1;
            }
            updated.flags |= DELETED_FLAG;
        }
    }
    *muzzle = updated;
    return 0;
}
