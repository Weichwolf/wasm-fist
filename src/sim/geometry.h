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

#endif
