#include "sim/geometry.h"

#include "assets/orders.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    WORD_BITS = 16,
    BYTE_BITS = 8,
    LARGE_LANE_BITS = 24,
    SIGN_BIT = 31,
    ROOT_START_BIT = 30,
    OCTANT_SCALE = 65536,
    OCTANTS = 8,
    ANGLE_FRACTION = 256,
    ANGLE_ROUND = 128,
    TURN_ROUND = 4,
    DISTANCE_SMALL_BITS = 32,
    DISTANCE_MEDIUM_BITS = 40,
    DISTANCE_LARGE_BITS = 48,
    DISTANCE_MEDIUM_SCALE = 4,
    DISTANCE_LARGE_SCALE = 8,
    DISTANCE_HIGH_SCALE = 16
};

/* Original SS:2448: 257 authored atan knots, including the wrapped endpoint.
 * Keep this immutable numeric data independent of guest registers/scratch. */
static const uint16_t angle_knots[] = {
    0,     326,   652,   978,   1304,  1630,  1955,  2281,  2607,  2932,  3258,  3583,  3909,
    4234,  4559,  4884,  5208,  5533,  5857,  6182,  6506,  6830,  7153,  7477,  7800,  8123,
    8446,  8768,  9090,  9412,  9734,  10055, 10377, 10697, 11018, 11338, 11658, 11977, 12296,
    12615, 12933, 13251, 13569, 13886, 14203, 14519, 14835, 15151, 15466, 15781, 16095, 16409,
    16722, 17035, 17347, 17659, 17970, 18281, 18591, 18901, 19210, 19519, 19827, 20135, 20442,
    20748, 21054, 21360, 21664, 21968, 22272, 22575, 22877, 23179, 23480, 23781, 24081, 24380,
    24678, 24976, 25274, 25570, 25866, 26161, 26456, 26750, 27043, 27336, 27628, 27919, 28209,
    28499, 28788, 29076, 29364, 29651, 29937, 30222, 30507, 30791, 31074, 31356, 31638, 31919,
    32199, 32479, 32757, 33035, 33312, 33589, 33864, 34139, 34413, 34686, 34958, 35230, 35501,
    35771, 36040, 36308, 36576, 36843, 37109, 37374, 37639, 37902, 38165, 38427, 38688, 38949,
    39208, 39467, 39725, 39982, 40238, 40493, 40748, 41002, 41255, 41507, 41758, 42009, 42258,
    42507, 42755, 43003, 43249, 43494, 43739, 43983, 44226, 44468, 44710, 44950, 45190, 45429,
    45667, 45904, 46141, 46376, 46611, 46845, 47078, 47311, 47542, 47773, 48003, 48232, 48460,
    48687, 48914, 49140, 49365, 49589, 49812, 50035, 50257, 50478, 50698, 50917, 51136, 51353,
    51570, 51786, 52002, 52216, 52430, 52643, 52855, 53066, 53277, 53487, 53696, 53904, 54111,
    54318, 54524, 54729, 54933, 55137, 55340, 55542, 55743, 55943, 56143, 56342, 56540, 56738,
    56935, 57131, 57326, 57520, 57714, 57907, 58099, 58291, 58481, 58671, 58861, 59049, 59237,
    59424, 59611, 59796, 59981, 60166, 60349, 60532, 60714, 60896, 61076, 61256, 61436, 61614,
    61792, 61969, 62146, 62322, 62497, 62671, 62845, 63018, 63191, 63363, 63534, 63704, 63874,
    64043, 64212, 64379, 64547, 64713, 64879, 65044, 65209, 65373, 0};

static uint64_t squared_lane(uint32_t delta) {
    const uint32_t magnitude = delta <= INT32_MAX ? delta : 0U - delta;
    unsigned shift = 0;
    if (magnitude >= (UINT32_C(1) << LARGE_LANE_BITS)) {
        shift = WORD_BITS;
    } else if (magnitude >= (UINT32_C(1) << WORD_BITS)) {
        shift = BYTE_BITS;
    }
    const uint64_t lane = magnitude >> shift;
    return (lane * lane) << (shift * 2);
}

static uint32_t integer_root(uint32_t squared) {
    uint32_t result = 0;
    uint32_t bit = UINT32_C(1) << ROOT_START_BIT;
    while (bit > squared) {
        bit >>= 2;
    }
    while (bit != 0) {
        if (squared >= result + bit) {
            squared -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

static uint32_t distance(uint32_t delta_x, uint32_t delta_y) {
    const uint64_t squared = squared_lane(delta_x) + squared_lane(delta_y);
    unsigned scale = DISTANCE_HIGH_SCALE;
    if (squared < (UINT64_C(1) << DISTANCE_SMALL_BITS)) {
        scale = 0;
    } else if (squared < (UINT64_C(1) << DISTANCE_MEDIUM_BITS)) {
        scale = DISTANCE_MEDIUM_SCALE;
    } else if (squared < (UINT64_C(1) << DISTANCE_LARGE_BITS)) {
        scale = DISTANCE_LARGE_SCALE;
    }
    /* Original 0927 chooses a 32-bit root input at these exact size boundaries;
     * low coordinate/squared bits are discarded before taking the root. */
    return integer_root((uint32_t)(squared >> (scale * 2))) << scale;
}

static uint16_t interpolated_angle(uint16_t ratio, bool coarse) {
    const size_t index = ratio / ANGLE_FRACTION;
    const uint16_t lower = angle_knots[index];
    if (coarse) {
        return lower;
    }
    const uint16_t difference = (uint16_t)(angle_knots[index + 1] - lower);
    return (uint16_t)(lower + (((uint32_t)difference * (ratio % ANGLE_FRACTION) + ANGLE_ROUND) /
                               ANGLE_FRACTION));
}

static uint16_t heading(uint32_t delta_x, uint32_t delta_y, bool coarse) {
    uint32_t sector = 0;
    if ((delta_x >> SIGN_BIT) != 0) {
        delta_x = 0U - delta_x;
        delta_y = 0U - delta_y;
        sector = OCTANTS / 2;
    }
    if ((delta_y >> SIGN_BIT) != 0) {
        const uint32_t old_x = delta_x;
        delta_x = 0U - delta_y;
        delta_y = old_x;
        sector += OCTANTS / 4;
    }
    /* Fold using tan(a - pi/4) = (x - y)/(x + y). This also preserves original
     * 32-bit signed-extreme/zero behavior: 07a5 returns zero if the sum wraps. */
    while (delta_x >= delta_y) {
        delta_x -= delta_y;
        delta_y = (delta_y * 2) + delta_x;
        ++sector;
        if (delta_y == 0) {
            return 0;
        }
    }
    while ((delta_y >> SIGN_BIT) == 0) {
        delta_x <<= 1;
        delta_y <<= 1;
    }
    const uint32_t denominator = delta_y >> WORD_BITS;
    const uint32_t limit = denominator << WORD_BITS;
    /* Original 07e3..07ec saturates the division numerator at the last legal
     * word quotient; the normalized denominator deliberately drops low bits. */
    if (delta_x >= limit) {
        delta_x = limit - 1;
    }
    const uint16_t ratio = (uint16_t)(delta_x / denominator);
    const uint32_t angle = interpolated_angle(ratio, coarse);
    return (uint16_t)(((sector * OCTANT_SCALE) + angle + TURN_ROUND) / OCTANTS);
}

fist_planar_measurement fist_planar_measure(fist_order_waypoint source, fist_order_waypoint target,
                                            bool coarse) {
    const uint32_t delta_x = (uint32_t)target.x - (uint32_t)source.x;
    const uint32_t delta_y = (uint32_t)target.y - (uint32_t)source.y;
    return (fist_planar_measurement){heading(delta_x, delta_y, coarse), distance(delta_x, delta_y)};
}

uint32_t fist_planar_proximity(fist_order_waypoint source, fist_order_waypoint target) {
    const uint32_t delta_x = (uint32_t)target.x - (uint32_t)source.x;
    const uint32_t delta_y = (uint32_t)target.y - (uint32_t)source.y;
    const uint32_t lane_x = delta_x <= INT32_MAX ? delta_x : ~delta_x;
    const uint32_t lane_y = delta_y <= INT32_MAX ? delta_y : ~delta_y;
    if (lane_x >= lane_y) {
        return lane_x + (lane_y >> 1U);
    }
    return lane_y + (lane_x >> 1U);
}
