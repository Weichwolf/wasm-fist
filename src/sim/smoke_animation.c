#include "sim/smoke_animation.h"

#include <stdbool.h>
#include <stdint.h>

bool fist_smoke_animation_step(uint16_t *counter, uint8_t *frame, fist_smoke_animation_rule rule) {
    *counter = (uint16_t)(*counter + 1);
    if (*counter < rule.period) {
        return false;
    }
    *counter = 0;
    *frame = (uint8_t)(*frame + 1);
    return *frame >= rule.last_frame;
}
