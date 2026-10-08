#ifndef FIST_SIM_GROUND_SUPPORT_H
#define FIST_SIM_GROUND_SUPPORT_H

#include "assets/orders.h"
#include "sim/object_pool.h"
#include "sim/voice.h"
#include "sim/weapon_control.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    FIST_SUPPORT_QUEUE_SLOTS = 16,
    FIST_SUPPORT_NO_QUEUE = UINT8_MAX,
    FIST_SUPPORT_SMOKE_TYPE = 20
};

typedef struct {
    fist_pool_allocation allocation;
    fist_object_reference reference;
} fist_artillery_resource;

/* Already-decoded catalog or explicit manual-mission inputs. Delays are the
 * wrapped producer products, not guessed durations or a campaign parser. */
typedef struct {
    uint16_t air_stock[2];
    uint16_t air_delay[2];
    uint16_t artillery_delay[2];
    bool reverse_aircraft;
} fist_support_configuration;

/* A retained expired requester still occupies its entry until the later
 * dispatcher processes it. Reset clears admission, retaining other fields. */
typedef struct {
    fist_object_reference requester;
    uint16_t tick;
} fist_air_support_entry;

typedef struct {
    uint16_t phase;
    uint16_t clock;
    fist_order_waypoint target;
} fist_artillery_support_entry;

typedef struct {
    fist_support_configuration configuration;
    uint16_t last_request;
    uint16_t air_clock[2];
    uint16_t artillery_clock[2];
    uint16_t air_type[2];
    fist_air_support_entry air[2][FIST_SUPPORT_QUEUE_SLOTS];
    fist_artillery_support_entry artillery[2][FIST_SUPPORT_QUEUE_SLOTS];
    bool configured;
} fist_mission_support;

/* Original 9fd6/9fd7 advisory, distinct from the 969e/96a0 display owner. */
typedef struct {
    uint16_t until;
    uint8_t code;
    bool active;
} fist_timed_advisory;

/* Complete type-20 constructor. Its later update/dispatch method remains open;
 * creation stores sampled height in altitude bits 8..15, not ground_height. */
typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t projection_extent;
    uint16_t projection_scale;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
    uint8_t variant;
} fist_support_marker;

typedef struct {
    uint16_t slot;
    uint16_t clock;
    uint16_t tick;
    uint16_t voice_gate;
    uint8_t notice_context;
} fist_ground_station_request;

typedef struct {
    fist_weapon_events weapon;
    fist_voice_request voice;
    bool notice;
} fist_ground_station_result;

enum {
    FIST_SUPPORT_NONE,
    FIST_SUPPORT_AIR_COOLDOWN,
    FIST_SUPPORT_AIR_UNAVAILABLE,
    FIST_SUPPORT_AIR_CONFIRMED,
    FIST_SUPPORT_ARTILLERY_COOLDOWN,
    FIST_SUPPORT_ARTILLERY_NOT_IN_PLACE,
    FIST_SUPPORT_ARTILLERY_EMPTY,
    FIST_SUPPORT_ARTILLERY_BUSY,
    FIST_SUPPORT_ARTILLERY_QUEUE_FULL,
    FIST_SUPPORT_ARTILLERY_CONFIRMED
};

enum {
    FIST_SUPPORT_SMOKE_NONE,
    FIST_SUPPORT_SMOKE_EMPTY,
    FIST_SUPPORT_SMOKE_CAPACITY,
    FIST_SUPPORT_SMOKE_CREATED
};

typedef struct {
    uint16_t slot;
    uint16_t sound_source;
    uint16_t clock;
    uint16_t tick;
    uint8_t notice_context;
    bool coarse;
} fist_ground_support_request;

typedef struct {
    fist_pool_allocation marker;
    fist_voice_request sound;
    uint16_t resource_slot;
    uint8_t support;
    uint8_t smoke;
    uint8_t queue_index;
    bool notice;
    bool message;
} fist_ground_support_result;

#endif
