#ifndef FIST_SIM_WORLD_H
#define FIST_SIM_WORLD_H

/* Original map coordinates: 32-bit sampler inputs after a 13-bit shift.
 * Rendering divides map positions/altitudes by 256; headings use full u16 turns. */
enum {
    FIST_MAP_FIXED_BITS = 32,
    FIST_MAP_FIXED_SHIFT = 13,
    FIST_MAP_PERIOD = 1 << (FIST_MAP_FIXED_BITS - FIST_MAP_FIXED_SHIFT),
    FIST_POSITION_SCALE = 256,
    FIST_TURN_SIZE = 65536
};

#endif
