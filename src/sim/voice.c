#include "sim/voice.h"

#include <stddef.h>
#include <stdint.h>

int fist_voice_admit(fist_voice_history *history, fist_voice_environment environment,
                     uint8_t candidate, fist_voice_request *out) {
    enum {
        SIDE = 8,
        INTERVAL = 30,
        REQUEST_FLAG = 128,
        REQUEST_CLASS = 2,
        HIGH_BYTE = 0xff00,
        BYTE_SHIFT = 8
    };
    if (history == NULL || out == NULL) {
        return -1;
    }
    fist_voice_request request = {0};
    if (environment.gate == UINT16_MAX && environment.selected &&
        (environment.object_flags & SIDE) == 0 && candidate != FIST_VOICE_NONE &&
        (uint16_t)(environment.tick - history->admitted_at) >= INTERVAL) {
        history->admitted_at = environment.tick;
        request = (fist_voice_request){
            .ax = (uint16_t)((REQUEST_CLASS << BYTE_SHIFT) | (candidate | REQUEST_FLAG)),
            .dx = (uint16_t)(environment.tick & HIGH_BYTE),
            .emitted = true};
    }
    *out = request;
    return 0;
}
