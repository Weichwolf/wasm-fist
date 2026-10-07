#ifndef FIST_SIM_VOICE_H
#define FIST_SIM_VOICE_H

#include <stdbool.h>
#include <stdint.h>

enum { FIST_VOICE_NONE = UINT8_MAX };

typedef struct {
    /* Original shared DS:9fca, not a per-actor cooldown. */
    uint16_t admitted_at;
} fist_voice_history;

typedef struct {
    uint16_t gate;
    uint16_t tick;
    uint8_t object_flags;
    bool selected;
} fist_voice_environment;

typedef struct {
    uint16_t ax;
    uint16_t dx;
    uint32_t ecx;
    bool emitted;
} fist_voice_request;

/* Complete bf3c admission and op-64 logical request. Does not play or mix PCM.
 * gate is the actual 6da2 word predicate, not an inferred device meaning.
 * Reusable for target/fire/damage request candidates. Invalid input is atomic. */
int fist_voice_admit(fist_voice_history *history, fist_voice_environment environment,
                     uint8_t candidate, fist_voice_request *out);

#endif
