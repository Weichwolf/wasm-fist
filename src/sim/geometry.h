#ifndef FIST_SIM_GEOMETRY_H
#define FIST_SIM_GEOMETRY_H

#include "assets/orders.h"

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

/* Complete original a17e/08e8 proximity return used by target discovery.
 * Uses wrapped target-minus-source DWORDs, one's-complement negative lanes,
 * then larger lane plus half the smaller. This directional estimate differs
 * from 0541 navigation distance; callers retain the original argument order.
 * Numeric scratch is local; inputs/state remain unchanged. */
uint32_t fist_planar_proximity(fist_order_waypoint source, fist_order_waypoint target);

#endif
