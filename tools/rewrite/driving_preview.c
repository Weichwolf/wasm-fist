#include "app/driving.h"
#include "app/driving_view.h"
#include "assets/scenario.h"
#include "assets/source.h"
#include "probe_io.h"
#include "probe_source.h"
#include "render/renderer.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include "platform/wasm/present.h"
#else
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_hints.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#endif

enum {
    FRAME_WIDTH = 640,
    FRAME_HEIGHT = 400,
    CHANNELS = 4,
    ARGUMENT_COUNT = 4,
    DECIMAL_BASE = 10,
    MICROSECONDS_PER_MILLISECOND = 1000,
    FRAME_PERIOD_MS = 16
};
static fist_driving driving;
static fist_renderer *renderer;

void fist_preview_destroy(void) {
    fist_renderer_destroy(renderer);
    renderer = NULL;
    fist_driving_destroy(&driving);
}

int fist_preview_advance(uint32_t elapsed_us) {
    if (renderer == NULL) {
        return -1;
    }
    const fist_driving_interval interval = {elapsed_us, driving.keys};
    return fist_driving_advance(&driving, interval);
}

/* Positive ASCII presses; negative ASCII releases. Binding/held state is C-owned. */
int fist_preview_key(int event) {
    if (renderer == NULL || event == INT_MIN) {
        return -1;
    }
    const uint16_t key = fist_driving_key(event < 0 ? -event : event);
    const uint16_t keys = event < 0 ? driving.keys & (uint16_t)~key : driving.keys | key;
    const fist_driving_interval interval = {0, keys};
    return fist_driving_advance(&driving, interval);
}

int fist_preview_freeze(void) {
    if (renderer == NULL) {
        return -1;
    }
    /* Release held keys first; an already-held P must not suppress the pause edge. */
    if (fist_driving_advance(&driving, (fist_driving_interval){0, 0}) != 0) {
        return -1;
    }
    if (driving.paused == 0 &&
        fist_driving_advance(&driving, (fist_driving_interval){0, FIST_DRIVE_PAUSE}) != 0) {
        return -1;
    }
    return fist_driving_advance(&driving, (fist_driving_interval){0, 0});
}

int fist_preview_frame(void) {
    if (renderer == NULL || fist_driving_draw(&driving, renderer) != 0) {
        return -1;
    }
#ifdef __EMSCRIPTEN__
    fist_present_rgba(fist_renderer_pixels(renderer), FRAME_WIDTH, FRAME_HEIGHT);
#endif
    return 0;
}

const uint8_t *fist_preview_pixels(void) {
    return renderer == NULL ? NULL : fist_renderer_pixels(renderer);
}

/* Observable state for the independent platform/input gate, no JS simulation. */
enum {
    VALUE_TICKS,
    VALUE_X,
    VALUE_Y,
    VALUE_HULL,
    VALUE_TURRET,
    VALUE_SPEED,
    VALUE_THROTTLE,
    VALUE_PAUSED,
    VALUE_KEYS,
    VALUE_HEIGHT,
    VALUE_WEAPON,
    VALUE_AMMUNITION,
    VALUE_RELOAD,
    VALUE_SELECTIONS,
    VALUE_RELOADS,
    VALUE_TYPE
};
double fist_preview_value(int field) {
    fist_weapon_status weapon = {0};
    if (renderer == NULL || fist_weapon_inspect(&driving.player, &weapon) != 0) {
        return -1;
    }
    switch (field) {
    case VALUE_TICKS:
        return (double)driving.ticks;
    case VALUE_X:
        return driving.player.map_x;
    case VALUE_Y:
        return driving.player.map_y;
    case VALUE_HULL:
        return driving.player.drive.heading;
    case VALUE_TURRET:
        return driving.player.turret.heading;
    case VALUE_SPEED:
        return driving.player.drive.speed;
    case VALUE_THROTTLE:
        return driving.player.drive.throttle;
    case VALUE_PAUSED:
        return driving.paused;
    case VALUE_KEYS:
        return driving.keys;
    case VALUE_HEIGHT:
        return driving.player.ground_height;
    case VALUE_WEAPON:
        return weapon.selected;
    case VALUE_AMMUNITION:
        return weapon.ammunition;
    case VALUE_RELOAD:
        return weapon.countdown;
    case VALUE_SELECTIONS:
        return (double)driving.feedback.selections;
    case VALUE_RELOADS:
        return (double)driving.feedback.reloads;
    case VALUE_TYPE:
        return driving.player.type;
    default:
        return -1;
    }
}

static int load_scene(char **argv) {
    errno = 0;
    char *end = NULL;
    const unsigned long side = strtoul(argv[3], &end, DECIMAL_BASE);
    if (errno != 0 || end == argv[3] || *end != '\0' || side == 0 || side > UINT32_MAX) {
        return -1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        return -1;
    }
    size_t size = 0;
    uint8_t *data = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    fist_scenario scenario = {0};
    int result = -1;
    if (data != NULL && closed == 0 && fist_scenario_decode(data, size, &scenario) == 0) {
        fist_probe_source storage = {.directory = argv[2]};
        const fist_asset_source source = {fist_probe_source_read, &storage};
        const fist_driving_options options = {.height_side = (uint32_t)side};
        result = fist_driving_load(&scenario, &source, &options, &driving);
        fist_probe_source_close(&storage);
    }
    free(data);
    if (result == 0) {
        renderer = fist_renderer_create(FRAME_WIDTH, FRAME_HEIGHT);
        result = renderer == NULL ? -1 : fist_preview_frame();
    }
    if (result != 0) {
        fist_preview_destroy();
    }
    return result;
}

#ifndef __EMSCRIPTEN__
static int advance_clock(uint64_t *last_ms) {
    const uint64_t now = SDL_GetTicks64();
    const uint64_t elapsed = now - *last_ms;
    if (elapsed > UINT32_MAX / MICROSECONDS_PER_MILLISECOND) {
        return -1;
    }
    *last_ms = now;
    return fist_preview_advance((uint32_t)elapsed * MICROSECONDS_PER_MILLISECOND);
}

static int present_frame(SDL_Renderer *display, SDL_Texture *texture) {
    if (fist_preview_frame() != 0) {
        return -1;
    }
    void *destination = NULL;
    int pitch = 0;
    if (SDL_LockTexture(texture, NULL, &destination, &pitch) != 0) {
        return -1;
    }
    const uint8_t *pixels = fist_preview_pixels();
    const size_t stride = (size_t)FRAME_WIDTH * CHANNELS;
    for (size_t row = 0; row < FRAME_HEIGHT; ++row) {
        uint8_t *target = (uint8_t *)destination + (row * (size_t)pitch);
        const uint8_t *source = pixels + ((FRAME_HEIGHT - row - 1) * stride);
        for (size_t index = 0; index < stride; ++index) {
            target[index] = source[index];
        }
    }
    SDL_UnlockTexture(texture);
    if (SDL_RenderClear(display) != 0 || SDL_RenderCopy(display, texture, NULL, NULL) != 0) {
        return -1;
    }
    SDL_RenderPresent(display);
    return 0;
}

static int handle_event(const SDL_Event *event) {
    switch (event->type) {
    case SDL_QUIT:
        return 1;
    case SDL_KEYDOWN:
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            return 1;
        }
        return fist_preview_key(event->key.keysym.sym);
    case SDL_KEYUP:
        return fist_preview_key(-event->key.keysym.sym);
    case SDL_WINDOWEVENT:
        if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            return fist_preview_freeze();
        }
        break;
    default:
        break;
    }
    return 0;
}

static int event_loop(SDL_Renderer *display, SDL_Texture *texture) {
    uint64_t last_ms = SDL_GetTicks64();
    uint64_t frame_ms = last_ms;
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (advance_clock(&last_ms) != 0) {
                return -1;
            }
            const int handled = handle_event(&event);
            if (handled != 0) {
                return handled > 0 ? 0 : -1;
            }
        }
        if (advance_clock(&last_ms) != 0) {
            return -1;
        }
        if (last_ms - frame_ms >= FRAME_PERIOD_MS) {
            if (present_frame(display, texture) != 0) {
                return -1;
            }
            frame_ms = last_ms;
        }
        SDL_Delay(1);
    }
}

static int native_display(void) {
    /* SDL's software renderer can otherwise create an accelerated window
     * framebuffer. softgl owns rendering; request the actual software surface. */
    if (SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0") == 0 ||
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        return -1;
    }
    SDL_Window *window = SDL_CreateWindow("Armored Fist | W/S A/D Q/E Space P | 1-5 Tab weapons",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          FRAME_WIDTH, FRAME_HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *display =
        window == NULL ? NULL : SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Texture *texture =
        display == NULL ? NULL
                        : SDL_CreateTexture(display, SDL_PIXELFORMAT_RGBA32,
                                            SDL_TEXTUREACCESS_STREAMING, FRAME_WIDTH, FRAME_HEIGHT);
    const int result = texture == NULL ? -1 : event_loop(display, texture);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(display);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
#endif

int main(int argc, char **argv) {
    if (argc != ARGUMENT_COUNT || load_scene(argv) != 0) {
        if (fputs("Usage: fist_driving_preview SCENARIO.FSG ASSET_DIRECTORY HEIGHT_SIDE\n",
                  stderr) == EOF) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
    }
#ifdef __EMSCRIPTEN__
    return EXIT_SUCCESS;
#else
    const int result = native_display();
    fist_preview_destroy();
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
}
