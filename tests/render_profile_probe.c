#include "render/renderer.h"

#include <GL/softgl.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GLIBC__)
/* LLVM 19's include cleaner requires glibc's direct typedef provider for
 * the linker-boundary signatures, in addition to the public pthread API. */
#include <bits/pthreadtypes.h>
#endif

#ifdef __EMSCRIPTEN__
#include "platform/wasm/present.h"
#endif

enum { FRAME_WIDTH = 640, FRAME_HEIGHT = 360, RGBA_CHANNELS = 4, CYCLE_COUNT = 12 };

/* Linker fault injection at the actual pthread boundary, with no dependency
 * edits. Assembly names are the linker's wrap ABI; owned C identifiers remain
 * ordinary names. All creation calls are made by the calling render thread. */
int fist_real_pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                             void *(*entry)(void *),
                             void *argument) __asm__("__real_pthread_create");
int fist_test_pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                             void *(*entry)(void *),
                             void *argument) __asm__("__wrap_pthread_create");

static int failure_mode;
static unsigned create_calls;
static unsigned failed_calls;
static unsigned successful_calls;
static pthread_t caller;

int fist_test_pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                             void *(*entry)(void *), void *argument) {
    if (pthread_equal(caller, pthread_self()) == 0) {
        return EAGAIN;
    }
    ++create_calls;
    if (failure_mode == 1 || (failure_mode == 2 && create_calls % 2 == 0)) {
        ++failed_calls;
        return EAGAIN;
    }
    const int result = fist_real_pthread_create(thread, attributes, entry, argument);
    successful_calls += result == 0;
    return result;
}

static int profile_matches(fist_renderer *renderer, int width, int height, int samples) {
    fist_render_profile profile = {0};
    return fist_renderer_get_profile(renderer, &profile) == 0 && profile.width == width &&
           profile.height == height && profile.samples == samples &&
           profile.sample_buffers == (samples != 0) &&
           profile.configured_helpers == FIST_RENDER_HELPERS &&
           profile.started_helpers == FIST_RENDER_HELPERS &&
           profile.total_threads == FIST_RENDER_HELPERS + 1 &&
           (samples == 0 || profile.multisample_enabled != 0);
}

static int contracts(void) {
    fist_render_profile profile = {.width = FRAME_WIDTH, .height = FRAME_HEIGHT};
    const fist_render_profile before = profile;
    if (fist_renderer_get_profile(NULL, &profile) != -1 ||
        memcmp(&profile, &before, sizeof(profile)) != 0 ||
        fist_renderer_get_profile(NULL, NULL) != -1) {
        return EXIT_FAILURE;
    }
    fist_renderer_destroy(NULL);
    static const int invalid[][3] = {
        {0, 1, 0},  {1, 0, 4}, {-1, 1, 4}, {1, -1, 0}, {INT_MAX, 2, 0}, {INT_MAX, INT_MAX, 4},
        {1, 1, -1}, {1, 1, 1}, {1, 1, 2},  {1, 1, 8}};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        fist_renderer *renderer = fist_renderer_create_multisample(
            invalid[index][0], invalid[index][1], invalid[index][2]);
        if (renderer != NULL) {
            fist_renderer_destroy(renderer);
            return EXIT_FAILURE;
        }
    }
    fist_renderer *renderer = fist_renderer_create(1, 1);
    if (renderer == NULL) {
        return EXIT_FAILURE;
    }
    const int valid =
        profile_matches(renderer, 1, 1, 0) && fist_renderer_get_profile(renderer, NULL) == -1;
    if (valid == 0 || fist_renderer_draw_probe(renderer) != 0) {
        fist_renderer_destroy(renderer);
        return EXIT_FAILURE;
    }
    glEnable(UINT32_MAX);
    const int preserved_error =
        fist_renderer_get_profile(renderer, &profile) == 0 && glGetError() == GL_INVALID_ENUM;
    fist_renderer_destroy(renderer);
    return preserved_error != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int cycles(void) {
    static const int dimensions[][2] = {{FRAME_WIDTH, FRAME_HEIGHT}, {17, 11}, {1, 1}};
    for (unsigned cycle = 0; cycle < CYCLE_COUNT; ++cycle) {
        const int width = dimensions[cycle % 3][0];
        const int height = dimensions[cycle % 3][1];
        const int samples = cycle % 2 == 0 ? FIST_RENDER_TARGET_SAMPLES : 0;
        fist_renderer *renderer = fist_renderer_create_multisample(width, height, samples);
        if (renderer == NULL) {
            return EXIT_FAILURE;
        }
        int valid = profile_matches(renderer, width, height, samples);
        for (unsigned draw = 0; valid != 0 && draw < 2; ++draw) {
            valid = fist_renderer_draw_probe(renderer) == 0;
            const uint8_t expected[] = {(cycle % 2) * UINT8_MAX, draw * UINT8_MAX,
                                        (cycle % 3 == 0) * UINT8_MAX, UINT8_MAX};
            glClearColor((GLclampf)(cycle % 2), (GLclampf)draw, (GLclampf)(cycle % 3 == 0), 1);
            glClear(GL_COLOR_BUFFER_BIT);
            const uint8_t *pixels = fist_renderer_pixels(renderer);
            if (pixels == NULL) {
                valid = 0;
                break;
            }
            const size_t bytes = (size_t)width * (size_t)height * RGBA_CHANNELS;
            for (size_t index = 0; index < bytes; ++index) {
                valid = valid != 0 && pixels[index] == expected[index % RGBA_CHANNELS];
            }
        }
        fist_renderer_destroy(renderer);
        if (valid == 0) {
            return EXIT_FAILURE;
        }
    }
    return printf("{\"cycles\":%d}\n", CYCLE_COUNT) > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int failure(int mode) {
    failure_mode = mode;
    fist_renderer *renderer =
        fist_renderer_create_multisample(FRAME_WIDTH, FRAME_HEIGHT, FIST_RENDER_TARGET_SAMPLES);
    if (renderer != NULL) {
        fist_renderer_destroy(renderer);
        return EXIT_FAILURE;
    }
    const unsigned failures = failed_calls;
    const unsigned successes = successful_calls;
    if (failures == 0 || (mode == 2 && successes == 0)) {
        return EXIT_FAILURE;
    }
    failure_mode = 0;
    renderer =
        fist_renderer_create_multisample(FRAME_WIDTH, FRAME_HEIGHT, FIST_RENDER_TARGET_SAMPLES);
    if (renderer == NULL) {
        return EXIT_FAILURE;
    }
    const int valid =
        profile_matches(renderer, FRAME_WIDTH, FRAME_HEIGHT, FIST_RENDER_TARGET_SAMPLES) &&
        fist_renderer_draw_probe(renderer) == 0 && fist_renderer_pixels(renderer) != NULL;
    fist_renderer_destroy(renderer);
    if (valid == 0) {
        return EXIT_FAILURE;
    }
    return printf("{\"failed_starts\":%u,\"partial_starts\":%u,\"recovered_helpers\":%d}\n",
                  failures, successes, FIST_RENDER_HELPERS) > 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

static int render_fixture(fist_renderer *renderer) {
    /* Pixel-space integration geometry, never gameplay constants. */
    static const GLfloat triangles[][2] = {
        {24.25F, 24.25F},  {280.25F, 24.25F}, {24.25F, 216.25F},
        {400.25F, 100.0F}, {400.5F, 100.0F},  {400.375F, 100.25F},
        {460.0F, 180.5F},  {460.25F, 180.5F}, {460.125F, 180.75F}};
    if (fist_renderer_draw_probe(renderer) != 0) {
        return -1;
    }
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, FRAME_WIDTH, 0, FRAME_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1, 1, 1);
    glBegin(GL_TRIANGLES);
    for (size_t index = 0; index < sizeof(triangles) / sizeof(triangles[0]); ++index) {
        glVertex2fv(triangles[index]);
    }
    glEnd();
    return glGetError() == GL_NO_ERROR ? 0 : -1;
}

static int render(int samples, const char *path) {
    fist_renderer *renderer = fist_renderer_create_multisample(FRAME_WIDTH, FRAME_HEIGHT, samples);
    if (renderer == NULL) {
        return EXIT_FAILURE;
    }
    fist_render_profile profile = {0};
    if (fist_renderer_get_profile(renderer, &profile) != 0 || render_fixture(renderer) != 0) {
        fist_renderer_destroy(renderer);
        return EXIT_FAILURE;
    }
    const uint8_t *pixels = fist_renderer_pixels(renderer);
    FILE *output = fopen(path, "wb");
    if (pixels == NULL || output == NULL) {
        if (output != NULL) {
            (void)fclose(output);
        }
        fist_renderer_destroy(renderer);
        return EXIT_FAILURE;
    }
    const size_t bytes = (size_t)FRAME_WIDTH * FRAME_HEIGHT * RGBA_CHANNELS;
    const int written = fwrite(pixels, 1, bytes, output) == bytes;
    const int closed = fclose(output);
#ifdef __EMSCRIPTEN__
    fist_present_rgba(pixels, FRAME_WIDTH, FRAME_HEIGHT);
#endif
    fist_renderer_destroy(renderer);
    if (written == 0 || closed != 0) {
        return EXIT_FAILURE;
    }
    return printf("{\"width\":%d,\"height\":%d,\"sample_buffers\":%d,\"samples\":%d,"
                  "\"multisample_enabled\":%d,\"configured_helpers\":%d,\"started_helpers\":%d,"
                  "\"total_threads\":%d}\n",
                  profile.width, profile.height, profile.sample_buffers, profile.samples,
                  profile.multisample_enabled, profile.configured_helpers, profile.started_helpers,
                  profile.total_threads) > 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

int main(int argc, char **argv) {
    caller = pthread_self();
    if (argc == 2) {
        if (strcmp(argv[1], "contracts") == 0) {
            return contracts();
        }
        if (strcmp(argv[1], "cycles") == 0) {
            return cycles();
        }
        if (strcmp(argv[1], "failure-all") == 0 || strcmp(argv[1], "failure-partial") == 0) {
            return failure(strcmp(argv[1], "failure-all") == 0 ? 1 : 2);
        }
    }
    if (argc == 3 && (strcmp(argv[1], "0") == 0 || strcmp(argv[1], "4") == 0)) {
        return render(strcmp(argv[1], "0") == 0 ? 0 : FIST_RENDER_TARGET_SAMPLES, argv[2]);
    }
    return EXIT_FAILURE;
}
