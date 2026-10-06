#include "sim/world_step.h"

#include "assets/units.h"
#include "sim/object_pool.h"

#include <stddef.h>
#include <stdint.h>

enum { DISABLED_COUNTDOWN = UINT8_MAX, BORROW_VALUE = 59, MUTED_VOICE = 2, VOICE_COMMAND = 0x280 };

static void mission_countdown(uint8_t time[3]) {
    if (time[0] == DISABLED_COUNTDOWN || (time[0] == 0 && time[1] == 0 && time[2] == 0)) {
        return;
    }
    time[2] = (uint8_t)(time[2] - 1);
    if (time[2] != UINT8_MAX) {
        return;
    }
    time[2] = BORROW_VALUE;
    time[1] = (uint8_t)(time[1] - 1);
    if (time[1] == UINT8_MAX) {
        time[1] = BORROW_VALUE;
        time[0] = (uint8_t)(time[0] - 1);
    }
}

int fist_world_begin_tick(fist_world_clock *clock, uint16_t device_timer, fist_world_tick *out) {
    if (clock == NULL || out == NULL) {
        return -1;
    }
    fist_world_clock updated = *clock;
    fist_world_tick result = {.voice_command = FIST_WORLD_NO_COMMAND};
    mission_countdown(updated.mission_countdown);
    updated.tick = (uint16_t)(updated.tick + 1);
    if (updated.pending_voice != UINT8_MAX && updated.voice_at == updated.tick) {
        result.voice_due = true;
        if (updated.voice_mode != MUTED_VOICE) {
            result.voice_command = (uint16_t)(VOICE_COMMAND | updated.pending_voice);
        }
        updated.last_voice_timer = device_timer;
        updated.pending_voice = UINT8_MAX;
    }
    for (size_t index = 0; index < 2; ++index) {
        if (updated.auxiliary_countdown[index] != 0) {
            --updated.auxiliary_countdown[index];
        }
    }
    *clock = updated;
    *out = result;
    return 0;
}

int fist_world_next(const fist_object_pool *pool, fist_world_pass *pass,
                    fist_pool_allocation *out) {
    if (pass == NULL || out == NULL || pass->next_entry > FIST_UNIT_REGISTRY_COUNT ||
        !fist_object_pool_is_valid(pool)) {
        return -1;
    }
    size_t index = pass->next_entry;
    while (index < FIST_UNIT_REGISTRY_COUNT) {
        const fist_pool_entry entry = pool->registry[index];
        ++index;
        if (entry.slot != FIST_POOL_NO_SLOT) {
            *out = (fist_pool_allocation){pool->slots[entry.slot].type, entry.slot,
                                          (uint16_t)(index - 1), entry.value};
            pass->next_entry = (uint16_t)index;
            return FIST_WORLD_VISIT;
        }
    }
    pass->next_entry = FIST_UNIT_REGISTRY_COUNT;
    return FIST_WORLD_END;
}
