#ifndef FIST_SIM_WORLD_STEP_H
#define FIST_SIM_WORLD_STEP_H

#include "sim/object_pool.h"

#include <stdbool.h>
#include <stdint.h>

enum { FIST_WORLD_VISIT = 0, FIST_WORLD_END = 1, FIST_WORLD_NO_COMMAND = UINT16_MAX };

typedef struct {
    uint16_t tick;
    /* Original 969e and 969c; their consuming actions remain separate owners. */
    uint16_t auxiliary_countdown[2];
    uint16_t voice_at;
    uint16_t last_voice_timer;
    /* Original 6da6..6da8: minutes, seconds, subticks. Minutes ff disables it. */
    uint8_t mission_countdown[3];
    uint8_t pending_voice;
    uint8_t voice_mode;
} fist_world_clock;

typedef struct {
    bool voice_due;
    /* Exact e2c2 producer AX, or NO_COMMAND. DL=0 and ECX=0. Not mixed PCM. */
    uint16_t voice_command;
} fist_world_tick;

typedef struct {
    uint16_t next_entry;
} fist_world_pass;

/* Complete common c0e5 prefix up to the registry pass. Consume the scheduled
 * voice even if muted; return its explicit command for the shared audio owner.
 * device_timer is original ISR word 0452, separate from simulation tick 6cde.
 * Returns 0, or -1 preserving clock/output on null input. All byte/word states
 * retain their actual widths and wrap; no invented timer-domain restrictions. */
int fist_world_begin_tick(fist_world_clock *clock, uint16_t device_timer, fist_world_tick *out);

/* Zero-initialize a pass before visiting objects. Each successful call advances
 * before the caller runs the returned current class method. Read pool afresh on
 * every call, so later-entry births run in this pass and earlier/current births
 * wait. An end preserves allocation output; invalid input preserves both pass
 * and output. This iterator owns no class methods or payloads. The live-world
 * owner must dispatch actual methods and stage its complete transaction. */
int fist_world_next(const fist_object_pool *pool, fist_world_pass *pass, fist_pool_allocation *out);

#endif
