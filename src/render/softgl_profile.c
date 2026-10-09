#include "render/softgl_profile.h"
#include "render/renderer.h"

#include <GL/softgl.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>
#include <workers.h>

enum { RGBA_COMPONENTS = 4 };

int fist_softgl_get_profile(softgl_ctx *context, fist_render_profile *out) {
    if (context == NULL || out == NULL) {
        return -1;
    }
    const sg_worker_pool *pool = context->workers;
    fist_render_profile profile = {.width = context->fb.w,
                                   .height = context->fb.h,
                                   .configured_helpers = sg_thread_count(context)};
    for (int index = 0; pool != NULL && index < pool->nworkers; ++index) {
        profile.started_helpers += pool->workers[index].started != 0;
    }
    profile.total_threads = profile.started_helpers + 1;

    softgl_ctx *previous = sg_current();
    softgl_make_current(context);
    glGetIntegerv(GL_SAMPLE_BUFFERS, &profile.sample_buffers);
    glGetIntegerv(GL_SAMPLES, &profile.samples);
    profile.multisample_enabled = glIsEnabled(GL_MULTISAMPLE) == GL_TRUE;
    softgl_make_current(previous);
    *out = profile;
    return 0;
}

softgl_ctx *fist_softgl_create_profile(int width, int height, int samples) {
    if (width <= 0 || height <= 0 || (samples != 0 && samples != FIST_RENDER_TARGET_SAMPLES) ||
        width > INT32_MAX / RGBA_COMPONENTS / height) {
        return NULL;
    }
    softgl_ctx *context = softgl_create_multisample(width, height, samples);
    if (context == NULL) {
        return NULL;
    }
    /* Upstream creates an automatic pool. Replace it while no draw is queued;
     * explicit hints count helpers and exclude the calling render thread. */
    sg_workers_shutdown(context);
    sg_workers_init(context, FIST_RENDER_HELPERS);
    fist_render_profile profile = {0};
    if (fist_softgl_get_profile(context, &profile) != 0 || profile.samples != samples ||
        profile.sample_buffers != (samples != 0) ||
        profile.configured_helpers != FIST_RENDER_HELPERS ||
        profile.started_helpers != FIST_RENDER_HELPERS ||
        (samples != 0 && profile.multisample_enabled == 0)) {
        softgl_destroy(context);
        return NULL;
    }
    return context;
}
