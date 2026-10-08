#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "mission_probe_io.h"
#include "probe_io.h"
#include "sim/ground_support.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/world.h"
#include "vehicle_probe_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_BYTES = 80,
    CASE_BYTES = HEADER_BYTES + FIST_UNIT_EXTENDED_SIZE,
    ARTILLERY = 27,
    TARGET = 26,
    ACTOR_SLOT = FIST_POOL_SHORT_SLOTS,
    STATION = 0,
    SUPPORT = 1,
    CONFIGURE = 2,
    TARGET_RELEASE = 1,
    TARGET_REUSE = 2,
    GUN_RELEASE = 4,
    GUN_REUSE = 8,
    GUN_ORPHAN = 16,
    FILLER_TYPE = 18,
    REUSE_EXTENDED_FILLER = 5,
    SEED_AIR_TICK = 17,
    SEED_ARTILLERY_CLOCK = 31,
    SEED_TARGET_X = 33,
    SEED_TARGET_Y = 35,
    REQUEST_INTERVAL = 1800,
    VOICE_INTERVAL = 30,
    OUTPUT_SENTINEL = 0x5a,
    INVALID_RANDOM = 6,
    INVALID_COUNT = 8,
    INVALID_GUN_REFERENCE = 9,
    INVALID_GUN_PAYLOAD = 10,
    INVALID_STOCK = 11,
    INVALID_GUN_TYPE = 12
};

enum {
    H_OPERATION = 0,
    H_SELECTED = 1,
    H_SOURCE = 2,
    H_COARSE = 3,
    H_TARGET_TYPE = 4,
    H_VARIANT = 6,
    H_CONTEXT = 7,
    H_CLOCK = 8,
    H_TICK = 10,
    H_VOICE_GATE = 12,
    H_VOICE_PRIOR = 14,
    H_REQUEST_PRIOR = 16,
    H_AIR_CLOCK = 18,
    H_ARTILLERY_CLOCK = 20,
    H_AIR_STOCK = 22,
    H_AIR_DELAY = 24,
    H_ARTILLERY_DELAY = 26,
    H_ARTILLERY_COUNT = 28,
    H_AIR_USED = 29,
    H_ARTILLERY_USED = 30,
    H_FULL_POOL = 31,
    H_AMMUNITION = 32,
    H_SEEDS = 40,
    H_CURSOR = 48,
    H_RETAINED = 49,
    H_POST_REQUESTER = 50,
    H_INVALID = 51,
    H_TARGET_X = 52,
    H_TARGET_Y = 56,
    H_DISPLAY_TICKS = 60,
    H_DISPLAY_KIND = 62,
    H_ADVISORY_CODE = 63,
    H_ADVISORY_UNTIL = 64,
    H_MESSAGE_TICKS = 66,
    H_SOUND_SELECTOR = 68,
    H_CONFIGURED = 70,
    H_STEPS = 71,
    H_REVERSE = 72,
    H_HEIGHT = 73,
    H_GUN_INDEX = 74,
    H_ACTOR_SLOT = 76,
    H_TARGET_SLOT = 78,
    BATCH_HEADER = 8,
    MIN_DETAIL = 512,
    MAX_DETAIL = 4096,
    PREPARED_ROUNDS = 5,
    POSE_Y_OFFSET = 8,
    ALTITUDE_OFFSET = 12,
    HIGH_SEED = 32768,
};

typedef struct {
    const uint8_t *header;
    const uint8_t *saved;
} probe_input;

typedef struct {
    fist_mission_world world;
    fist_mission_world before;
    fist_mission_world guard;
    fist_pool_allocation target;
    fist_pool_allocation guns[FIST_MISSION_ARTILLERY_SIDE_SLOTS];
    uint8_t pixel;
    uint16_t actor_slot;
    const fist_mission_world *prepared;
    fist_klc_image height;
} probe_case;

static void fill_bytes(size_t size, void *object, uint8_t value) {
    uint8_t *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        bytes[index] = value;
    }
}

static int install_vehicle(fist_mission_world *world, const uint8_t *raw,
                           fist_pool_allocation allocation) {
    const fist_unit_definition definition = {.type = fist_read_u16le(raw),
                                             .registry_index = allocation.registry_index,
                                             .generation = allocation.value,
                                             .map_x = fist_read_i32le(raw + 4),
                                             .map_y = fist_read_i32le(raw + 8),
                                             .altitude = fist_read_i32le(raw + 12),
                                             .snapshot = {raw, FIST_UNIT_EXTENDED_SIZE}};
    return fist_vehicle_restore(&definition, &world->objects[allocation.slot].vehicle);
}

static int import_object(probe_case *value, uint16_t type, uint16_t index, uint16_t generation,
                         fist_pool_allocation *out) {
    return fist_object_pool_import(&value->world.pool, (fist_pool_import){type, index, generation},
                                   out);
}

static int install_target(probe_case *value, probe_input input) {
    const uint8_t *header = input.header;
    const uint8_t *raw = input.saved;
    const uint16_t type = fist_read_u16le(header + H_TARGET_TYPE);
    if (type == FIST_POOL_NO_SLOT) {
        return 0;
    }
    if (import_object(value, type, 1, 2, &value->target) != 0) {
        return -1;
    }
    fist_mission_object *object = &value->world.objects[value->target.slot];
    const fist_object_pose pose = {fist_read_i32le(header + H_TARGET_X),
                                   fist_read_i32le(header + H_TARGET_Y), 0, 0};
    if (type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        uint8_t copy[FIST_UNIT_EXTENDED_SIZE];
        fist_probe_capture(raw, sizeof(copy), copy);
        copy[0] = (uint8_t)type;
        copy[1] = 0;
        if (install_vehicle(&value->world, copy, value->target) != 0) {
            return -1;
        }
        object->vehicle.map_x = pose.x;
        object->vehicle.map_y = pose.y;
    } else {
        /* Declared typed target pose, not invented class initialization. */
        object->other = (fist_other_actor){
            .allocation = value->target, .pose = pose, .mode = header[H_VARIANT]};
    }
    return fist_object_pool_reference(
        &value->world.pool, value->target.slot,
        &value->world.objects[value->actor_slot].vehicle.command.target);
}

static int fill_earlier_slots(probe_case *value, fist_pool_allocation allocation) {
    if (value->prepared == NULL) {
        return 0;
    }
    const bool extended = allocation.slot >= FIST_POOL_SHORT_SLOTS;
    const uint16_t start = extended ? FIST_POOL_SHORT_SLOTS : 0;
    const uint16_t type = extended ? REUSE_EXTENDED_FILLER : FILLER_TYPE;
    fist_probe_capture(&value->world, sizeof(value->guard), &value->guard);
    for (uint16_t slot = start; slot < allocation.slot; ++slot) {
        if (value->world.pool.slots[slot].used != 0) {
            continue;
        }
        fist_pool_allocation filler = {0};
        if (fist_object_pool_allocate(&value->world.pool, (fist_pool_request){type, 0}, &filler) !=
                FIST_POOL_OK ||
            filler.slot != slot) {
            return -1;
        }
        /* Declared normal constructor fixtures occupy earlier free cells.
         * No class update, rendering or battle reachability is claimed. */
        value->world.objects[slot] = extended
                                         ? (fist_mission_object){.other = {.allocation = filler}}
                                         : (fist_mission_object){.muzzle = {.allocation = filler}};
        fist_probe_capture(&value->world.objects[slot], sizeof(value->guard.objects[slot]),
                           &value->guard.objects[slot]);
    }
    fist_probe_capture(&value->world.pool, sizeof(value->guard.pool), &value->guard.pool);
    return fist_probe_unchanged(&value->world, sizeof(value->world), &value->guard) ? 0 : -1;
}

static int retire_and_reuse(probe_case *value, fist_pool_allocation allocation, bool reuse) {
    if (reuse && fill_earlier_slots(value, allocation) != 0) {
        return -1;
    }
    fist_pool_allocation released = {0};
    if (fist_object_pool_release(&value->world.pool, allocation.registry_index, &released) != 0 ||
        released.slot != allocation.slot) {
        return -1;
    }
    if (reuse) {
        fist_pool_allocation successor = {0};
        if (import_object(value, allocation.type, allocation.registry_index, allocation.value,
                          &successor) != 0 ||
            successor.slot != allocation.slot || successor.value != allocation.value) {
            return -1;
        }
    }
    return 0;
}

static int prepare_resources(probe_case *value, probe_input input) {
    const uint8_t *header = input.header;
    const uint8_t *raw = input.saved;
    value->actor_slot = ACTOR_SLOT;
    fist_mission_world_reset(&value->world);
    value->world.preparation.prepared = 1;
    value->world.preparation.artillery_count[1] = header[H_ARTILLERY_COUNT];
    value->target = (fist_pool_allocation){.slot = FIST_POOL_NO_SLOT};
    fist_pool_allocation actor = {0};
    if (import_object(value, fist_read_u16le(raw), 0, 1, &actor) != 0 ||
        actor.slot != value->actor_slot || install_vehicle(&value->world, raw, actor) != 0 ||
        install_target(value, input) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_MISSION_ARTILLERY_SIDE_SLOTS; ++index) {
        fist_pool_allocation *gun = &value->guns[index];
        if (import_object(value, ARTILLERY, (uint16_t)(index + 2), (uint16_t)(index + 3), gun) !=
            0) {
            return -1;
        }
        value->world.objects[gun->slot].other =
            (fist_other_actor){.allocation = *gun, .mode = (uint8_t)(index & 1), .flags = 1};
        value->world.objects[gun->slot].other.state.type27.rounds =
            fist_read_u16le(header + H_AMMUNITION + (index * 2));
        fist_artillery_resource *resource = &value->world.preparation.artillery[1][index];
        resource->allocation = *gun;
        if (fist_object_pool_reference(&value->world.pool, gun->slot, &resource->reference) != 0) {
            return -1;
        }
    }
    return 0;
}

static int prepare_retention(probe_case *value, const uint8_t *header) {
    const uint8_t retained = header[H_RETAINED];
    if ((retained & TARGET_RELEASE) != 0 && value->target.slot != FIST_POOL_NO_SLOT &&
        retire_and_reuse(value, value->target, (retained & TARGET_REUSE) != 0) != 0) {
        return -1;
    }
    const size_t gun_index = header[H_GUN_INDEX];
    if (gun_index >= FIST_MISSION_ARTILLERY_SIDE_SLOTS) {
        return -1;
    }
    if ((retained & GUN_RELEASE) != 0 &&
        retire_and_reuse(value, value->guns[gun_index], (retained & GUN_REUSE) != 0) != 0) {
        return -1;
    }
    if ((retained & GUN_ORPHAN) != 0) {
        fist_pool_allocation replacement = {0};
        if (import_object(value, ARTILLERY, value->guns[gun_index].registry_index,
                          value->guns[gun_index].value, &replacement) != 0) {
            return -1;
        }
        value->world.objects[replacement.slot].other =
            (fist_other_actor){.allocation = replacement};
    }
    if (header[H_FULL_POOL] != 0) {
        while (value->world.pool.short_count < FIST_POOL_SHORT_SLOTS) {
            fist_pool_allocation filler = {0};
            if (fist_object_pool_allocate(&value->world.pool, (fist_pool_request){FILLER_TYPE, 0},
                                          &filler) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int seed_support(probe_case *value, const uint8_t *header) {
    value->world.combat.selected_slot =
        header[H_SELECTED] != 0 ? value->actor_slot : FIST_POOL_NO_SLOT;
    const fist_support_configuration configuration = {
        .air_stock = {23, fist_read_u16le(header + H_AIR_STOCK)},
        .air_delay = {17, fist_read_u16le(header + H_AIR_DELAY)},
        .artillery_delay = {19, fist_read_u16le(header + H_ARTILLERY_DELAY)},
        .reverse_aircraft = header[H_REVERSE] != 0};
    if (fist_mission_world_configure_support(&value->world, &configuration,
                                             fist_read_u16le(header + H_CLOCK)) != 0) {
        return -1;
    }
    value->world.support.configured = header[H_CONFIGURED] != 0;
    value->world.support.last_request = fist_read_u16le(header + H_REQUEST_PRIOR);
    value->world.support.air_clock[1] = fist_read_u16le(header + H_AIR_CLOCK);
    value->world.support.artillery_clock[1] = fist_read_u16le(header + H_ARTILLERY_CLOCK);
    value->world.voice.admitted_at = fist_read_u16le(header + H_VOICE_PRIOR);
    value->world.target_notice = (fist_target_notice){
        .duration = fist_read_u16le(header + H_DISPLAY_TICKS), .kind = header[H_DISPLAY_KIND]};
    value->world.advisory = (fist_timed_advisory){fist_read_u16le(header + H_ADVISORY_UNTIL),
                                                  header[H_ADVISORY_CODE], true};
    value->world.artillery_message_ticks = fist_read_u16le(header + H_MESSAGE_TICKS);
    value->world.sound_selector = fist_read_u16le(header + H_SOUND_SELECTOR);
    value->pixel = header[H_HEIGHT];
    if (value->prepared == NULL) {
        value->height = (fist_klc_image){.width = 1, .height = 1, .pixels = &value->pixel};
    }
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        value->world.random.words[index] = fist_read_u16le(header + H_SEEDS + (index * 2));
    }
    value->world.random.next_stream = header[H_CURSOR];
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        for (size_t index = 0; index < FIST_SUPPORT_QUEUE_SLOTS; ++index) {
            fist_air_support_entry *air = &value->world.support.air[side][index];
            if (side == 0 || index < header[H_AIR_USED]) {
                if (fist_object_pool_reference(&value->world.pool, value->actor_slot,
                                               &air->requester) != 0) {
                    return -1;
                }
            }
            air->tick = (uint16_t)(SEED_AIR_TICK + index + side);
            value->world.support.artillery[side][index] = (fist_artillery_support_entry){
                .phase = (uint16_t)((side == 0 || index < header[H_ARTILLERY_USED]) ? 1 : 0),
                .clock = (uint16_t)(SEED_ARTILLERY_CLOCK + index + side),
                .target = {(int32_t)(SEED_TARGET_X + index + side),
                           (int32_t)(SEED_TARGET_Y + index + side)}};
        }
    }
    return 0;
}

static void inject_invalid(probe_case *value, const uint8_t *header) {
    switch (header[H_INVALID]) {
    case 1:
        value->world.preparation.prepared = 0;
        break;
    case 2:
        value->world.objects[value->actor_slot].vehicle.component_size = 0;
        break;
    case 3:
        value->world.objects[value->actor_slot].vehicle.command.target =
            (fist_object_reference){0, 1};
        break;
    case INVALID_RANDOM:
        value->world.random.next_stream = FIST_RANDOM_STREAMS;
        break;
    case INVALID_COUNT:
        value->world.preparation.artillery_count[1] = FIST_MISSION_ARTILLERY_SIDE_SLOTS + 1;
        break;
    case INVALID_GUN_REFERENCE:
        value->world.preparation.artillery[1][0].reference = (fist_object_reference){0};
        break;
    case INVALID_GUN_PAYLOAD:
        value->world.objects[value->guns[0].slot].other.allocation.type = TARGET;
        break;
    case INVALID_GUN_TYPE: {
        fist_pool_allocation retyped = {0};
        if (fist_object_pool_retype(&value->world.pool, value->guns[0], TARGET, &retyped) != 0) {
            value->world.preparation.prepared = 0;
        }
        break;
    }
    case INVALID_STOCK:
        value->world.objects[value->actor_slot].vehicle.weapons.class_parameter = UINT16_MAX;
        break;
    default:
        break;
    }
}

static int prepared_resources(const fist_mission_world *world) {
    if (world->preparation.prepared != 1) {
        return -1;
    }
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        const size_t count = world->preparation.artillery_count[side];
        if (count > FIST_MISSION_ARTILLERY_SIDE_SLOTS) {
            return -1;
        }
        for (size_t index = 0; index < count; ++index) {
            const fist_artillery_resource resource = world->preparation.artillery[side][index];
            const uint16_t slot = resource.allocation.slot;
            if (slot >= FIST_UNIT_REGISTRY_COUNT || resource.allocation.type != ARTILLERY ||
                world->pool.slots[slot].type != ARTILLERY || resource.reference.slot != slot ||
                !fist_object_pool_reference_is_live(&world->pool, resource.reference)) {
                return -1;
            }
            const fist_other_actor *gun = &world->objects[slot].other;
            if (!fist_probe_unchanged(&gun->allocation, sizeof(gun->allocation),
                                      &resource.allocation) ||
                gun->state.type27.rounds != PREPARED_ROUNDS) {
                return -1;
            }
        }
    }
    return 0;
}

static int canonical_target(probe_case *value, const uint8_t *header) {
    const uint16_t slot = fist_read_u16le(header + H_TARGET_SLOT);
    value->target = (fist_pool_allocation){.slot = FIST_POOL_NO_SLOT};
    if (slot == FIST_POOL_NO_SLOT) {
        return fist_read_u16le(header + H_TARGET_TYPE) == FIST_POOL_NO_SLOT ? 0 : -1;
    }
    fist_object_pose ground = {0};
    fist_mission_view view = {0};
    if (fist_mission_world_view(&value->world, slot, &ground, &view) != 0 ||
        value->world.pool.slots[slot].type != fist_read_u16le(header + H_TARGET_TYPE) ||
        view.pose->x != fist_read_i32le(header + H_TARGET_X) ||
        view.pose->y != fist_read_i32le(header + H_TARGET_Y) ||
        (value->world.pool.slots[slot].type == TARGET &&
         value->world.objects[slot].other.mode != header[H_VARIANT])) {
        return -1;
    }
    if ((header[H_RETAINED] & TARGET_RELEASE) != 0 &&
        fist_object_pool_find(&value->world.pool, slot, &value->target) != FIST_POOL_OK) {
        return -1;
    }
    return fist_object_pool_reference(
        &value->world.pool, slot, &value->world.objects[value->actor_slot].vehicle.command.target);
}

static int prepare_canonical(probe_case *value, probe_input input) {
    const uint8_t *header = input.header;
    value->actor_slot = fist_read_u16le(header + H_ACTOR_SLOT);
    fist_probe_capture(value->prepared, sizeof(value->world), &value->world);
    if (value->actor_slot >= FIST_UNIT_REGISTRY_COUNT ||
        value->world.pool.slots[value->actor_slot].used == 0 ||
        value->world.pool.slots[value->actor_slot].type >= FIST_UNIT_GROUND_VEHICLE_COUNT ||
        fist_read_u16le(input.saved) != value->world.pool.slots[value->actor_slot].type ||
        header[H_ARTILLERY_COUNT] != value->world.preparation.artillery_count[1]) {
        return -1;
    }
    const fist_vehicle_state *actor = &value->world.objects[value->actor_slot].vehicle;
    const fist_pool_allocation identity = {actor->type, value->actor_slot, actor->registry_index,
                                           actor->generation};
    if (actor->map_x != fist_read_i32le(input.saved + 4) ||
        actor->map_y != fist_read_i32le(input.saved + POSE_Y_OFFSET) ||
        actor->altitude != fist_read_i32le(input.saved + ALTITUDE_OFFSET) ||
        install_vehicle(&value->world, input.saved, identity) != 0 ||
        canonical_target(value, header) != 0) {
        return -1;
    }
    for (size_t index = 0; index < FIST_MISSION_ARTILLERY_SIDE_SLOTS; ++index) {
        value->guns[index] = value->world.preparation.artillery[1][index].allocation;
    }
    return 0;
}

static int prepare(probe_case *value, probe_input input) {
    const int prepared =
        value->prepared != NULL ? prepare_canonical(value, input) : prepare_resources(value, input);
    if (prepared != 0 || prepare_retention(value, input.header) != 0 ||
        seed_support(value, input.header) != 0) {
        return -1;
    }
    inject_invalid(value, input.header);
    return 0;
}

static int marker_pool_contract(const probe_case *value, const fist_ground_support_result *result) {
    const fist_pool_allocation marker = result->marker;
    const fist_object_pool *pool = &value->world.pool;
    const fist_object_pool *before = &value->before.pool;
    if (marker.slot >= FIST_POOL_SHORT_SLOTS || marker.registry_index >= FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    const fist_pool_entry old = before->registry[marker.registry_index];
    return marker.type == FIST_SUPPORT_SMOKE_TYPE && marker.value == 1 &&
                   before->slots[marker.slot].used == 0 && old.slot == FIST_POOL_NO_SLOT &&
                   old.value == 0 && pool->slots[marker.slot].type == FIST_SUPPORT_SMOKE_TYPE &&
                   pool->slots[marker.slot].used == 1 &&
                   pool->registry[marker.registry_index].slot == marker.slot &&
                   pool->registry[marker.registry_index].value == 1 &&
                   pool->short_count == before->short_count + 1 &&
                   pool->lifetimes[marker.slot] != 0 &&
                   pool->lifetimes[marker.slot] != before->lifetimes[marker.slot]
               ? 0
               : -1;
}

static int unchanged_owners(probe_case *value, const fist_ground_support_result *result,
                            bool support) {
    fist_mission_world *guard = &value->guard;
    const fist_mission_world *before = &value->before;
    fist_probe_capture(&value->world, sizeof(*guard), guard);
    fist_probe_capture(&before->objects[value->actor_slot].vehicle, sizeof(fist_vehicle_state),
                       &guard->objects[value->actor_slot].vehicle);
    fist_probe_capture(&before->advisory, sizeof(guard->advisory), &guard->advisory);
    if (!support) {
        fist_probe_capture(&before->voice, sizeof(guard->voice), &guard->voice);
    } else {
        fist_probe_capture(&before->support, sizeof(guard->support), &guard->support);
        fist_probe_capture(&before->random, sizeof(guard->random), &guard->random);
        fist_probe_capture(&before->target_notice, sizeof(guard->target_notice),
                           &guard->target_notice);
        guard->sound_selector = before->sound_selector;
        guard->artillery_message_ticks = before->artillery_message_ticks;
        if (result->resource_slot != FIST_POOL_NO_SLOT) {
            guard->objects[result->resource_slot].other.state.type27.rounds =
                before->objects[result->resource_slot].other.state.type27.rounds;
        }
        if (result->smoke == FIST_SUPPORT_SMOKE_CREATED) {
            if (marker_pool_contract(value, result) != 0) {
                return -1;
            }
            const size_t slot = result->marker.slot;
            const size_t registry = result->marker.registry_index;
            fist_probe_capture(&before->objects[slot], sizeof(guard->objects[slot]),
                               &guard->objects[slot]);
            fist_probe_capture(&before->pool.slots[slot], sizeof(guard->pool.slots[slot]),
                               &guard->pool.slots[slot]);
            guard->pool.lifetimes[slot] = before->pool.lifetimes[slot];
            fist_probe_capture(&before->pool.registry[registry],
                               sizeof(guard->pool.registry[registry]),
                               &guard->pool.registry[registry]);
            guard->pool.short_count = before->pool.short_count;
        }
    }
    return fist_probe_unchanged(guard, sizeof(*guard), before) ? 0 : -1;
}

static uint16_t observed_rounds(const fist_mission_world *world, uint16_t slot) {
    if (slot >= FIST_UNIT_REGISTRY_COUNT ||
        (world->pool.slots[slot].used != 0 &&
         world->pool.slots[slot].type == FIST_SUPPORT_SMOKE_TYPE)) {
        return 0;
    }
    /* Invalid-metadata fixtures can retype a slot while deliberately retaining
     * its other/type27 payload. Observe that preserved counter on failure;
     * an actual marker constructor replaces the union and has no gun rounds. */
    return world->objects[slot].other.state.type27.rounds;
}

static void observe(const probe_case *value, int status, const fist_ground_station_result *station,
                    const fist_ground_support_result *support) {
    const fist_mission_world *world = &value->world;
    const fist_vehicle_state *actor = &world->objects[value->actor_slot].vehicle;
    printf("status %d\n", status);
    fist_probe_write_vehicle_state(actor);
    printf("target %u %d %u %u\n", (unsigned)actor->command.target.slot,
           fist_object_pool_reference_is_live(&world->pool, actor->command.target),
           (unsigned)actor->command.target_range, (unsigned)actor->command.target_heading);
    printf("station %u %u %u %u %u %u %u %u %u %u\n", (unsigned)station->weapon.station_changed,
           (unsigned)station->weapon.timer_expired, (unsigned)station->weapon.voice_request,
           (unsigned)station->weapon.notice_request, (unsigned)station->weapon.notice_ticks,
           (unsigned)station->voice.ax, (unsigned)station->voice.dx, station->voice.ecx,
           (unsigned)station->voice.emitted, (unsigned)station->notice);
    printf("result %u %u %u %u %u %u %u %u %u %u %u %u %u\n", (unsigned)support->support,
           (unsigned)support->smoke, (unsigned)support->queue_index,
           (unsigned)support->resource_slot, (unsigned)support->marker.slot,
           (unsigned)support->marker.registry_index, (unsigned)support->marker.value,
           (unsigned)support->sound.ax, (unsigned)support->sound.dx, support->sound.ecx,
           (unsigned)support->sound.emitted, (unsigned)support->notice, (unsigned)support->message);
    printf("history %u %u %u %u %u %u %u %u %u\n", (unsigned)world->voice.admitted_at,
           (unsigned)world->target_notice.duration, (unsigned)world->target_notice.kind,
           (unsigned)world->advisory.code, (unsigned)world->advisory.until,
           (unsigned)world->advisory.active, (unsigned)world->artillery_message_ticks,
           (unsigned)world->sound_selector, (unsigned)world->support.last_request);
    printf("random %u %u %u %u %u\n", (unsigned)world->random.words[0],
           (unsigned)world->random.words[1], (unsigned)world->random.words[2],
           (unsigned)world->random.words[3], (unsigned)world->random.next_stream);
    printf("clocks %u %u %u %u\n", (unsigned)world->support.air_clock[0],
           (unsigned)world->support.air_clock[1], (unsigned)world->support.artillery_clock[0],
           (unsigned)world->support.artillery_clock[1]);
    printf("configuration %u %u %u %u %u %u %u %u %u %u\n",
           (unsigned)world->support.configuration.air_stock[0],
           (unsigned)world->support.configuration.air_stock[1],
           (unsigned)world->support.configuration.air_delay[0],
           (unsigned)world->support.configuration.air_delay[1],
           (unsigned)world->support.configuration.artillery_delay[0],
           (unsigned)world->support.configuration.artillery_delay[1],
           (unsigned)world->support.configuration.reverse_aircraft,
           (unsigned)world->support.air_type[0], (unsigned)world->support.air_type[1],
           (unsigned)world->support.configured);
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        printf("air%zu", side);
        for (size_t index = 0; index < FIST_SUPPORT_QUEUE_SLOTS; ++index) {
            const fist_air_support_entry *entry = &world->support.air[side][index];
            printf(" %u:%u:%u", (unsigned)(entry->requester.lifetime != 0),
                   (unsigned)entry->requester.slot, (unsigned)entry->tick);
        }
        puts("");
        printf("artillery%zu", side);
        for (size_t index = 0; index < FIST_SUPPORT_QUEUE_SLOTS; ++index) {
            const fist_artillery_support_entry *entry = &world->support.artillery[side][index];
            printf(" %u:%u:%d:%d", (unsigned)entry->phase, (unsigned)entry->clock, entry->target.x,
                   entry->target.y);
        }
        puts("");
    }
    printf("guns");
    for (size_t index = 0; index < FIST_MISSION_ARTILLERY_SIDE_SLOTS; ++index) {
        const fist_artillery_resource *resource = &world->preparation.artillery[1][index];
        printf(" %u:%u:%d", (unsigned)value->guns[index].slot,
               (unsigned)observed_rounds(world, value->guns[index].slot),
               fist_object_pool_reference_is_live(&world->pool, resource->reference));
    }
    puts("");
    printf("pool %u %u\n", (unsigned)world->pool.short_count, (unsigned)world->pool.extended_count);
    if (status == 0 && support->smoke == FIST_SUPPORT_SMOKE_CREATED) {
        const fist_support_marker *marker = &world->objects[support->marker.slot].support_marker;
        printf("marker %u %u %u %u %d %d %d %u %u %u %u %u %u %u\n",
               (unsigned)marker->allocation.type, (unsigned)marker->allocation.slot,
               (unsigned)marker->allocation.registry_index, (unsigned)marker->allocation.value,
               marker->pose.x, marker->pose.y, marker->pose.altitude,
               (unsigned)marker->pose.heading, (unsigned)marker->projection_extent,
               (unsigned)marker->projection_scale, (unsigned)marker->flags,
               (unsigned)marker->secondary_flags, (unsigned)marker->ground_height,
               (unsigned)marker->variant);
    }
    puts("end");
}

static int execute_step(probe_case *value, const uint8_t *header, uint16_t clock, uint16_t tick,
                        fist_ground_station_result *station, fist_ground_support_result *support) {
    int status = 0;
    if (header[H_OPERATION] == CONFIGURE) {
        status = fist_mission_world_configure_support(&value->world,
                                                      &value->world.support.configuration, clock);
        *station = (fist_ground_station_result){0};
        *support = (fist_ground_support_result){0};
        fist_probe_capture(&value->world, sizeof(value->guard), &value->guard);
        fist_probe_capture(&value->before.support, sizeof(value->guard.support),
                           &value->guard.support);
        if (!fist_probe_unchanged(&value->guard, sizeof(value->guard), &value->before)) {
            return -1;
        }
    } else if (header[H_OPERATION] == STATION) {
        const fist_ground_station_request request = {value->actor_slot, clock, tick,
                                                     fist_read_u16le(header + H_VOICE_GATE),
                                                     header[H_CONTEXT]};
        status = fist_mission_world_select_station(&value->world, request, station);
        *support = (fist_ground_support_result){0};
    } else if (header[H_OPERATION] == SUPPORT) {
        const fist_ground_support_request request = {value->actor_slot,
                                                     header[H_SOURCE] != 0 ? value->actor_slot
                                                                           : FIST_POOL_NO_SLOT,
                                                     clock,
                                                     tick,
                                                     header[H_CONTEXT],
                                                     header[H_COARSE] != 0};
        status = fist_mission_world_request_support(
            &value->world, header[H_INVALID] == 4 ? NULL : &value->height, request, support);
        *station = (fist_ground_station_result){0};
    } else {
        return -1;
    }
    return status;
}

static int requester_retention(probe_case *value, const fist_ground_support_result *support,
                               bool reuse) {
    if (support->support != FIST_SUPPORT_AIR_CONFIRMED ||
        support->queue_index >= FIST_SUPPORT_QUEUE_SLOTS) {
        return -1;
    }
    const fist_object_reference queued =
        value->world.support.air[1][support->queue_index].requester;
    fist_object_reference old = {0};
    fist_pool_allocation actor = {0};
    if (fist_object_pool_reference(&value->world.pool, value->actor_slot, &old) != 0 ||
        queued.slot != old.slot || queued.lifetime != old.lifetime ||
        fist_object_pool_find(&value->world.pool, value->actor_slot, &actor) != FIST_POOL_OK) {
        return -1;
    }
    if (reuse && fill_earlier_slots(value, actor) != 0) {
        return -1;
    }
    fist_probe_capture(&value->world, sizeof(value->guard), &value->guard);
    if (retire_and_reuse(value, actor, reuse) != 0 ||
        fist_object_pool_reference_is_live(&value->world.pool, queued)) {
        return -1;
    }
    fist_object_reference successor = {0};
    const int captured =
        fist_object_pool_reference(&value->world.pool, value->actor_slot, &successor);
    if ((reuse && (captured != 0 || successor.slot != queued.slot ||
                   successor.lifetime == queued.lifetime)) ||
        (!reuse && captured != FIST_POOL_UNAVAILABLE)) {
        return -1;
    }
    /* Releasing the requester changes pool metadata only. In particular, the
     * actual admitted queue keeps its expired reference and cannot acquire
     * the successor, even when the saved allocation tuple repeats. */
    fist_probe_capture(&value->world.pool, sizeof(value->guard.pool), &value->guard.pool);
    if (!fist_probe_unchanged(&value->world, sizeof(value->world), &value->guard)) {
        return -1;
    }
    printf("requester_lifetime %u %u 0 %u\n", (unsigned)value->actor_slot, (unsigned)reuse,
           (unsigned)(captured == FIST_POOL_OK));
    return 0;
}

static int run_case(probe_case *value, const uint8_t *header) {
    const uint8_t *raw = header + HEADER_BYTES;
    if (header[H_OPERATION] > CONFIGURE || header[H_POST_REQUESTER] > 2 ||
        (header[H_POST_REQUESTER] != 0 &&
         (header[H_OPERATION] != SUPPORT || header[H_STEPS] != 1))) {
        return -1;
    }
    uint8_t copy[FIST_UNIT_EXTENDED_SIZE];
    fist_probe_capture(raw, sizeof(copy), copy);
    if (prepare(value, (probe_input){header, copy}) != 0) {
        return -1;
    }
    /* No borrowed saved input may survive installation. */
    fill_bytes(sizeof(copy), copy, 0);
    const unsigned steps = header[H_STEPS] != 0 ? header[H_STEPS] : 1U;
    for (unsigned step = 0; step < steps; ++step) {
        fist_probe_capture(&value->world, sizeof(value->before), &value->before);
        const uint16_t clock =
            (uint16_t)(fist_read_u16le(header + H_CLOCK) + (step * REQUEST_INTERVAL));
        const uint16_t tick =
            (uint16_t)(fist_read_u16le(header + H_TICK) + (step * VOICE_INTERVAL));
        fist_ground_station_result station;
        fist_ground_support_result support;
        fill_bytes(sizeof(station), &station, OUTPUT_SENTINEL);
        fill_bytes(sizeof(support), &support, OUTPUT_SENTINEL);
        uint8_t station_before[sizeof(station)];
        uint8_t support_before[sizeof(support)];
        fist_probe_capture(&station, sizeof(station), station_before);
        fist_probe_capture(&support, sizeof(support), support_before);
        const int status = execute_step(value, header, clock, tick, &station, &support);
        if (status != 0) {
            if (!fist_probe_unchanged(&value->world, sizeof(value->world), &value->before) ||
                (header[H_OPERATION] == STATION &&
                 !fist_probe_unchanged(&station, sizeof(station), station_before)) ||
                (header[H_OPERATION] == SUPPORT &&
                 !fist_probe_unchanged(&support, sizeof(support), support_before))) {
                return -1;
            }
            station = (fist_ground_station_result){0};
            support = (fist_ground_support_result){0};
        } else if (header[H_OPERATION] != CONFIGURE &&
                   unchanged_owners(value, &support, header[H_OPERATION] == SUPPORT) != 0) {
            return -1;
        }
        observe(value, status, &station, &support);
        if (header[H_POST_REQUESTER] != 0 &&
            (status != 0 ||
             requester_retention(value, &support, header[H_POST_REQUESTER] == 2) != 0)) {
            return -1;
        }
    }
    return 0;
}

static uint8_t *read_file(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    uint8_t *data = fist_probe_read_file(file, size);
    if (fclose(file) != 0) {
        free(data);
        return NULL;
    }
    return data;
}

static int load_prepared(const char *path, const fist_klc_image *height,
                         fist_mission_world *world) {
    size_t size = 0;
    uint8_t *data = read_file(path, &size);
    fist_scenario scenario = {0};
    fist_units units = {0};
    fist_mission_orders orders = {0};
    const fist_random random = {.words = {1, 2, HIGH_SEED, UINT16_MAX}, .next_stream = 3};
    int status = data == NULL ? -1 : fist_scenario_decode(data, size, &scenario);
    if (status == 0) {
        status = fist_units_decode(&scenario, &units);
    }
    if (status == 0) {
        status = fist_mission_orders_decode(&scenario, &orders);
    }
    if (status == 0) {
        status = fist_mission_world_initialize(&units, &random, 0, world);
        if (status == 0) {
            world->orders = orders;
            world->orders_loaded = 1;
        }
    }
    fist_units_destroy(&units);
    if (data != NULL) {
        fill_bytes(size, data, UINT8_MAX);
    }
    free(data);
    if (status == 0) {
        status = fist_mission_world_prepare(world, height, 0);
    }
    return status == 0 ? prepared_resources(world) : status;
}

static int canonical_batch(uint8_t *data, size_t size, size_t *start, fist_klc_image *height) {
    if (size < BATCH_HEADER) {
        return -1;
    }
    const uint32_t side = fist_read_u32le(data);
    if (side < MIN_DETAIL || side > MAX_DETAIL || (side & (side - 1)) != 0) {
        return -1;
    }
    const size_t pixels = (size_t)side * side;
    if (size < BATCH_HEADER + pixels) {
        return -1;
    }
    *start = BATCH_HEADER + pixels;
    const size_t remaining = size - *start;
    if (remaining == 0 || remaining % CASE_BYTES != 0 ||
        fist_read_u32le(data + sizeof(uint32_t)) != remaining / CASE_BYTES) {
        return -1;
    }
    *height = (fist_klc_image){.width = side, .height = side, .pixels = data + BATCH_HEADER};
    return 0;
}

static int run_batch(probe_case *value, size_t start, const uint8_t *data, size_t size) {
    for (size_t offset = start; offset < size; offset += CASE_BYTES) {
        if (run_case(value, data + offset) != 0) {
            printf("Invalid fixture or unrelated mutation at case %zu\n",
                   (offset - start) / CASE_BYTES);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    const bool canonical = argc == 4 && strcmp(argv[1], "--canonical") == 0;
    if (!canonical && argc != 2) {
        return 1;
    }
    size_t size = 0;
    uint8_t *data = read_file(argv[canonical ? 3 : 1], &size);
    if (data == NULL) {
        return 1;
    }
    probe_case *value = calloc(1, sizeof(*value));
    fist_mission_world *prepared = canonical ? calloc(1, sizeof(*prepared)) : NULL;
    size_t start = sizeof(uint32_t);
    int status = value == NULL || (canonical && prepared == NULL) ? 1 : 0;
    if (status == 0 && canonical) {
        status = canonical_batch(data, size, &start, &value->height);
        if (status == 0) {
            status = load_prepared(argv[2], &value->height, prepared);
        }
        if (status == 0) {
            puts("prepared");
            status = fist_probe_write_mission_world(prepared);
            puts("children");
            value->prepared = prepared;
        }
    } else if (status == 0 &&
               (size <= start || fist_read_u32le(data) != (size - start) / CASE_BYTES ||
                (size - start) % CASE_BYTES != 0)) {
        status = 1;
    }
    if (status == 0) {
        status = run_batch(value, start, data, size);
    }
    free(prepared);
    free(value);
    free(data);
    return status == 0 ? 0 : 1;
}
