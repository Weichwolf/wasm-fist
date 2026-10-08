#ifndef FIST_SIM_AUTOMATIC_FIRE_H
#define FIST_SIM_AUTOMATIC_FIRE_H

#include "sim/object_pool.h"
#include "sim/rotation.h"
#include "sim/voice.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

enum { FIST_SURFACE_AIR_TYPE = 15, FIST_SURFACE_AIR_NO_RACK = UINT8_MAX };

/* Complete dynamic b8d1 constructor state. Saved restoration and b918 flight
 * are separate methods; no guest pointers or borrowed source records survive. */
typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    fist_object_reference origin;
    fist_object_reference target;
    fist_spatial_velocity velocity;
    uint16_t projection_extent;
    uint16_t projection_scale;
    uint16_t age;
    uint16_t steering;
    int16_t speed;
    int16_t elevation;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
    uint8_t variant;
    uint8_t stage;
} fist_surface_air_missile;

typedef struct {
    uint16_t slot;
    /* Caller-owned c047 source identity, distinct from display selection.
     * Comparison only; NO_SLOT or another slot emits no logical request. */
    uint16_t sound_source;
} fist_surface_air_request;

typedef struct {
    fist_surface_air_request source;
    /* The parent's already-consumed phase value. This child draws no RNG. */
    uint16_t phase_random;
    /* af97 when true (M3/BMP only); complete afa2 when false. */
    bool missile_only;
} fist_automatic_fire_request;

typedef struct {
    fist_pool_allocation missile;
    fist_voice_request sound;
    uint8_t rack;
    bool requested;
    bool armed;
    bool launched;
    bool message;
} fist_automatic_fire_result;

#endif
