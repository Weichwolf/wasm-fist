#ifndef FIST_ASSETS_ORDERS_H
#define FIST_ASSETS_ORDERS_H

#include "assets/scenario.h"
#include "assets/units.h"

#include <stdint.h>

enum {
    FIST_ORDER_WAYPOINTS = 32,
    FIST_ORDER_HEADER_BYTES = 11,
    FIST_ORDER_DESCRIPTOR_WORDS = 11,
    FIST_ORDER_PATH_BYTES = 1 + FIST_ORDER_HEADER_BYTES + (FIST_ORDER_WAYPOINTS * 8),
    FIST_ORDER_PATH_BLOCK_BYTES = FIST_UNIT_PLATOON_COUNT * FIST_ORDER_PATH_BYTES,
    FIST_ORDER_DESCRIPTOR_BLOCK_BYTES = FIST_UNIT_PLATOON_COUNT * FIST_ORDER_DESCRIPTOR_WORDS * 2
};

typedef struct {
    int32_t x;
    int32_t y;
} fist_order_waypoint;

typedef struct {
    uint8_t count;
    /* Retained original header bytes; no invented semantics. */
    uint8_t header[FIST_ORDER_HEADER_BYTES];
    /* Includes every saved slot beyond count, needed for complete persistence. */
    fist_order_waypoint points[FIST_ORDER_WAYPOINTS];
} fist_order_route;

typedef struct {
    /* Behavior, waypoint mode, formation, throttle, then retained unknown words.
     * The decoder preserves full-width words; consumers own selector admission. */
    uint16_t words[FIST_ORDER_DESCRIPTOR_WORDS];
} fist_order_descriptor;

typedef struct {
    fist_order_route routes[FIST_UNIT_PLATOON_COUNT];
    fist_order_descriptor descriptors[FIST_UNIT_PLATOON_COUNT];
} fist_mission_orders;

/* Decode exact complete PATH/PINF blocks from a live decoded scenario. Owns all
 * values, retains no source views, and does not consume RNG. Return 0 on success,
 * -1 for null/missing/malformed input, preserving out on every failure. */
int fist_mission_orders_decode(const fist_scenario *scenario, fist_mission_orders *out);

#endif
