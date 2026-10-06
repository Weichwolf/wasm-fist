#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/units.h"
#include "object_pool_probe_io.h"
#include "probe_io.h"
#include "sim/collision.h"
#include "sim/object_pool.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/rotation.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    FILE_HEADER = 4,
    CASE_HEADER = 32,
    CASE_OPERATION = 0,
    CASE_FINISH = 1,
    CASE_IMPORTS = 2,
    CASE_TICKS = 4,
    CASE_AGE = 6,
    CASE_GRACE = 8,
    CASE_FRAME = 10,
    CASE_LAST_FRAME = 11,
    CASE_PERIOD = 12,
    CASE_COUNTDOWN = 13,
    CASE_VX = 14,
    CASE_VY = 16,
    CASE_VZ = 18,
    CASE_ORIGIN = 20,
    CASE_RANDOM = 22,
    CASE_CURSOR = 30,
    CASE_RESERVED = 31,
    BODY_BYTES = 24,
    BODY_INDEX = 2,
    BODY_VALUE = 4,
    BODY_X = 6,
    BODY_Y = 10,
    BODY_ALTITUDE = 14,
    BODY_HEADING = 18,
    BODY_SCALE = 20,
    BODY_FLAGS = 22,
    BODY_MODE = 23,
    PROJECTILE_TYPE = 8,
    EXPLOSION_TYPE = 4,
    MUZZLE_TYPE = 18,
    EFFECT_EXTENT = 512,
    HEIGHT_SIDE = 2,
    HEIGHT_PIXELS = HEIGHT_SIDE * HEIGHT_SIDE,
    MARKER = 123
};

typedef struct {
    fist_object_pool pool;
    fist_pool_allocation allocations[FIST_UNIT_REGISTRY_COUNT];
    fist_object_pose poses[FIST_UNIT_REGISTRY_COUNT];
    fist_collision_body bodies[FIST_UNIT_REGISTRY_COUNT];
    fist_random random;
    fist_projectile shell;
    fist_explosion explosion;
    fist_muzzle_smoke muzzle;
    uint8_t heights[HEIGHT_PIXELS];
    uint16_t ticks;
    uint8_t operation;
    uint8_t finish;
} flight_case;

static int same_bytes(const void *left, size_t size, const void *right) {
    const unsigned char *first = left;
    const unsigned char *second = right;
    for (size_t index = 0; index < size; ++index) {
        if (first[index] != second[index]) {
            return 0;
        }
    }
    return 1;
}

static fist_projectile_environment environment(flight_case *value, const fist_klc_image *height) {
    return (fist_projectile_environment){&value->pool,
                                         {&value->pool, value->bodies, FIST_UNIT_REGISTRY_COUNT},
                                         height,
                                         &value->random};
}

static int import_body(flight_case *value, const uint8_t *input, size_t ordinal) {
    fist_pool_allocation allocation = {0};
    const fist_pool_import request = {fist_read_u16le(input), fist_read_u16le(input + BODY_INDEX),
                                      fist_read_u16le(input + BODY_VALUE)};
    if (fist_object_pool_import(&value->pool, request, &allocation) != 0) {
        return -1;
    }
    const size_t slot = allocation.slot;
    value->poses[slot] = (fist_object_pose){
        fist_read_i32le(input + BODY_X), fist_read_i32le(input + BODY_Y),
        fist_read_i32le(input + BODY_ALTITUDE), fist_read_u16le(input + BODY_HEADING)};
    value->bodies[slot] =
        (fist_collision_body){&value->poses[slot], fist_read_u16le(input + BODY_SCALE),
                              input[BODY_FLAGS], input[BODY_MODE]};
    value->allocations[ordinal] = allocation;
    return 0;
}

static int prepare(flight_case *value, const uint8_t *input, size_t size) {
    const size_t count = fist_read_u16le(input + CASE_IMPORTS);
    if (count == 0 || count > FIST_UNIT_REGISTRY_COUNT ||
        size != CASE_HEADER + (count * BODY_BYTES) + HEIGHT_PIXELS || input[CASE_FINISH] > 1 ||
        input[CASE_OPERATION] > 2 || input[CASE_RESERVED] != 0 ||
        input[CASE_CURSOR] >= FIST_RANDOM_STREAMS || fist_read_u16le(input + CASE_TICKS) == 0 ||
        fist_read_u16le(input + CASE_ORIGIN) >= count) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    for (size_t index = 0; index < count; ++index) {
        if (import_body(value, input + CASE_HEADER + (index * BODY_BYTES), index) != 0) {
            return -1;
        }
    }
    for (size_t index = 0; index < HEIGHT_PIXELS; ++index) {
        value->heights[index] = input[CASE_HEADER + (count * BODY_BYTES) + index];
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        value->random.words[index] =
            fist_read_u16le(input + CASE_RANDOM + (index * sizeof(uint16_t)));
    }
    value->random.next_stream = input[CASE_CURSOR];
    value->ticks = fist_read_u16le(input + CASE_TICKS);
    value->operation = input[CASE_OPERATION];
    value->finish = input[CASE_FINISH];
    const fist_pool_allocation allocation = value->allocations[0];
    const fist_object_pose pose = value->poses[allocation.slot];
    const fist_collision_body body = value->bodies[allocation.slot];
    value->shell = (fist_projectile){
        .allocation = allocation,
        .pose = pose,
        .velocity = {fist_read_i16le(input + CASE_VX), fist_read_i16le(input + CASE_VY),
                     fist_read_i16le(input + CASE_VZ)},
        .age = fist_read_u16le(input + CASE_AGE),
        .collision_grace = fist_read_u16le(input + CASE_GRACE),
        .origin_slot = value->allocations[fist_read_u16le(input + CASE_ORIGIN)].slot,
        .target_slot = FIST_POOL_NO_SLOT,
        .flags = body.flags,
        .ground_height = input[CASE_FRAME],
        .mode = body.mode};
    value->explosion = (fist_explosion){.allocation = allocation,
                                        .pose = {pose.x, pose.y, pose.altitude, 0},
                                        .model_code = pose.heading,
                                        .extent = EFFECT_EXTENT,
                                        .projection_scale = body.projection_scale,
                                        .callback_selector = fist_read_u16le(input + CASE_GRACE),
                                        .height_offset = fist_read_u16le(input + CASE_AGE),
                                        .frame = input[CASE_FRAME],
                                        .last_frame = input[CASE_LAST_FRAME],
                                        .period = input[CASE_PERIOD],
                                        .countdown = input[CASE_COUNTDOWN],
                                        .flags = body.flags};
    value->muzzle = (fist_muzzle_smoke){.allocation = allocation,
                                        .pose = pose,
                                        .projection_scale = body.projection_scale,
                                        .animation_counter = fist_read_u16le(input + CASE_AGE),
                                        .animation_frame = input[CASE_FRAME],
                                        .flags = body.flags};
    if (value->operation == 0) {
        value->bodies[allocation.slot].pose = &value->shell.pose;
    }
    return 0;
}

static void write_shell(const fist_projectile *shell) {
    printf("shell %ld %ld %ld %u %d %d %d %u %u %u %u %u %u %u %u\n", (long)shell->pose.x,
           (long)shell->pose.y, (long)shell->pose.altitude, (unsigned)shell->pose.heading,
           (int)shell->velocity.x, (int)shell->velocity.y, (int)shell->velocity.z,
           (unsigned)shell->age, (unsigned)shell->collision_grace, (unsigned)shell->flags,
           (unsigned)shell->ground_height, (unsigned)shell->mode, (unsigned)shell->origin_slot,
           (unsigned)shell->target_slot, (unsigned)shell->phase);
}

static void write_explosion(const fist_explosion *explosion) {
    printf("explosion %u %u %u %u %ld %ld %ld %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)explosion->allocation.type, (unsigned)explosion->allocation.slot,
           (unsigned)explosion->allocation.registry_index, (unsigned)explosion->allocation.value,
           (long)explosion->pose.x, (long)explosion->pose.y, (long)explosion->pose.altitude,
           (unsigned)explosion->model_code, (unsigned)explosion->extent,
           (unsigned)explosion->projection_scale, (unsigned)explosion->callback_selector,
           (unsigned)explosion->height_offset, (unsigned)explosion->frame,
           (unsigned)explosion->last_frame, (unsigned)explosion->period,
           (unsigned)explosion->countdown, (unsigned)explosion->flags);
}

static void write_random(const fist_random *random) {
    printf("random %u", (unsigned)random->next_stream);
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        printf(" %u", (unsigned)random->words[index]);
    }
    printf("\n");
}

static int flight_tick(flight_case *value, const fist_projectile_environment *context) {
    fist_projectile_step result = {0};
    if (fist_projectile_advance(&value->shell, context, &result) != 0) {
        return -1;
    }
    printf("hit %u %u %u %u\n", (unsigned)result.hit.slot, (unsigned)result.hit.registry_index,
           (unsigned)result.hit.value, (unsigned)result.hit.aspect);
    write_shell(&value->shell);
    write_random(&value->random);
    if (value->finish != 0 && (result.phase == FIST_PROJECTILE_GROUND_IMPACT ||
                               result.phase == FIST_PROJECTILE_UNIT_IMPACT)) {
        /* Fixture resumes at the post-damage boundary; no fake damage method. */
        fist_projectile_impact impact = {0};
        if (fist_projectile_finish_impact(&value->pool, &value->shell, &impact) != 0) {
            return -1;
        }
        printf("impact %u %u %u %u\n", (unsigned)impact.has_explosion, (unsigned)impact.notice,
               (unsigned)impact.sound_request, (unsigned)impact.hit_voice);
        if (impact.has_explosion) {
            write_explosion(&impact.explosion);
        }
        write_shell(&value->shell);
    }
    return 0;
}

static int observe(flight_case *value) {
    const fist_klc_image height = {
        .width = HEIGHT_SIDE, .height = HEIGHT_SIDE, .pixels = value->heights};
    const fist_projectile_environment context = environment(value, &height);
    for (size_t tick = 0; tick < value->ticks; ++tick) {
        int status = 0;
        if (value->operation == 0) {
            status = flight_tick(value, &context);
        } else if (value->operation == 1) {
            status = fist_explosion_advance(&value->pool, &value->explosion);
            if (status == 0) {
                write_explosion(&value->explosion);
            }
        } else {
            status = fist_muzzle_smoke_advance(&value->pool, &value->muzzle);
            if (status == 0) {
                printf("muzzle %u %u %u\n", (unsigned)value->muzzle.animation_counter,
                       (unsigned)value->muzzle.animation_frame, (unsigned)value->muzzle.flags);
            }
        }
        if (status != 0) {
            return -1;
        }
        fist_probe_write_object_pool(&value->pool);
        if ((value->operation == 0 && value->shell.phase != FIST_PROJECTILE_FLYING) ||
            (value->operation == 1 && (value->explosion.flags & 1U) != 0) ||
            (value->operation == 2 && (value->muzzle.flags & 1U) != 0)) {
            break;
        }
    }
    return 0;
}

static int rejected(flight_case *value, const fist_projectile_environment *context) {
    const flight_case before = *value;
    const fist_projectile_step marker = {{MARKER, MARKER, MARKER, MARKER}, MARKER};
    fist_projectile_step output = marker;
    return fist_projectile_advance(&value->shell, context, &output) == -1 &&
           same_bytes(value, sizeof(*value), &before) &&
           same_bytes(&output, sizeof(output), &marker);
}

static int invalid_inputs(flight_case *value) {
    const fist_projectile_environment context = environment(value, NULL);
    const flight_case before = *value;
    fist_projectile_step output = {0};
    int valid = rejected(value, NULL) && rejected(value, &context) &&
                fist_projectile_advance(NULL, &context, &output) == -1 &&
                fist_projectile_advance(&value->shell, &context, NULL) == -1;
    value->shell.pose.altitude = INT32_MAX;
    fist_projectile_environment broken = context;
    broken.world.body_count = 0;
    valid = valid && rejected(value, &broken);
    broken = context;
    broken.pool = NULL;
    valid = valid && rejected(value, &broken);
    broken = context;
    broken.random = NULL;
    valid = valid && rejected(value, &broken);
    value->shell.phase = FIST_PROJECTILE_UNIT_IMPACT;
    valid = valid && rejected(value, &context);
    value->shell.phase = FIST_PROJECTILE_FLYING;
    value->shell.target_slot = 0;
    valid = valid && rejected(value, &context);
    value->shell.target_slot = FIST_POOL_NO_SLOT;
    value->shell.allocation.value = MARKER;
    valid = valid && rejected(value, &context);
    *value = before;
    const fist_projectile_impact marker = {.notice = MARKER};
    fist_projectile_impact impact = marker;
    valid = valid && fist_projectile_finish_impact(&value->pool, &value->shell, &impact) == -1 &&
            same_bytes(value, sizeof(*value), &before) &&
            same_bytes(&impact, sizeof(impact), &marker) &&
            fist_explosion_advance(&value->pool, &value->explosion) == -1 &&
            fist_muzzle_smoke_advance(&value->pool, &value->muzzle) == -1 &&
            same_bytes(value, sizeof(*value), &before);
    return valid ? 0 : -1;
}

static int run(const uint8_t *input, size_t size) {
    if (size < FILE_HEADER) {
        return -1;
    }
    const size_t count = fist_read_u32le(input);
    if (count == 0 || count > (size - FILE_HEADER) / CASE_HEADER) {
        return -1;
    }
    size_t offset = FILE_HEADER;
    flight_case *value = calloc(1, sizeof(*value));
    if (value == NULL) {
        return -1;
    }
    int status = 0;
    for (size_t index = 0; index < count; ++index) {
        if (size - offset < CASE_HEADER) {
            status = -1;
            break;
        }
        const size_t extent = CASE_HEADER +
                              (fist_read_u16le(input + offset + CASE_IMPORTS) * BODY_BYTES) +
                              HEIGHT_PIXELS;
        *value = (flight_case){0};
        if (extent > size - offset || prepare(value, input + offset, extent) != 0 ||
            observe(value) != 0) {
            status = -1;
            break;
        }
        offset += extent;
    }
    if (offset != size) {
        status = -1;
    }
    free(value);
    return status;
}

static int identity_rejections(flight_case *value) {
    const flight_case before = *value;
    const fist_projectile_environment context = environment(value, NULL);
    value->shell.pose.altitude = INT32_MAX;
    const uint16_t slot = value->shell.allocation.slot;
    value->bodies[slot].pose = &value->poses[slot];
    int valid = rejected(value, &context);
    value->bodies[slot].pose = &value->shell.pose;
    value->bodies[slot].mode = MARKER;
    valid = valid && rejected(value, &context);
    value->bodies[slot].mode = value->shell.mode;
    value->bodies[slot].flags = MARKER;
    valid = valid && rejected(value, &context);
    value->bodies[slot].flags = value->shell.flags;
    value->random.next_stream = FIST_RANDOM_STREAMS;
    valid = valid && rejected(value, &context);
    value->random.next_stream = 0;
    value->shell.origin_slot = FIST_POOL_NO_SLOT;
    valid = valid && rejected(value, &context);
    *value = before;
    return valid ? 0 : -1;
}

static int pool_lookup_rejections(flight_case *value) {
    const fist_pool_allocation marker = {MARKER, MARKER, MARKER, MARKER};
    fist_pool_allocation output = marker;
    int valid = fist_object_pool_find(NULL, 0, &output) == -1 &&
                fist_object_pool_find(&value->pool, FIST_POOL_NO_SLOT, &output) == -1 &&
                fist_object_pool_find(&value->pool, value->shell.allocation.slot, NULL) == -1 &&
                fist_object_pool_find(&value->pool, 1, &output) == FIST_POOL_UNAVAILABLE &&
                same_bytes(&output, sizeof(output), &marker);
    value->pool.registry[0].slot = FIST_POOL_NO_SLOT;
    valid = valid && fist_object_pool_find(&value->pool, 0, &output) == FIST_POOL_UNAVAILABLE &&
            same_bytes(&output, sizeof(output), &marker);
    value->pool.registry[0].slot = value->shell.allocation.slot;
    return valid ? 0 : -1;
}

static int self_check(void) {
    flight_case *value = calloc(1, sizeof(*value));
    if (value == NULL) {
        return -1;
    }
    fist_object_pool_reset(&value->pool);
    fist_pool_allocation allocation = {0};
    int status = fist_object_pool_allocate(&value->pool, (fist_pool_request){PROJECTILE_TYPE, 0},
                                           &allocation);
    if (status == 0) {
        value->shell.allocation = allocation;
        value->shell.target_slot = FIST_POOL_NO_SLOT;
        value->shell.origin_slot = allocation.slot;
        value->bodies[allocation.slot].pose = &value->shell.pose;
        status = invalid_inputs(value) == 0 && identity_rejections(value) == 0 &&
                         pool_lookup_rejections(value) == 0
                     ? 0
                     : -1;
    }
    free(value);
    return status;
}

int main(int argc, char **argv) {
    if (argc != 2 || self_check() != 0) {
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *input = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    const int status = input != NULL && closed == 0 ? run(input, size) : -1;
    free(input);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
