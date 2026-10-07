#include "app/driving.h"

#include "assets/heightfield.h"
#include "assets/klc.h"
#include "assets/model.h"
#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "assets/vehicle.h"
#include "sim/driver.h"
#include "sim/ground.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/random.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static fist_vehicle_state *player_state(const fist_driving *driving) {
    if (driving == NULL || (driving->world != NULL && driving->preview_player != NULL)) {
        return NULL;
    }
    if (driving->world == NULL) {
        return driving->preview_player;
    }
    fist_mission_world *world = driving->world;
    const uint16_t slot = world->combat.selected_slot;
    if (fist_mission_world_object(world, slot) == NULL ||
        world->pool.slots[slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return NULL;
    }
    return &world->objects[slot].vehicle;
}

const fist_vehicle_state *fist_driving_player(const fist_driving *driving) {
    return player_state(driving);
}

void fist_driving_destroy(fist_driving *driving) {
    if (driving == NULL) {
        return;
    }
    fist_terrain_destroy(&driving->terrain);
    fist_units_destroy(&driving->units);
    fist_model_destroy(&driving->model);
    fist_klc_destroy(&driving->installed_height);
    free(driving->preview_player);
    free(driving->world);
    *driving = (fist_driving){0};
}

static int load_player(fist_driving *driving, const fist_driving_options *options,
                       const fist_unit_definition *definition, const fist_scenario *scenario,
                       int mission) {
    if (mission != 0) {
        driving->world = malloc(sizeof(*driving->world));
        if (driving->world == NULL) {
            return -1;
        }
        const int status =
            fist_mission_world_initialize(&driving->units, &options->random, 0, driving->world);
        if (status != 0) {
            return status;
        }
        if (fist_mission_orders_decode(scenario, &driving->world->orders) != 0) {
            return -1;
        }
        driving->world->orders_loaded = 1;
        driving->world->combat.selected_slot = driving->world->combat.roster[0];
        fist_vehicle_state *actor = player_state(driving);
        return actor == NULL ? -1 : 0;
    }
    driving->preview_player = malloc(sizeof(*driving->preview_player));
    if (driving->preview_player == NULL) {
        return -1;
    }
    fist_random random = options->random;
    return fist_vehicle_initialize(definition, &random, 0, driving->preview_player);
}

static int load(const fist_scenario *scenario, const fist_asset_source *source,
                const fist_driving_options *options, fist_driving *out, int mission) {
    if (scenario == NULL || source == NULL || options == NULL || out == NULL) {
        return -1;
    }
    fist_driving driving = {0};
    driving.feedback.voice_request = FIST_WEAPON_NO_REQUEST;
    driving.feedback.notice = FIST_WEAPON_NO_REQUEST;
    int status = fist_units_decode(scenario, &driving.units);
    const fist_unit_definition *player = fist_units_roster_get(&driving.units, 0);
    if (status == 0) {
        status = load_player(&driving, options, player, scenario, mission);
    }
    fist_vehicle_state *actor = status == 0 ? player_state(&driving) : NULL;
    fist_weapon_status weapon = {0};
    if (status == 0) {
        status = fist_weapon_inspect(actor, &weapon);
    }
    if (status == 0) {
        status = fist_vehicle_visual_decode(player, &driving.visual);
    }
    if (status == 0) {
        status = fist_terrain_load(scenario, source, &driving.terrain);
    }
    if (status == 0) {
        status = fist_heightfield_resample(&driving.terrain.heightmap, options->height_side,
                                           &driving.installed_height);
    }
    if (status == 0 && mission != 0) {
        status = fist_mission_world_prepare(driving.world, &driving.installed_height, 0);
        if (status == 0) {
            status = fist_driver_take_control(actor);
        }
    }
    if (status == 0) {
        status = fist_vehicle_ground_update(actor, &driving.installed_height);
    }
    if (status == 0) {
        status = fist_model_load(driving.visual.model_name, source, &driving.model);
    }
    if (status != 0) {
        fist_driving_destroy(&driving);
        return status;
    }
    *out = driving;
    return 0;
}

int fist_driving_load(const fist_scenario *scenario, const fist_asset_source *source,
                      const fist_driving_options *options, fist_driving *out) {
    return load(scenario, source, options, out, 0);
}

int fist_driving_load_mission(const fist_scenario *scenario, const fist_asset_source *source,
                              const fist_driving_options *options, fist_driving *out) {
    return load(scenario, source, options, out, 1);
}

static int direction(uint16_t keys, uint16_t positive, uint16_t negative) {
    return ((keys & positive) != 0) - ((keys & negative) != 0);
}

typedef struct {
    fist_vehicle_state player;
    fist_driving_feedback feedback;
    uint64_t tick;
} driving_update;

static int record_weapon_events(driving_update *update, const fist_weapon_events *events) {
    fist_driving_feedback *feedback = &update->feedback;
    if ((events->station_changed != 0 && feedback->selections == UINT64_MAX) ||
        (events->timer_expired != 0 && feedback->reloads == UINT64_MAX) ||
        (events->voice_request != FIST_WEAPON_NO_REQUEST &&
         feedback->voice_requests == UINT64_MAX) ||
        (events->notice_request != FIST_WEAPON_NO_REQUEST &&
         update->tick > UINT64_MAX - events->notice_ticks)) {
        return -1;
    }
    feedback->selections += events->station_changed;
    feedback->reloads += events->timer_expired;
    if (events->voice_request != FIST_WEAPON_NO_REQUEST) {
        ++feedback->voice_requests;
        feedback->voice_request = events->voice_request;
    }
    if (events->notice_request != FIST_WEAPON_NO_REQUEST) {
        feedback->notice = events->notice_request;
        feedback->notice_deadline = update->tick + events->notice_ticks;
    }
    if (update->tick >= feedback->notice_deadline) {
        feedback->notice = FIST_WEAPON_NO_REQUEST;
    }
    return 0;
}

static int weapon_edges(driving_update *update, uint16_t edges) {
    static const uint16_t keys[] = {FIST_DRIVE_WEAPON_1, FIST_DRIVE_WEAPON_2, FIST_DRIVE_WEAPON_3,
                                    FIST_DRIVE_WEAPON_4, FIST_DRIVE_WEAPON_5};
    fist_weapon_status status = {0};
    if (fist_weapon_inspect(&update->player, &status) != 0) {
        return -1;
    }
    for (size_t slot = 0; slot < sizeof(keys) / sizeof(keys[0]); ++slot) {
        if ((edges & keys[slot]) != 0 && slot < status.station_count) {
            fist_weapon_events events = {0};
            if (fist_weapon_select(&update->player, (uint8_t)(slot * 2), &events) != 0 ||
                fist_driver_take_control(&update->player) != 0 ||
                record_weapon_events(update, &events) != 0) {
                return -1;
            }
        }
    }
    if ((edges & FIST_DRIVE_NEXT_WEAPON) != 0) {
        fist_weapon_events events = {0};
        if (fist_weapon_cycle(&update->player, &events) != 0 ||
            fist_driver_take_control(&update->player) != 0 ||
            record_weapon_events(update, &events) != 0) {
            return -1;
        }
    }
    return 0;
}

int fist_driving_advance(fist_driving *driving, fist_driving_interval interval) {
    fist_vehicle_state *player = player_state(driving);
    const uint16_t keys = interval.keys;
    const uint64_t microseconds = UINT64_C(1000000);
    const uint64_t tick_phase = FIST_DRIVER_TICK_COUNTS * microseconds;
    if (player == NULL || keys > FIST_DRIVE_KEYS || driving->keys > FIST_DRIVE_KEYS ||
        driving->paused > 1 || driving->clock_phase >= tick_phase ||
        player->type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        player->component_size != fist_vehicle_component_size(player->type) ||
        (driving->world != NULL && driving->world->pending_player_impact != FIST_POOL_NO_SLOT)) {
        return -1;
    }
    const fist_driver_controls controls = {
        .throttle_change = (int8_t)direction(driving->keys, FIST_DRIVE_FASTER, FIST_DRIVE_SLOWER),
        .steering = (int8_t)direction(driving->keys, FIST_DRIVE_RIGHT, FIST_DRIVE_LEFT),
        .turret_change =
            (int16_t)(direction(driving->keys, FIST_DRIVE_TURRET_RIGHT, FIST_DRIVE_TURRET_LEFT) *
                      FIST_DRIVER_TURRET_INCREMENT),
        .throttle_off = (uint8_t)((driving->keys & FIST_DRIVE_THROTTLE_OFF) != 0)};
    uint64_t phase = driving->clock_phase;
    uint64_t count = 0;
    if (driving->paused == 0) {
        phase += (uint64_t)interval.elapsed_us * FIST_DRIVER_PIT_RATE;
        count = phase / tick_phase;
        phase %= tick_phase;
    }
    if (driving->ticks > UINT64_MAX - count) {
        return -1;
    }
    driving_update update = {*player, driving->feedback, driving->ticks};
    for (uint64_t tick = 0; tick < count; ++tick) {
        fist_weapon_events events = {0};
        if (fist_driver_step(&update.player, &driving->installed_height, &controls, &events) != 0) {
            return -1;
        }
        ++update.tick;
        if (record_weapon_events(&update, &events) != 0) {
            return -1;
        }
    }
    uint8_t paused = driving->paused;
    if ((keys & FIST_DRIVE_PAUSE) != 0 && (driving->keys & FIST_DRIVE_PAUSE) == 0) {
        paused ^= 1;
    }
    if (paused == 0 && weapon_edges(&update, keys & (uint16_t)~driving->keys) != 0) {
        return -1;
    }
    *player = update.player;
    driving->feedback = update.feedback;
    driving->clock_phase = phase;
    driving->ticks = update.tick;
    driving->paused = paused;
    driving->keys = keys;
    return 0;
}

uint16_t fist_driving_key(int key) {
    if (key >= 'a' && key <= 'z') {
        key += 'A' - 'a';
    }
    switch (key) {
    case 'W':
        return FIST_DRIVE_FASTER;
    case 'S':
        return FIST_DRIVE_SLOWER;
    case 'A':
        return FIST_DRIVE_LEFT;
    case 'D':
        return FIST_DRIVE_RIGHT;
    case 'Q':
        return FIST_DRIVE_TURRET_LEFT;
    case 'E':
        return FIST_DRIVE_TURRET_RIGHT;
    case ' ':
        return FIST_DRIVE_THROTTLE_OFF;
    case 'P':
        return FIST_DRIVE_PAUSE;
    case '1':
        return FIST_DRIVE_WEAPON_1;
    case '2':
        return FIST_DRIVE_WEAPON_2;
    case '3':
        return FIST_DRIVE_WEAPON_3;
    case '4':
        return FIST_DRIVE_WEAPON_4;
    case '5':
        return FIST_DRIVE_WEAPON_5;
    case '\t':
        return FIST_DRIVE_NEXT_WEAPON;
    default:
        return 0;
    }
}
