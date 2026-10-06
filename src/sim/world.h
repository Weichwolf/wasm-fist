#ifndef FIST_SIM_WORLD_H
#define FIST_SIM_WORLD_H

#include <stdint.h>

/* Original map coordinates: 32-bit sampler inputs after a 13-bit shift.
 * Rendering divides map positions/altitudes by 256; headings use full u16 turns. */
enum {
    FIST_MAP_FIXED_BITS = 32,
    FIST_MAP_FIXED_SHIFT = 13,
    FIST_MAP_PERIOD = 1 << (FIST_MAP_FIXED_BITS - FIST_MAP_FIXED_SHIFT),
    FIST_POSITION_SCALE = 256,
    FIST_TURN_SIZE = 65536
};

/* Shared typed world pose, borrowed by collision and owned by live payloads. */
typedef struct {
    int32_t x;
    int32_t y;
    int32_t altitude;
    uint16_t heading;
} fist_object_pose;

/* Original DWORD position addition, with portable signed conversion. */
static inline int32_t fist_position_add(int32_t position, int32_t offset) {
    const uint32_t value = (uint32_t)position + (uint32_t)offset;
    return value <= INT32_MAX ? (int32_t)value : -1 - (int32_t)(UINT32_MAX - value);
}

#endif
