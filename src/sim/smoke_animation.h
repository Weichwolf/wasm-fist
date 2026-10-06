#ifndef FIST_SIM_SMOKE_ANIMATION_H
#define FIST_SIM_SMOKE_ANIMATION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t period;
    uint8_t last_frame;
} fist_smoke_animation_rule;

/* Internal counter/frame owner for validated type-17/18 updates. */
bool fist_smoke_animation_step(uint16_t *counter, uint8_t *frame, fist_smoke_animation_rule rule);

#endif
