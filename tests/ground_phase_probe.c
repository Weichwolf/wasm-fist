#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "assets/view.h"
#include "mission_probe_io.h"
#include "probe_io.h"
#include "sim/ground_support.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"
#include "sim/weapon_control.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_BYTES = 80,
    BATCH_HEADER_BYTES = 8,
    ACTOR_BYTES = FIST_UNIT_EXTENDED_SIZE,
    TARGET_BYTES = FIST_UNIT_EXTENDED_SIZE,
    ORDERS_BYTES = FIST_ORDER_PATH_BLOCK_BYTES + FIST_ORDER_DESCRIPTOR_BLOCK_BYTES,
    CASE_BYTES = HEADER_BYTES + ACTOR_BYTES + TARGET_BYTES + ORDERS_BYTES,
    MIN_DETAIL = 512,
    MAX_DETAIL = 4096,
    HIGH_SEED = 32768,
    OUTPUT_CANARY = 0x5a,
    REFERENCE_RELEASE = 1,
    REFERENCE_REUSE = 2
};

enum {
    H_ACTOR = 0,
    H_TARGET = 2,
    H_CANDIDATE = 4,
    H_DIAGNOSTIC = 6,
    H_SELECTED = 8,
    H_SOURCE = 10,
    H_TICK = 12,
    H_CLOCK = 14,
    H_GATE = 16,
    H_SAVED = 18,
    H_INHIBITION = 20,
    H_CONTEXT = 21,
    H_LINK = 22,
    H_COARSE = 23,
    H_SEEDS = 24,
    H_CURSOR = 32,
    H_RETAIN = 33,
    H_ORDERS_LOADED = 34,
    H_INVALID = 35,
    H_VOICE_PRIOR = 36,
    H_REQUEST_PRIOR = 38,
    H_AIR_STOCK = 40,
    H_AIR_DELAY = 44,
    H_ARTILLERY_DELAY = 48,
    H_AIR_CLOCK = 52,
    H_ARTILLERY_CLOCK = 56,
    H_CONFIGURED = 60,
    H_REVERSE = 61,
    H_TARGET_OVERRIDE = 62,
    H_NOTICE_KIND = 63,
    H_NOTICE_TICKS = 64,
    H_MESSAGE_TICKS = 66,
    H_ADVISORY_CODE = 68,
    H_ADVISORY_ACTIVE = 69,
    H_ADVISORY_UNTIL = 70,
    H_SELECTOR = 72,
    H_DIAGNOSTIC_LIFETIME = 74,
    H_TARGET_LIFETIME = 75,
    H_CANDIDATE_LIFETIME = 76,
    H_NO_HEIGHT = 77,
    H_RESERVED = 78
};

enum {
    INVALID_NONE,
    INVALID_PREPARATION,
    INVALID_RANDOM,
    INVALID_COMPONENT,
    INVALID_CANDIDATE,
    INVALID_TARGET,
    INVALID_RESOURCE_COUNT
};

typedef struct {
    fist_mission_world prepared;
    fist_mission_world world;
    fist_mission_world before;
    fist_object_reference diagnostic;
    fist_klc_image height;
} probe_state;

static void fill_bytes(size_t size, void *object, uint8_t value) {
    uint8_t *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        bytes[index] = value;
    }
}

static uint8_t *read_path(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    uint8_t *data = fist_probe_read_file(file, size);
    const int status = fclose(file);
    if (status != 0) {
        free(data);
        return NULL;
    }
    return data;
}

static int restore_payload(fist_mission_world *world, uint16_t slot, const uint8_t *raw) {
    fist_pool_allocation allocation = {0};
    if (fist_object_pool_find(&world->pool, slot, &allocation) != 0 ||
        allocation.type != fist_read_u16le(raw)) {
        return -1;
    }
    const size_t bytes = allocation.type < FIST_UNIT_GROUND_VEHICLE_COUNT ? FIST_UNIT_EXTENDED_SIZE
                                                                          : FIST_UNIT_SHORT_SIZE;
    const fist_unit_definition definition = {.type = allocation.type,
                                             .registry_index = allocation.registry_index,
                                             .generation = allocation.value,
                                             .map_x = fist_read_i32le(raw + 4),
                                             .map_y = fist_read_i32le(raw + 8),
                                             .altitude = fist_read_i32le(raw + 12),
                                             .snapshot = {raw, bytes}};
    if (allocation.type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return fist_vehicle_restore(&definition, &world->objects[slot].vehicle);
    }
    return fist_other_actor_restore(&definition, allocation, &world->objects[slot].other);
}

static int capture(const fist_mission_world *world, uint16_t slot, fist_object_reference *out) {
    *out = (fist_object_reference){0};
    return slot == FIST_POOL_NO_SLOT ? 0 : fist_object_pool_reference(&world->pool, slot, out);
}

static int change_lifetime(fist_mission_world *world, fist_object_reference reference,
                           uint8_t operation) {
    if (operation == 0) {
        return 0;
    }
    fist_pool_allocation allocation = {0};
    if (operation > REFERENCE_REUSE ||
        fist_object_pool_find(&world->pool, reference.slot, &allocation) != 0) {
        return -1;
    }
    const fist_mission_object payload = world->objects[allocation.slot];
    fist_pool_allocation released = {0};
    if (fist_object_pool_release(&world->pool, allocation.registry_index, &released) != 0 ||
        fist_object_pool_reference_is_live(&world->pool, reference)) {
        return -1;
    }
    if (operation == REFERENCE_RELEASE) {
        return 0;
    }
    fist_pool_allocation replacement = {0};
    if (fist_object_pool_import(
            &world->pool,
            (fist_pool_import){allocation.type, allocation.registry_index, allocation.value},
            &replacement) != 0 ||
        replacement.slot != allocation.slot ||
        fist_object_pool_reference_is_live(&world->pool, reference)) {
        return -1;
    }
    world->objects[replacement.slot] = payload;
    return 0;
}

static int install_case(probe_state *state, const uint8_t *data) {
    const uint16_t slot = fist_read_u16le(data + H_ACTOR);
    if (data[H_RETAIN] > 1 || data[H_COARSE] > 1 || data[H_CONFIGURED] > 1 || data[H_REVERSE] > 1 ||
        data[H_TARGET_OVERRIDE] > 1 || data[H_NO_HEIGHT] > 1 || data[H_ADVISORY_ACTIVE] > 1 ||
        data[H_INVALID] > INVALID_RESOURCE_COUNT || fist_read_u16le(data + H_RESERVED) != 0 ||
        slot >= FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    if (data[H_RETAIN] == 0) {
        state->world = state->prepared;
        if (restore_payload(&state->world, slot, data + HEADER_BYTES) != 0) {
            return -1;
        }
        const uint16_t target = fist_read_u16le(data + H_TARGET);
        if (data[H_TARGET_OVERRIDE] &&
            restore_payload(&state->world, target, data + HEADER_BYTES + ACTOR_BYTES) != 0) {
            return -1;
        }
        fist_vehicle_command *command = &state->world.objects[slot].vehicle.command;
        if (capture(&state->world, target, &command->target) != 0 ||
            capture(&state->world, fist_read_u16le(data + H_CANDIDATE), &command->candidate) != 0 ||
            capture(&state->world, fist_read_u16le(data + H_DIAGNOSTIC), &state->diagnostic) != 0) {
            return -1;
        }
        const uint8_t *blocks = data + HEADER_BYTES + ACTOR_BYTES + TARGET_BYTES;
        fist_scenario scenario = {0};
        scenario.chunks[FIST_SCENARIO_PATHS] =
            (fist_asset_view){blocks, FIST_ORDER_PATH_BLOCK_BYTES};
        scenario.chunks[FIST_SCENARIO_PLAYER_INFO] = (fist_asset_view){
            blocks + FIST_ORDER_PATH_BLOCK_BYTES, FIST_ORDER_DESCRIPTOR_BLOCK_BYTES};
        if (fist_mission_orders_decode(&scenario, &state->world.orders) != 0) {
            return -1;
        }
        state->world.orders_loaded = data[H_ORDERS_LOADED];
        for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
            state->world.random.words[index] = fist_read_u16le(data + H_SEEDS + (index * 2));
        }
        state->world.random.next_stream = data[H_CURSOR];
        fist_support_configuration configuration = {.reverse_aircraft = data[H_REVERSE] != 0};
        for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
            configuration.air_stock[side] = fist_read_u16le(data + H_AIR_STOCK + (side * 2));
            configuration.air_delay[side] = fist_read_u16le(data + H_AIR_DELAY + (side * 2));
            configuration.artillery_delay[side] =
                fist_read_u16le(data + H_ARTILLERY_DELAY + (side * 2));
        }
        if (fist_mission_world_configure_support(&state->world, &configuration,
                                                 fist_read_u16le(data + H_CLOCK)) != 0) {
            return -1;
        }
        state->world.support.configured = data[H_CONFIGURED] != 0;
        state->world.support.last_request = fist_read_u16le(data + H_REQUEST_PRIOR);
        for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
            state->world.support.air_clock[side] = fist_read_u16le(data + H_AIR_CLOCK + (side * 2));
            state->world.support.artillery_clock[side] =
                fist_read_u16le(data + H_ARTILLERY_CLOCK + (side * 2));
        }
        state->world.voice.admitted_at = fist_read_u16le(data + H_VOICE_PRIOR);
        state->world.target_notice = (fist_target_notice){
            .duration = fist_read_u16le(data + H_NOTICE_TICKS), .kind = data[H_NOTICE_KIND]};
        state->world.artillery_message_ticks = fist_read_u16le(data + H_MESSAGE_TICKS);
        state->world.advisory =
            (fist_timed_advisory){.until = fist_read_u16le(data + H_ADVISORY_UNTIL),
                                  .code = data[H_ADVISORY_CODE],
                                  .active = data[H_ADVISORY_ACTIVE] != 0};
        state->world.sound_selector = fist_read_u16le(data + H_SELECTOR);
    }
    fist_vehicle_command *command = &state->world.objects[slot].vehicle.command;
    if (change_lifetime(&state->world, state->diagnostic, data[H_DIAGNOSTIC_LIFETIME]) != 0 ||
        change_lifetime(&state->world, command->target, data[H_TARGET_LIFETIME]) != 0 ||
        change_lifetime(&state->world, command->candidate, data[H_CANDIDATE_LIFETIME]) != 0) {
        return -1;
    }
    switch (data[H_INVALID]) {
    case INVALID_NONE:
        break;
    case INVALID_PREPARATION:
        state->world.preparation.prepared = 0;
        break;
    case INVALID_RANDOM:
        state->world.random.next_stream = FIST_RANDOM_STREAMS;
        break;
    case INVALID_COMPONENT:
        state->world.objects[slot].vehicle.component_size = 0;
        break;
    case INVALID_CANDIDATE:
        command->candidate = (fist_object_reference){.slot = 1};
        break;
    case INVALID_TARGET:
        command->target = (fist_object_reference){.slot = 1};
        break;
    case INVALID_RESOURCE_COUNT:
        state->world.preparation.artillery_count[1] = FIST_MISSION_ARTILLERY_SIDE_SLOTS + 1;
        break;
    default:
        return -1;
    }
    state->world.combat.selected_slot = fist_read_u16le(data + H_SELECTED);
    return 0;
}

static void write_reference(const fist_mission_world *world, fist_object_reference reference) {
    printf(" %u %u %u", (unsigned)reference.slot, (unsigned)(reference.lifetime != 0),
           (unsigned)(fist_object_pool_reference_is_live(&world->pool, reference) != 0));
}

static void write_voice(const char *label, fist_voice_request voice) {
    printf("%s %u %u %lu %u\n", label, (unsigned)voice.ax, (unsigned)voice.dx,
           (unsigned long)voice.ecx, (unsigned)voice.emitted);
}

static void write_allocation(fist_pool_allocation allocation) {
    printf(" %u %u %u %u", (unsigned)allocation.type, (unsigned)allocation.slot,
           (unsigned)allocation.registry_index, (unsigned)allocation.value);
}

static void write_result(const fist_mission_world *world, const fist_ground_phase_result *result) {
    printf("phase %u %u %u %u %u %u\n", (unsigned)result->phase_random, (unsigned)result->callback,
           (unsigned)result->automatic, (unsigned)result->heading_sampled,
           (unsigned)result->diagnostic_emitted, (unsigned)result->drive.refresh_drive_display);
    const fist_ground_diagnostic *diagnostic = &result->diagnostic;
    printf("diagnostic");
    write_reference(world, diagnostic->actor);
    write_reference(world, diagnostic->candidate);
    printf(" %u %u %u %u %u %u %u %u %u %u %u %u\n", (unsigned)diagnostic->throttle_bits,
           (unsigned)diagnostic->platoon_speed, (unsigned)diagnostic->route_points,
           (unsigned)diagnostic->saved, (unsigned)diagnostic->navigation_range,
           (unsigned)diagnostic->mode, (unsigned)diagnostic->discovery_count,
           (unsigned)diagnostic->maneuver, (unsigned)diagnostic->platoon,
           (unsigned)diagnostic->member, (unsigned)diagnostic->inhibited,
           (unsigned)diagnostic->candidate_live);
    printf("discovery");
    write_reference(world, result->discovery.primary);
    write_reference(world, result->discovery.secondary);
    printf(" %u %u %u %u\n", (unsigned)result->discovery.primary_range,
           (unsigned)result->discovery.secondary_operand, (unsigned)result->discovery.priority,
           (unsigned)result->discovery.count);
    write_voice("discovery_voice", result->discovery.voice);
    printf("acquisition %u %u %u\n", (unsigned)result->acquisition.attempted,
           (unsigned)result->acquisition.installed, (unsigned)result->acquisition.message);
    write_voice("acquisition_voice", result->acquisition.voice);
    printf("fire");
    write_allocation(result->fire.missile);
    printf(" %u %u %u %u %u\n", (unsigned)result->fire.rack, (unsigned)result->fire.requested,
           (unsigned)result->fire.armed, (unsigned)result->fire.launched,
           (unsigned)result->fire.message);
    write_voice("fire_sound", result->fire.sound);
    const fist_weapon_events *weapon = &result->station.weapon;
    printf("station %u %u %u %u %u %u\n", (unsigned)weapon->station_changed,
           (unsigned)weapon->timer_expired, (unsigned)weapon->voice_request,
           (unsigned)weapon->notice_request, (unsigned)weapon->notice_ticks,
           (unsigned)result->station.notice);
    write_voice("station_voice", result->station.voice);
    printf("support");
    write_allocation(result->support.marker);
    printf(" %u %u %u %u %u %u\n", (unsigned)result->support.resource_slot,
           (unsigned)result->support.support, (unsigned)result->support.smoke,
           (unsigned)result->support.queue_index, (unsigned)result->support.notice,
           (unsigned)result->support.message);
    write_voice("support_sound", result->support.sound);
}

static int write_world(const fist_mission_world *world) {
    if (fist_probe_write_mission_world(world) != 0) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (world->pool.slots[slot].used &&
            world->pool.slots[slot].type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
            const fist_vehicle_command *command = &world->objects[slot].vehicle.command;
            const fist_vehicle_weapons *weapons = &world->objects[slot].vehicle.weapons;
            printf("ground_extra %zu %u %u %u %u %u %u %u %u %u %u %u %u", slot,
                   (unsigned)world->objects[slot].vehicle.platoon,
                   (unsigned)world->objects[slot].vehicle.member, (unsigned)command->maneuver_count,
                   (unsigned)command->blocked_count, (unsigned)command->retreat_count,
                   (unsigned)command->maneuver_heading, (unsigned)command->secondary_heading,
                   (unsigned)command->discovery_count, (unsigned)command->target_range,
                   (unsigned)command->target_heading, (unsigned)weapons->rack_state[0],
                   (unsigned)weapons->rack_state[1]);
            write_reference(world, command->target);
            write_reference(world, command->candidate);
            puts("");
        }
    }
    const fist_target_notice *notice = &world->target_notice;
    printf("notifications %u %u %u %u %u %u %u %u %u %u %u\n", (unsigned)world->voice.admitted_at,
           (unsigned)notice->duration, (unsigned)notice->type, (unsigned)notice->variant,
           (unsigned)notice->enemy, (unsigned)notice->kind, (unsigned)world->sound_selector,
           (unsigned)world->advisory.until, (unsigned)world->advisory.code,
           (unsigned)world->advisory.active, (unsigned)world->artillery_message_ticks);
    const fist_mission_support *support = &world->support;
    printf("support_state %u %u %u\n", (unsigned)support->configured,
           (unsigned)support->last_request, (unsigned)support->configuration.reverse_aircraft);
    for (size_t side = 0; side < FIST_DAMAGE_SIDES; ++side) {
        printf("support_side %zu %u %u %u %u %u %u\n", side,
               (unsigned)support->configuration.air_stock[side],
               (unsigned)support->configuration.air_delay[side],
               (unsigned)support->configuration.artillery_delay[side],
               (unsigned)support->air_clock[side], (unsigned)support->artillery_clock[side],
               (unsigned)support->air_type[side]);
        for (size_t entry = 0; entry < FIST_SUPPORT_QUEUE_SLOTS; ++entry) {
            printf("air %zu %zu", side, entry);
            write_reference(world, support->air[side][entry].requester);
            printf(" %u\n", (unsigned)support->air[side][entry].tick);
            const fist_artillery_support_entry *artillery = &support->artillery[side][entry];
            printf("art %zu %zu %u %u %ld %ld\n", side, entry, (unsigned)artillery->phase,
                   (unsigned)artillery->clock, (long)artillery->target.x,
                   (long)artillery->target.y);
        }
    }
    return 0;
}

/* Commands may promote the physical roster, but do not own damage/census,
 * preparation, the selected-impact handoff or existing allocation lifetimes.
 * Compare their complete bytes, including fields absent from legacy text I/O. */
static int unrelated_owners_unchanged(const probe_state *state) {
    fist_combat_state combat = state->world.combat;
    fist_probe_capture(state->before.combat.roster, sizeof(combat.roster), combat.roster);
    if (!fist_probe_unchanged(&combat, sizeof(combat), &state->before.combat) ||
        !fist_probe_unchanged(&state->world.preparation, sizeof(state->world.preparation),
                              &state->before.preparation) ||
        state->world.pending_player_impact != state->before.pending_player_impact ||
        state->world.orders_loaded != state->before.orders_loaded) {
        return -1;
    }
    for (size_t slot = 0; slot < FIST_UNIT_REGISTRY_COUNT; ++slot) {
        if (state->before.pool.slots[slot].used &&
            state->before.pool.lifetimes[slot] != state->world.pool.lifetimes[slot]) {
            return -1;
        }
    }
    return 0;
}

static int run_case(probe_state *state, const uint8_t *input, size_t index) {
    if ((index == 0 && input[H_RETAIN] != 0) || install_case(state, input) != 0) {
        return -1;
    }
    state->before = state->world;
    fist_ground_phase_result result;
    uint8_t result_before[sizeof(result)];
    fill_bytes(sizeof(result), &result, OUTPUT_CANARY);
    fist_probe_capture(&result, sizeof(result), result_before);
    const fist_ground_phase_request request = {.slot = fist_read_u16le(input + H_ACTOR),
                                               .tick = fist_read_u16le(input + H_TICK),
                                               .clock = fist_read_u16le(input + H_CLOCK),
                                               .voice_gate = fist_read_u16le(input + H_GATE),
                                               .sound_source = fist_read_u16le(input + H_SOURCE),
                                               .diagnostic_saved = fist_read_u16le(input + H_SAVED),
                                               .diagnostic_actor = state->diagnostic,
                                               .inhibition = input[H_INHIBITION],
                                               .notice_context = input[H_CONTEXT],
                                               .link_mode = input[H_LINK],
                                               .coarse = input[H_COARSE] != 0};
    const int status = fist_mission_world_ground_command(
        &state->world, input[H_NO_HEIGHT] ? NULL : &state->height, request, &result);
    printf("case %zu %d\n", index, status);
    if (status != 0) {
        if (!fist_probe_unchanged(&state->world, sizeof(state->world), &state->before) ||
            !fist_probe_unchanged(&result, sizeof(result), result_before)) {
            return -1;
        }
        puts("atomic 1");
        return 0;
    }
    if (unrelated_owners_unchanged(state) != 0) {
        return -1;
    }
    write_result(&state->world, &result);
    return write_world(&state->world);
}

static int load_prepared(const char *path, probe_state *state) {
    size_t size = 0;
    uint8_t *data = read_path(path, &size);
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
        status = fist_mission_world_initialize(&units, &random, 0, &state->prepared);
        if (status == 0) {
            state->prepared.orders = orders;
            state->prepared.orders_loaded = 1;
        }
    }
    fist_units_destroy(&units);
    if (data != NULL) {
        fill_bytes(size, data, UINT8_MAX);
    }
    free(data);
    return status == 0 ? fist_mission_world_prepare(&state->prepared, &state->height, 0) : status;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return 1;
    }
    size_t size = 0;
    uint8_t *data = read_path(argv[2], &size);
    probe_state *state = calloc(1, sizeof(*state));
    if (data == NULL || state == NULL || size < BATCH_HEADER_BYTES) {
        free(state);
        free(data);
        return 1;
    }
    const uint32_t side = fist_read_u32le(data);
    const uint32_t count = fist_read_u32le(data + sizeof(uint32_t));
    int status = side < MIN_DETAIL || side > MAX_DETAIL || (side & (side - 1)) != 0;
    size_t start = BATCH_HEADER_BYTES;
    if (status == 0) {
        start += (size_t)side * side;
        status = size < start;
        if (status == 0) {
            const size_t remaining = size - start;
            status = count == 0 || remaining % CASE_BYTES != 0 || remaining / CASE_BYTES != count;
        }
    }
    if (status == 0) {
        state->height =
            (fist_klc_image){.width = side, .height = side, .pixels = data + BATCH_HEADER_BYTES};
        status = load_prepared(argv[1], state);
    }
    if (status == 0) {
        puts("prepared");
        status = write_world(&state->prepared);
    }
    for (size_t index = 0; status == 0 && index < count; ++index) {
        uint8_t input[CASE_BYTES];
        fist_probe_capture(data + start + (index * CASE_BYTES), sizeof(input), input);
        fill_bytes(sizeof(input), data + start + (index * CASE_BYTES), UINT8_MAX);
        status = run_case(state, input, index);
        fill_bytes(sizeof(input), input, UINT8_MAX);
    }
    free(state);
    free(data);
    return status == 0 ? 0 : 1;
}
