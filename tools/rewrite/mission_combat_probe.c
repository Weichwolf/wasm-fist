#include "assets/bytes.h"
#include "assets/klc.h"
#include "assets/scenario.h"
#include "assets/units.h"
#include "mission_probe_io.h"
#include "probe_io.h"
#include "sim/driver.h"
#include "sim/mission_update.h"
#include "sim/mission_world.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"
#include "sim/world_step.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    HEADER = 40,
    COMMAND_BYTES = 7,
    CURSOR = 8,
    LINK = 9,
    SCALE = 10,
    SELECTED = 14,
    TICK = 16,
    ANIMATION = 18,
    WIND_X = 20,
    WIND_Y = 24,
    SMOKE_ENABLED = 28,
    TREE_CHANGED = 29,
    TREE_VARIANT = 30,
    COARSE = 31,
    COUNT = 32,
    HEIGHT = 36,
    HEIGHT_SIDE = 2,
    HEIGHT_PIXELS = 4,
    HEIGHT_SHIFT = 8,
    GROUND_QUERY_LIMIT = 128,
    SHELL = 8,
    FIRE = 0,
    VISIT = 1,
    RESUME = 2,
    SELECT = 3,
    RELOAD = 4,
    REQUEST = 5,
    PASS = 6,
    FIRST = 1,
    SECOND = 3,
    REPEAT = 5,
    PHASE_STEP = 2,
    MARKER = 123
};

typedef struct {
    uint16_t first;
    uint16_t second;
    uint16_t repeat;
    uint8_t operation;
} command;

typedef struct {
    command *commands;
    size_t count;
    fist_random random;
    fist_mission_update_environment environment;
    uint16_t scales[FIST_DAMAGE_SIDES];
    uint16_t selected;
    uint8_t link;
    uint8_t heights[HEIGHT_PIXELS];
} program;

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

static void capture(const void *object, uint8_t *out, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        out[index] = bytes[index];
    }
}

static uint8_t *capture_owned(const void *object, size_t size) {
    uint8_t *bytes = malloc(size);
    if (bytes != NULL) {
        capture(object, bytes, size);
    }
    return bytes;
}

static int unchanged(const void *object, const uint8_t *before, size_t size) {
    const unsigned char *bytes = object;
    for (size_t index = 0; index < size; ++index) {
        if (bytes[index] != before[index]) {
            return 0;
        }
    }
    return 1;
}

static int rejected_visit(fist_mission_world *world, fist_pool_allocation allocation,
                          const fist_mission_update_environment *environment, int null_output) {
    uint8_t *before = capture_owned(world, sizeof(*world));
    if (before == NULL) {
        return -1;
    }
    fist_mission_update_result result = {.target_type = MARKER};
    uint8_t output[sizeof(result)];
    capture(&result, output, sizeof(output));
    const int status =
        fist_mission_world_visit(world, allocation, environment, null_output != 0 ? NULL : &result);
    const int preserved = status == -1 && unchanged(world, before, sizeof(*world)) &&
                          unchanged(&result, output, sizeof(result));
    free(before);
    return preserved ? 0 : -1;
}

static int verify_rejections(fist_mission_world *world,
                             const fist_mission_update_environment *environment) {
    fist_world_pass pass = {0};
    fist_pool_allocation allocation = {0};
    const int next = fist_world_next(&world->pool, &pass, &allocation);
    if (next < 0) {
        return -1;
    }
    fist_mission_update_result output = {.target_type = MARKER};
    if (fist_mission_world_visit(NULL, allocation, environment, &output) != -1 ||
        output.target_type != MARKER || rejected_visit(world, allocation, NULL, 0) != 0 ||
        rejected_visit(world, allocation, environment, 1) != 0) {
        return -1;
    }
    fist_pool_allocation stale = allocation;
    stale.value ^= 1;
    if (rejected_visit(world, stale, environment, 0) != 0) {
        return -1;
    }
    stale = allocation;
    ++stale.type;
    if (rejected_visit(world, stale, environment, 0) != 0) {
        return -1;
    }
    stale = allocation;
    stale.slot = FIST_UNIT_REGISTRY_COUNT;
    if (rejected_visit(world, stale, environment, 0) != 0) {
        return -1;
    }
    stale = allocation;
    stale.registry_index = FIST_UNIT_REGISTRY_COUNT;
    if (rejected_visit(world, stale, environment, 0) != 0) {
        return -1;
    }
    const uint8_t stream = world->random.next_stream;
    world->random.next_stream = FIST_RANDOM_STREAMS;
    const int invalid_random = rejected_visit(world, allocation, environment, 0);
    world->random.next_stream = stream;
    uint8_t *before = capture_owned(world, sizeof(*world));
    if (before == NULL) {
        return -1;
    }
    fist_projectile_impact impact = {.sound_request = MARKER};
    uint8_t saved[sizeof(impact)];
    capture(&impact, saved, sizeof(saved));
    const int resume = fist_mission_world_resume_impact(world, &impact);
    const int null_resume = fist_mission_world_resume_impact(world, NULL);
    const int null_world = fist_mission_world_resume_impact(NULL, &impact);
    const int preserved = resume == -1 && unchanged(world, before, sizeof(*world)) &&
                          unchanged(&impact, saved, sizeof(impact));
    free(before);
    return invalid_random == 0 && preserved && null_resume == -1 && null_world == -1 ? 0 : -1;
}

static int decode(const uint8_t *data, size_t size, program *out) {
    if (size < HEADER || data[CURSOR] >= FIST_RANDOM_STREAMS || data[COARSE] > 1 ||
        (size - HEADER) % COMMAND_BYTES != 0 ||
        fist_read_u32le(data + COUNT) != (size - HEADER) / COMMAND_BYTES) {
        return -1;
    }
    out->count = (size - HEADER) / COMMAND_BYTES;
    out->commands = calloc(out->count == 0 ? 1 : out->count, sizeof(*out->commands));
    if (out->commands == NULL) {
        return -1;
    }
    for (size_t index = 0; index < out->count; ++index) {
        const uint8_t *raw = data + HEADER + (index * COMMAND_BYTES);
        if (raw[0] > PASS) {
            return -1;
        }
        out->commands[index] =
            (command){fist_read_u16le(raw + FIRST), fist_read_u16le(raw + SECOND),
                      fist_read_u16le(raw + REPEAT), raw[0]};
    }
    out->random.next_stream = data[CURSOR];
    for (size_t index = 0; index < FIST_RANDOM_STREAMS; ++index) {
        out->random.words[index] = fist_read_u16le(data + (index * sizeof(uint16_t)));
    }
    for (size_t index = 0; index < FIST_DAMAGE_SIDES; ++index) {
        out->scales[index] = fist_read_u16le(data + SCALE + (index * sizeof(uint16_t)));
    }
    for (size_t index = 0; index < HEIGHT_PIXELS; ++index) {
        out->heights[index] = data[HEIGHT + index];
    }
    out->selected = fist_read_u16le(data + SELECTED);
    out->link = data[LINK];
    out->environment = (fist_mission_update_environment){
        .weather = {fist_read_i32le(data + WIND_X), fist_read_i32le(data + WIND_Y),
                    data[SMOKE_ENABLED]},
        .trees = {data[TREE_CHANGED], data[TREE_VARIANT]},
        .tick = fist_read_u16le(data + TICK),
        .animation_phase = fist_read_u16le(data + ANIMATION),
        .coarse = data[COARSE] != 0};
    return 0;
}

static void write_combat(const fist_mission_world *world) {
    const fist_combat_state *state = &world->combat;
    printf("combat %u %u %u %u %u %u %u %u %u %u\n", (unsigned)state->source_scale[0],
           (unsigned)state->source_scale[1], (unsigned)state->selected_slot,
           (unsigned)state->damage_flash, (unsigned)state->destroyed_by_side[0],
           (unsigned)state->destroyed_by_side[1],
           (unsigned)state->clear_side_destroyed_by_clear_source,
           (unsigned)state->pair_destroyed_by_side[0], (unsigned)state->pair_destroyed_by_side[1],
           (unsigned)world->pending_player_impact);
    printf("sizes");
    for (size_t index = 0; index < FIST_DAMAGE_COUNTED_PLATOONS; ++index) {
        printf(" %u", (unsigned)state->platoon_sizes[index]);
    }
    printf("\n");
}

static int write_state(const fist_mission_world *world) {
    write_combat(world);
    return fist_probe_write_mission_world(world);
}

static int observed_visit(fist_mission_world *world, fist_pool_allocation allocation,
                          const fist_mission_update_environment *environment) {
    const fist_projectile *projectile = &world->objects[allocation.slot].projectile;
    if (allocation.type == SHELL && projectile->age == 0 && projectile->velocity.z == 0 &&
        (uint16_t)((uint32_t)projectile->pose.altitude >> HEIGHT_SHIFT) < GROUND_QUERY_LIMIT) {
        fist_mission_update_environment missing_height = *environment;
        missing_height.height = NULL;
        if (rejected_visit(world, allocation, &missing_height, 0) != 0) {
            return -1;
        }
    }
    uint8_t *before = capture_owned(world, sizeof(*world));
    if (before == NULL) {
        return -1;
    }
    fist_mission_update_result result = {.target_type = MARKER};
    uint8_t output[sizeof(result)];
    capture(&result, output, sizeof(output));
    const int status = fist_mission_world_visit(world, allocation, environment, &result);
    const int preserved = status == 0 || (unchanged(world, before, sizeof(*world)) &&
                                          unchanged(&result, output, sizeof(result)));
    free(before);
    if (!preserved) {
        return -1;
    }
    printf("visit %u %u %u %d\n", (unsigned)allocation.registry_index, (unsigned)allocation.slot,
           (unsigned)allocation.type, status);
    if (status == 0) {
        printf("event %u %u %u %u %u %u %u %u %u\n", (unsigned)result.flight.phase,
               (unsigned)result.flight.hit.slot, (unsigned)result.flight.hit.registry_index,
               (unsigned)result.flight.hit.value, (unsigned)result.flight.hit.aspect,
               (unsigned)result.target_type, (unsigned)result.damaged, (unsigned)result.has_impact,
               (unsigned)result.player_loss);
        if (result.damaged && result.target_type < FIST_UNIT_GROUND_VEHICLE_COUNT) {
            const fist_vehicle_damage_result *damage = &result.ground_damage;
            printf("ground_event %u %u %u %u %u %u %u %u %u\nvoices",
                   (unsigned)damage->applied_damage, (unsigned)damage->destroyed,
                   (unsigned)damage->has_wreck, (unsigned)damage->selected_destroyed,
                   (unsigned)damage->refresh_damage_display, (unsigned)damage->sound_request,
                   (unsigned)damage->destruction_sound_request, (unsigned)damage->explosion_count,
                   (unsigned)damage->voice_count);
            for (size_t index = 0; index < damage->voice_count; ++index) {
                printf(" %u", (unsigned)damage->voice_requests[index]);
            }
            printf("\n");
        } else if (result.damaged) {
            const fist_other_damage_result *damage = &result.other_damage;
            printf("other_event %u %u %u %u %u %u %u\n", (unsigned)damage->applied_damage,
                   (unsigned)damage->destroyed, (unsigned)damage->released,
                   (unsigned)damage->refresh_damage_display, (unsigned)damage->sound_request,
                   (unsigned)damage->voice_request, (unsigned)damage->has_explosion);
        }
        printf("births %u %u %u %u %u\n", (unsigned)result.destruction.has_smoke,
               (unsigned)result.aircraft.has_smoke, (unsigned)result.aircraft.has_explosion,
               (unsigned)result.aircraft.released, (unsigned)result.aircraft.sound_request);
        if (result.has_impact) {
            printf("impact %u %u %u %u\n", (unsigned)result.impact.has_explosion,
                   (unsigned)result.impact.notice, (unsigned)result.impact.sound_request,
                   (unsigned)result.impact.hit_voice);
        }
    }
    return write_state(world);
}

static int visit_next(fist_mission_world *world, fist_world_pass *pass,
                      const fist_mission_update_environment *environment, uint16_t limit) {
    fist_pool_allocation allocation = {0};
    const int status = fist_world_next(&world->pool, pass, &allocation);
    if (status < 0) {
        return -1;
    }
    if (status == FIST_WORLD_END || allocation.registry_index >= limit) {
        return FIST_WORLD_END;
    }
    return observed_visit(world, allocation, environment);
}

static int span(fist_mission_world *world, command request,
                const fist_mission_update_environment *environment) {
    if (request.first > request.second || request.second > FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    fist_mission_update_environment current = *environment;
    for (unsigned iteration = 0; iteration < request.repeat; ++iteration) {
        current.tick = (uint16_t)(environment->tick + iteration);
        fist_world_pass pass = {request.first};
        while (pass.next_entry < request.second) {
            const int status = visit_next(world, &pass, &current, request.second);
            if (status < 0) {
                return -1;
            }
            if (status == FIST_WORLD_END || world->pending_player_impact != FIST_POOL_NO_SLOT) {
                break;
            }
        }
        if (world->pending_player_impact != FIST_POOL_NO_SLOT) {
            break;
        }
    }
    return 0;
}

static int weapon_command(fist_mission_world *world, fist_fire_history *history, command request,
                          const fist_mission_update_environment *environment) {
    if (fist_mission_world_object(world, request.first) == NULL ||
        world->pool.slots[request.first].type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    fist_vehicle_state *actor = &world->objects[request.first].vehicle;
    fist_weapon_events events = {0};
    int status = 0;
    if (request.operation == FIRE) {
        fist_fire_result fire = {0};
        status = fist_mission_world_fire_untargeted(
            world, history,
            (fist_fire_request){{request.first, environment->coarse}, request.second}, &fire);
        printf("fire %d %u %u %u %u %u %u\n", status, (unsigned)fire.requested,
               (unsigned)fire.dispatched, (unsigned)fire.weapon_panel_refresh,
               (unsigned)fire.voice_request, (unsigned)history->failed_at,
               (unsigned)fire.launch.outcome);
    } else if (request.operation == SELECT) {
        status = request.second > UINT8_MAX
                     ? -1
                     : fist_weapon_select(actor, (uint8_t)request.second, &events);
        if (status == 0) {
            status = fist_driver_take_control(actor);
        }
    } else if (request.operation == RELOAD) {
        for (unsigned repeat = 0; repeat < request.repeat && status == 0; ++repeat) {
            actor->drive.update_phase = (uint8_t)(actor->drive.update_phase + PHASE_STEP);
            status = fist_weapon_reload_phase(actor, &events);
        }
    } else {
        status = fist_weapon_request_fire(actor);
    }
    if (request.operation != FIRE) {
        printf("command %u %d\n", (unsigned)request.operation, status);
    }
    return write_state(world);
}

static int execute(fist_mission_world *world, const program *input) {
    fist_fire_history history = {0};
    uint8_t heights[HEIGHT_PIXELS] = {0};
    for (size_t index = 0; index < HEIGHT_PIXELS; ++index) {
        heights[index] = input->heights[index];
    }
    const fist_klc_image height = {.width = HEIGHT_SIDE, .height = HEIGHT_SIDE, .pixels = heights};
    fist_mission_update_environment environment = input->environment;
    environment.height = &height;
    if (verify_rejections(world, &environment) != 0 || write_state(world) != 0) {
        return -1;
    }
    for (size_t index = 0; index < input->count; ++index) {
        const command request = input->commands[index];
        if (request.operation == PASS) {
            if (span(world, request, &environment) != 0) {
                return -1;
            }
            continue;
        }
        if (request.operation == VISIT) {
            fist_world_pass pass = {request.first};
            environment.tick = request.second;
            if (visit_next(world, &pass, &environment, FIST_UNIT_REGISTRY_COUNT) < 0) {
                return -1;
            }
            continue;
        }
        if (request.operation == RESUME) {
            fist_projectile_impact impact = {0};
            const int status = fist_mission_world_resume_impact(world, &impact);
            printf("resume %d %u %u %u %u\n", status, (unsigned)impact.has_explosion,
                   (unsigned)impact.notice, (unsigned)impact.sound_request,
                   (unsigned)impact.hit_voice);
            if (write_state(world) != 0) {
                return -1;
            }
            continue;
        }
        if (weapon_command(world, &history, request, &environment) != 0) {
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_file(argv[1], &size);
    program input = {0};
    int status = data == NULL ? -1 : decode(data, size, &input);
    free(data);
    fist_scenario scenario = {0};
    fist_units units = {0};
    fist_mission_world *world = malloc(sizeof(*world));
    if (status == 0 && world != NULL) {
        data = read_file(argv[2], &size);
        status = data == NULL ? -1 : fist_scenario_decode(data, size, &scenario);
        if (status == 0) {
            status = fist_units_decode(&scenario, &units);
        }
        free(data);
        if (status == 0) {
            status = fist_mission_world_initialize(&units, &input.random, input.link, world);
        }
        fist_units_destroy(&units);
        if (status == 0) {
            world->combat.selected_slot = input.selected;
            for (size_t index = 0; index < FIST_DAMAGE_SIDES; ++index) {
                world->combat.source_scale[index] = input.scales[index];
            }
            status = execute(world, &input);
        }
    } else {
        status = -1;
    }
    free(world);
    free(input.commands);
    return status == 0 && ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
