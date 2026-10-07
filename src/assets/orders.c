#include "assets/orders.h"

#include "assets/bytes.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>

int fist_mission_orders_decode(const fist_scenario *scenario, fist_mission_orders *out) {
    enum { WORD_BYTES = 2, COORDINATE_BYTES = 4, POINT_BYTES = 8 };
    if (scenario == NULL || out == NULL) {
        return -1;
    }
    const fist_asset_view paths = scenario->chunks[FIST_SCENARIO_PATHS];
    const fist_asset_view descriptors = scenario->chunks[FIST_SCENARIO_PLAYER_INFO];
    if (paths.data == NULL || paths.size != FIST_ORDER_PATH_BLOCK_BYTES ||
        descriptors.data == NULL || descriptors.size != FIST_ORDER_DESCRIPTOR_BLOCK_BYTES) {
        return -1;
    }
    fist_mission_orders orders = {0};
    for (size_t platoon = 0; platoon < FIST_UNIT_PLATOON_COUNT; ++platoon) {
        const uint8_t *path = paths.data + (platoon * FIST_ORDER_PATH_BYTES);
        fist_order_route *route = &orders.routes[platoon];
        if (path[0] > FIST_ORDER_WAYPOINTS) {
            return -1;
        }
        route->count = path[0];
        for (size_t index = 0; index < FIST_ORDER_HEADER_BYTES; ++index) {
            route->header[index] = path[index + 1];
        }
        for (size_t index = 0; index < FIST_ORDER_WAYPOINTS; ++index) {
            const uint8_t *point = path + 1 + FIST_ORDER_HEADER_BYTES + (index * POINT_BYTES);
            route->points[index] = (fist_order_waypoint){fist_read_i32le(point),
                                                         fist_read_i32le(point + COORDINATE_BYTES)};
        }
        const uint8_t *descriptor =
            descriptors.data + (platoon * FIST_ORDER_DESCRIPTOR_WORDS * WORD_BYTES);
        for (size_t index = 0; index < FIST_ORDER_DESCRIPTOR_WORDS; ++index) {
            orders.descriptors[platoon].words[index] =
                fist_read_u16le(descriptor + (index * WORD_BYTES));
        }
    }
    *out = orders;
    return 0;
}
