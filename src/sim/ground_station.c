#include "sim/mission_world.h"

#include "sim/ground_support.h"
#include "sim/other_damage.h"

#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"
#include "sim/weapon_control.h"

#include <stddef.h>
#include <stdint.h>

enum {
    TARGET = 26,
    ARTILLERY = 27,
    FIRST_AIRCRAFT = 5,
    SECOND_AIRCRAFT = 6,
    PREFERENCES = 3,
    AUTHORED_VARIANTS = 4,
    NOTICE_SUPPRESSED = 2
};

/* Literal original TEXT pointer-table choices, verified by closed WI0104. */
static const uint8_t defaults[FIST_UNIT_GROUND_VEHICLE_COUNT][PREFERENCES] = {
    {2, 0, 6}, {0, 4, 2}, {0, 2, 4}, {4, 0, 2}};
static const uint8_t armored[FIST_UNIT_GROUND_VEHICLE_COUNT][PREFERENCES] = {
    {0, 6, 2}, {2, 4, 0}, {2, 4, 0}, {2, 0, 4}};
static const uint8_t airborne[FIST_UNIT_GROUND_VEHICLE_COUNT][PREFERENCES] = {
    {4, 4, 4}, {6, 6, 6}, {6, 6, 6}, {6, 6, 6}};
static const uint8_t artillery[FIST_UNIT_GROUND_VEHICLE_COUNT][PREFERENCES] = {
    {6, 0, 2}, {2, 4, 0}, {4, 2, 0}, {2, 0, 4}};
static const uint8_t variants[FIST_UNIT_GROUND_VEHICLE_COUNT][2][PREFERENCES] = {
    {{4, 0, 2}, {6, 0, 2}}, {{6, 4, 2}, {2, 4, 6}}, {{6, 2, 0}, {4, 2, 0}}, {{6, 0, 6}, {2, 0, 6}}};

static const uint8_t *preferences(const fist_mission_world *world,
                                  const fist_vehicle_state *actor) {
    const uint16_t kind = actor->type;
    const fist_object_reference target = actor->command.target;
    const uint16_t type = target.lifetime != 0 ? world->pool.slots[target.slot].type : 0;
    if (type == TARGET) {
        const fist_other_actor *object = &world->objects[target.slot].other;
        if (object->allocation.type != TARGET || object->allocation.slot != target.slot ||
            object->mode >= AUTHORED_VARIANTS) {
            return NULL;
        }
        return variants[kind][object->mode & 1];
    }
    if (type == 1 || type == 3) {
        return armored[kind];
    }
    if (type == FIRST_AIRCRAFT || type == SECOND_AIRCRAFT) {
        return airborne[kind];
    }
    return type == ARTILLERY ? artillery[kind] : defaults[kind];
}

int fist_mission_world_select_station(fist_mission_world *world,
                                      fist_ground_station_request request,
                                      fist_ground_station_result *out) {
    if (out == NULL || fist_mission_world_object(world, request.slot) == NULL ||
        world->preparation.prepared != 1 ||
        world->pool.slots[request.slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    fist_vehicle_state next = world->objects[request.slot].vehicle;
    if (next.type != world->pool.slots[request.slot].type ||
        next.component_size != fist_vehicle_component_size(next.type) ||
        !fist_object_reference_is_valid(next.command.target)) {
        return -1;
    }
    if (!fist_object_pool_reference_is_live(&world->pool, next.command.target)) {
        next.command.target = (fist_object_reference){0};
    }
    const uint8_t *choices = preferences(world, &next);
    if (choices == NULL) {
        return -1;
    }
    uint8_t station = choices[PREFERENCES - 1];
    for (size_t index = 0; index < PREFERENCES; ++index) {
        if (next.weapons.rounds[choices[index] / 2] != 0) {
            station = choices[index];
            break;
        }
    }
    fist_ground_station_result result = {0};
    if (fist_weapon_select(&next, station, &result.weapon) != 0) {
        return -1;
    }
    const bool selected = world->combat.selected_slot == request.slot;
    fist_voice_history voice = world->voice;
    if (fist_voice_admit(
            &voice,
            (fist_voice_environment){request.voice_gate, request.tick, next.object_flags, selected},
            result.weapon.voice_request, &result.voice) != 0) {
        return -1;
    }
    fist_timed_advisory advisory = world->advisory;
    if (result.weapon.notice_request != FIST_WEAPON_NO_REQUEST && selected &&
        request.notice_context != NOTICE_SUPPRESSED) {
        advisory = (fist_timed_advisory){(uint16_t)(request.clock + result.weapon.notice_ticks),
                                         result.weapon.notice_request, true};
        result.notice = true;
    }
    world->objects[request.slot].vehicle = next;
    world->voice = voice;
    world->advisory = advisory;
    *out = result;
    return 0;
}
