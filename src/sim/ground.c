#include "sim/ground.h"

#include "assets/klc.h"
#include "assets/units.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"

#include <stddef.h>
#include <stdint.h>

enum {
    MAX_INDEX_BITS = 16,
    TURN_SAMPLES = 512,
    QUARTER_TURN_SAMPLES = TURN_SAMPLES / 4,
    HEADING_BIN = FIST_TURN_SIZE / TURN_SAMPLES,
    SLOPE_SCALE = 128,
    BYTE_RANGE = 256
};

/* Original 9450 Q31 sine data after the sampler's arithmetic shift by 6.
 * All 512 offsets are retained: the original is not an exactly symmetric
 * quarter-wave table. Its cosine tail repeats the first 128 sine entries.
 * Original 640-word table SHA256:
 * 83fe0d0d21507728a3323eff27ce933234df3cb582ab047851f811fb9eaf4050. */
static const int32_t sample_offsets[TURN_SAMPLES] = {
    0,         411764,    823466,    1235045,   1646437,   2057582,   2468417,   2878880,
    3288909,   3698443,   4107420,   4515779,   4923457,   5330394,   5736529,   6141799,
    6546144,   6949504,   7351817,   7753023,   8153061,   8551872,   8949395,   9345569,
    9740337,   10133638,  10525412,  10915602,  11304147,  11690990,  12076073,  12459337,
    12840725,  13220179,  13597642,  13973057,  14346368,  14717518,  15086452,  15453114,
    15817449,  16179402,  16538918,  16895944,  17250425,  17602308,  17951541,  18298070,
    18641843,  18982809,  19320916,  19656114,  19988351,  20317579,  20643746,  20966805,
    21286706,  21603401,  21916843,  22226985,  22533779,  22837179,  23137141,  23433618,
    23726566,  24015941,  24301699,  24583797,  24862194,  25136846,  25407712,  25674752,
    25937926,  26197194,  26452516,  26703855,  26951172,  27194430,  27433594,  27668625,
    27899490,  28126153,  28348581,  28566739,  28780596,  28990118,  29195274,  29396034,
    29592367,  29784243,  29971634,  30154511,  30332847,  30506615,  30675789,  30840343,
    31000252,  31155494,  31306043,  31451878,  31592976,  31729316,  31860878,  31987642,
    32109589,  32226700,  32338958,  32446346,  32548847,  32646447,  32739130,  32826883,
    32909693,  32987546,  33060431,  33128338,  33191255,  33249175,  33302087,  33349983,
    33392858,  33430703,  33463514,  33491286,  33514014,  33531695,  33544326,  33551905,
    33554431,  33551905,  33544326,  33531695,  33514014,  33491286,  33463514,  33430703,
    33392858,  33349983,  33302087,  33249175,  33191255,  33128338,  33060431,  32987546,
    32909693,  32826883,  32739130,  32646447,  32548847,  32446346,  32338958,  32226700,
    32109589,  31987642,  31860878,  31729316,  31592976,  31451878,  31306043,  31155494,
    31000252,  30840343,  30675789,  30506615,  30332847,  30154511,  29971634,  29784243,
    29592367,  29396034,  29195274,  28990118,  28780596,  28566739,  28348581,  28126153,
    27899490,  27668625,  27433594,  27194430,  26951172,  26703855,  26452516,  26197194,
    25937926,  25674752,  25407712,  25136846,  24862194,  24583797,  24301699,  24015941,
    23726566,  23433618,  23137141,  22837179,  22533779,  22226985,  21916843,  21603401,
    21286706,  20966805,  20643746,  20317579,  19988351,  19656114,  19320916,  18982809,
    18641843,  18298070,  17951541,  17602308,  17250425,  16895944,  16538918,  16179402,
    15817449,  15453114,  15086452,  14717518,  14346368,  13973057,  13597642,  13220179,
    12840725,  12459337,  12076073,  11690990,  11304147,  10915602,  10525412,  10133638,
    9740337,   9345570,   8949395,   8551872,   8153061,   7753023,   7351817,   6949504,
    6546144,   6141799,   5736529,   5330394,   4923457,   4515779,   4107420,   3698443,
    3288909,   2878880,   2468417,   2057582,   1646437,   1235045,   823467,    411764,
    0,         -411765,   -823467,   -1235046,  -1646438,  -2057583,  -2468418,  -2878881,
    -3288910,  -3698444,  -4107421,  -4515780,  -4923458,  -5330395,  -5736530,  -6141800,
    -6546145,  -6949505,  -7351818,  -7753024,  -8153062,  -8551873,  -8949396,  -9345570,
    -9740338,  -10133639, -10525413, -10915603, -11304148, -11690991, -12076074, -12459338,
    -12840726, -13220180, -13597643, -13973058, -14346369, -14717519, -15086453, -15453115,
    -15817450, -16179403, -16538919, -16895945, -17250426, -17602309, -17951542, -18298071,
    -18641844, -18982810, -19320917, -19656115, -19988352, -20317580, -20643747, -20966806,
    -21286707, -21603402, -21916844, -22226986, -22533780, -22837180, -23137142, -23433619,
    -23726567, -24015942, -24301700, -24583798, -24862195, -25136847, -25407713, -25674753,
    -25937927, -26197195, -26452517, -26703856, -26951173, -27194431, -27433594, -27668626,
    -27899491, -28126154, -28348582, -28566740, -28780597, -28990119, -29195275, -29396035,
    -29592368, -29784244, -29971635, -30154512, -30332848, -30506616, -30675790, -30840344,
    -31000253, -31155495, -31306044, -31451879, -31592977, -31729317, -31860879, -31987643,
    -32109590, -32226701, -32338959, -32446347, -32548848, -32646448, -32739131, -32826884,
    -32909693, -32987547, -33060432, -33128339, -33191256, -33249176, -33302088, -33349984,
    -33392859, -33430704, -33463515, -33491287, -33514015, -33531695, -33544327, -33551906,
    -33554432, -33551906, -33544327, -33531695, -33514015, -33491287, -33463515, -33430704,
    -33392859, -33349984, -33302088, -33249176, -33191256, -33128339, -33060432, -32987547,
    -32909693, -32826884, -32739131, -32646448, -32548848, -32446347, -32338959, -32226701,
    -32109590, -31987643, -31860879, -31729317, -31592977, -31451879, -31306044, -31155495,
    -31000253, -30840344, -30675790, -30506616, -30332848, -30154512, -29971635, -29784244,
    -29592368, -29396035, -29195275, -28990119, -28780597, -28566740, -28348582, -28126154,
    -27899491, -27668626, -27433595, -27194431, -26951173, -26703856, -26452517, -26197195,
    -25937927, -25674753, -25407713, -25136847, -24862195, -24583798, -24301700, -24015942,
    -23726567, -23433619, -23137142, -22837180, -22533780, -22226986, -21916844, -21603402,
    -21286707, -20966806, -20643747, -20317580, -19988352, -19656115, -19320917, -18982810,
    -18641844, -18298071, -17951542, -17602309, -17250426, -16895945, -16538919, -16179403,
    -15817450, -15453115, -15086453, -14717519, -14346369, -13973058, -13597643, -13220180,
    -12840726, -12459338, -12076074, -11690991, -11304148, -10915603, -10525413, -10133639,
    -9740338,  -9345571,  -8949396,  -8551873,  -8153062,  -7753024,  -7351818,  -6949505,
    -6546145,  -6141800,  -5736530,  -5330395,  -4923458,  -4515780,  -4107421,  -3698444,
    -3288910,  -2878881,  -2468418,  -2057583,  -1646438,  -1235046,  -823468,   -411765,
};

static int index_bits(const fist_klc_image *height, unsigned *out) {
    if (height == NULL || height->pixels == NULL || height->width == 0 ||
        height->width != height->height || (height->width & (height->width - 1)) != 0 ||
        (size_t)height->width > SIZE_MAX / height->height) {
        return -1;
    }
    unsigned bits = 0;
    for (uint32_t side = height->width; side > 1; side /= 2) {
        ++bits;
    }
    if (bits > MAX_INDEX_BITS) {
        return -1;
    }
    *out = bits;
    return 0;
}

typedef struct {
    uint32_t x;
    uint32_t y;
} sample_point;

static uint8_t sample(const fist_klc_image *height, sample_point point, unsigned bits) {
    if (bits == 0) {
        return height->pixels[0];
    }
    const size_t column = point.x >> (FIST_MAP_FIXED_BITS - bits);
    const size_t row = point.y >> (FIST_MAP_FIXED_BITS - bits);
    return height->pixels[(row * height->width) + column];
}

static int16_t slope(uint8_t first, uint8_t second) {
    const uint8_t wrapped = (uint8_t)((unsigned)first - second);
    const int difference = wrapped <= INT8_MAX ? wrapped : (int)wrapped - BYTE_RANGE;
    return (int16_t)(difference * SLOPE_SCALE);
}

int fist_ground_sample(const fist_klc_image *height, const fist_ground_pose *pose,
                       fist_ground_contact *out) {
    unsigned bits = 0;
    if (pose == NULL || out == NULL || index_bits(height, &bits) != 0) {
        return -1;
    }
    const sample_point point = {.x = (uint32_t)pose->map_x << FIST_MAP_FIXED_SHIFT,
                                .y = 0U - ((uint32_t)pose->map_y << FIST_MAP_FIXED_SHIFT)};
    const uint16_t inverse_heading = (uint16_t)(0U - pose->heading);
    const size_t direction = inverse_heading / HEADING_BIN;
    const uint32_t sine = (uint32_t)sample_offsets[direction];
    const uint32_t cosine =
        (uint32_t)sample_offsets[(direction + QUARTER_TURN_SAMPLES) % TURN_SAMPLES];
    const uint8_t left = sample(height, (sample_point){point.x - cosine, point.y + sine}, bits);
    const uint8_t right = sample(height, (sample_point){point.x + cosine, point.y - sine}, bits);
    const uint8_t forward = sample(height, (sample_point){point.x - sine, point.y - cosine}, bits);
    const uint8_t backward = sample(height, (sample_point){point.x + sine, point.y + cosine}, bits);
    *out = (fist_ground_contact){.height = sample(height, point, bits),
                                 .roll = slope(left, right),
                                 .pitch = slope(forward, backward)};
    return 0;
}

int fist_vehicle_ground_update(fist_vehicle_state *vehicle, const fist_klc_image *height) {
    if (vehicle == NULL || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        vehicle->component_size != fist_vehicle_component_size(vehicle->type)) {
        return -1;
    }
    fist_ground_pose pose = {
        .map_x = vehicle->map_x, .map_y = vehicle->map_y, .heading = vehicle->drive.heading};
    fist_ground_contact hull = {0};
    fist_ground_contact turret = {0};
    if (fist_ground_sample(height, &pose, &hull) != 0) {
        return -1;
    }
    pose.heading = vehicle->turret.heading;
    if (fist_ground_sample(height, &pose, &turret) != 0) {
        return -1;
    }
    vehicle->ground_height = hull.height;
    vehicle->drive.terrain_roll = hull.roll;
    vehicle->drive.terrain_pitch = hull.pitch;
    vehicle->turret.terrain_roll = turret.roll;
    vehicle->turret.terrain_pitch = turret.pitch;
    return 0;
}
