#include "sim/primary_fire.h"

#include "sim/object_pool.h"
#include "sim/projectile_launch.h"
#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>

enum {
    AUTOMATIC_FIRE = 128,
    STATUS_COMPONENT = 28,
    REFRESH = 3,
    SUCCESS_VOICE = 12,
    FAILURE_VOICE = 13,
    FAILURE_INTERVAL = 240
};

int fist_m1_fire_untargeted(fist_object_pool *pool, fist_vehicle_state *vehicle,
                            fist_fire_history *history, fist_fire_request request,
                            fist_fire_result *out) {
    if (history == NULL || out == NULL ||
        !fist_m1_launch_source_valid(pool, vehicle, request.launch) ||
        vehicle->weapons.selected != 0) {
        return -1;
    }
    fist_vehicle_state actor = *vehicle;
    fist_object_pool next = *pool;
    fist_fire_history clock = *history;
    fist_fire_result result = {
        .launch = {.outcome = FIST_LAUNCH_EMPTY, .sound_request = FIST_LAUNCH_NO_SOUND},
        .voice_request = FIST_LAUNCH_NO_SOUND};
    result.requested = actor.weapons.trigger != 0 || (actor.secondary_flags & AUTOMATIC_FIRE) != 0;
    if (actor.weapons.trigger != 0) {
        --actor.weapons.trigger;
    }
    if (result.requested) {
        result.weapon_panel_refresh = true;
        actor.components[STATUS_COMPONENT] = REFRESH;
        if (actor.reload_countdown == 0) {
            result.dispatched = true;
            if (fist_m1_launch_untargeted(&next, &actor, request.launch, &result.launch) != 0) {
                return -1;
            }
            if (result.launch.outcome == FIST_LAUNCH_FIRED) {
                result.voice_request = SUCCESS_VOICE;
            } else if ((uint16_t)(request.tick - clock.failed_at) >= FAILURE_INTERVAL) {
                clock.failed_at = request.tick;
                result.voice_request = FAILURE_VOICE;
            }
        }
    }
    *pool = next;
    *vehicle = actor;
    *history = clock;
    *out = result;
    return 0;
}
