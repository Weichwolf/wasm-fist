#include "sim/rotation.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    HALF_TURN = 32768,
    QUARTER_TURN = 16384,
    KNOT_INTERVAL = 64,
    FULL_COEFFICIENT = 65536,
    FRACTION_SCALE = 256,
    FRACTION_ROUND = 128
};

/* Original SS:2042, including the wrapped endpoint used by interpolation.
 * This is immutable numeric data, independent of DOS register state. */
static const uint16_t quarter_wave[] = {
    0,     402,   804,   1206,  1608,  2010,  2412,  2814,  3216,  3617,  4019,  4420,  4821,
    5222,  5623,  6023,  6424,  6824,  7224,  7623,  8022,  8421,  8820,  9218,  9616,  10014,
    10411, 10808, 11204, 11600, 11996, 12391, 12785, 13180, 13573, 13966, 14359, 14751, 15143,
    15534, 15924, 16314, 16703, 17091, 17479, 17867, 18253, 18639, 19024, 19409, 19792, 20175,
    20557, 20939, 21320, 21699, 22078, 22457, 22834, 23210, 23586, 23961, 24335, 24708, 25080,
    25451, 25821, 26190, 26558, 26925, 27291, 27656, 28020, 28383, 28745, 29106, 29466, 29824,
    30182, 30538, 30893, 31248, 31600, 31952, 32303, 32652, 33000, 33347, 33692, 34037, 34380,
    34721, 35062, 35401, 35738, 36075, 36410, 36744, 37076, 37407, 37736, 38064, 38391, 38716,
    39040, 39362, 39683, 40002, 40320, 40636, 40951, 41264, 41576, 41886, 42194, 42501, 42806,
    43110, 43412, 43713, 44011, 44308, 44604, 44898, 45190, 45480, 45769, 46056, 46341, 46624,
    46906, 47186, 47464, 47741, 48015, 48288, 48559, 48828, 49095, 49361, 49624, 49886, 50146,
    50404, 50660, 50914, 51166, 51417, 51665, 51911, 52156, 52398, 52639, 52878, 53114, 53349,
    53581, 53812, 54040, 54267, 54491, 54714, 54934, 55152, 55368, 55582, 55794, 56004, 56212,
    56418, 56621, 56823, 57022, 57219, 57414, 57607, 57798, 57986, 58172, 58356, 58538, 58718,
    58896, 59071, 59244, 59415, 59583, 59750, 59914, 60075, 60235, 60392, 60547, 60700, 60851,
    60999, 61145, 61288, 61429, 61568, 61705, 61839, 61971, 62101, 62228, 62353, 62476, 62596,
    62714, 62830, 62943, 63054, 63162, 63268, 63372, 63473, 63572, 63668, 63763, 63854, 63944,
    64031, 64115, 64197, 64277, 64354, 64429, 64501, 64571, 64639, 64704, 64766, 64827, 64884,
    64940, 64993, 65043, 65091, 65137, 65180, 65220, 65259, 65294, 65328, 65358, 65387, 65413,
    65436, 65457, 65476, 65492, 65505, 65516, 65525, 65531, 65535, 0};

static uint32_t coefficient(uint16_t angle, bool coarse) {
    uint16_t folded = angle % HALF_TURN;
    if (folded == QUARTER_TURN) {
        return FULL_COEFFICIENT;
    }
    if (folded > QUARTER_TURN) {
        folded = (uint16_t)(HALF_TURN - folded);
    }
    const size_t index = folded / KNOT_INTERVAL;
    const uint16_t lower = quarter_wave[index];
    if (coarse) {
        return lower;
    }
    const uint16_t difference = (uint16_t)(quarter_wave[index + 1] - lower);
    const uint32_t fraction = (uint32_t)(folded % KNOT_INTERVAL) * (FRACTION_SCALE / KNOT_INTERVAL);
    return lower + (((uint32_t)difference * fraction + FRACTION_ROUND) / FRACTION_SCALE);
}

static int16_t lane(uint16_t angle, const fist_rotation *rotation) {
    const int32_t magnitude = rotation->magnitude;
    const uint32_t absolute = (uint32_t)(magnitude < 0 ? -magnitude : magnitude);
    const uint32_t product = coefficient(angle, rotation->coarse) * absolute;
    int32_t value = (int32_t)(product / FULL_COEFFICIENT);
    if ((angle >= HALF_TURN) != (magnitude < 0)) {
        value = -value;
    }
    /* Only magnitude -32768 can produce +32768: the original word wraps. */
    if (value > INT16_MAX) {
        value -= FULL_COEFFICIENT;
    }
    return (int16_t)value;
}

fist_velocity fist_rotate(fist_rotation rotation) {
    return (fist_velocity){.x = lane(rotation.heading, &rotation),
                           .y = lane((uint16_t)(QUARTER_TURN - rotation.heading), &rotation)};
}
