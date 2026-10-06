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

/* Original planar rotation 03a9: heading zero follows +Y, quarter turn +X.
 * Interpolation and full-scale coefficients preserve signed magnitudes. */
fist_velocity fist_rotate(fist_rotation rotation);

#endif
