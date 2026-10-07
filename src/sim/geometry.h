#ifndef FIST_SIM_GEOMETRY_H
#define FIST_SIM_GEOMETRY_H

#include "assets/orders.h"
#include "sim/world.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t heading;
    uint32_t distance;
} fist_planar_measurement;

/* Complete original 0541 planar semantic return: wrapped DWORD coordinates,
 * full-turn heading and the original distance precision/scale. coarse selects
 * the lower atan knot. Numeric scratch is local; inputs/state are unchanged. */
fist_planar_measurement fist_planar_measure(fist_order_waypoint source, fist_order_waypoint target,
                                            bool coarse);

/* Original a19a/0927 distance alone, sharing the measurement's precision and
 * wrapped coordinates. No angle interpolation or world state is needed. */
uint32_t fist_planar_distance(fist_order_waypoint source, fist_order_waypoint target);

typedef struct {
    uint16_t heading;
    uint16_t elevation;
    uint32_t distance;
} fist_spatial_measurement;

/* Complete original 0578: planar heading/distance plus wrapped signed altitude
 * bearing. Reuses the same angle and distance owners; no visibility or state. */
fist_spatial_measurement fist_spatial_measure(fist_object_pose source, fist_object_pose target,
                                              bool coarse);

/* Complete original a17e/08e8 proximity return used by target discovery.
 * Uses wrapped target-minus-source DWORDs, one's-complement negative lanes,
 * then larger lane plus half the smaller. This directional estimate differs
 * from 0541 navigation distance; callers retain the original argument order.
 * Numeric scratch is local; inputs/state remain unchanged. */
uint32_t fist_planar_proximity(fist_order_waypoint source, fist_order_waypoint target);

#endif
