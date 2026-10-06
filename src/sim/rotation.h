#ifndef FIST_SIM_ROTATION_H
#define FIST_SIM_ROTATION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t heading;
    int16_t magnitude;
    /* Original temporary SS:2040 mode: true selects the table's lower knot. */
    bool coarse;
} fist_rotation;

typedef struct {
    int16_t x;
    int16_t y;
} fist_velocity;

typedef struct {
    uint16_t heading;
    uint16_t elevation;
    int16_t magnitude;
    bool coarse;
} fist_spatial_rotation;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} fist_spatial_velocity;

/* Original planar rotation 03a9: heading zero follows +Y, quarter turn +X.
 * Interpolation and full-scale coefficients preserve signed magnitudes. */
fist_velocity fist_rotate(fist_rotation rotation);

/* Original spatial rotation 0459: elevation zero is horizontal, quarter turn
 * follows +Z. First truncate the unsigned horizontal magnitude, then rotate
 * it in the XY plane. Its full-scale horizontal coefficient is 65535; Z and
 * final XY lanes use 65536. Keep the sign separate until final word outputs. */
fist_spatial_velocity fist_rotate_spatial(fist_spatial_rotation rotation);

#endif
