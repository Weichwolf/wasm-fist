#ifndef FIST_RENDER_SOFTGL_PROFILE_H
#define FIST_RENDER_SOFTGL_PROFILE_H

#include "render/renderer.h"

#include <GL/softgl.h>

/* Sole owned adapter to the pinned dependency's private worker configuration.
 * No library sources are modified. Contexts remain idle until verified. */
softgl_ctx *fist_softgl_create_profile(int width, int height, int samples);
int fist_softgl_get_profile(softgl_ctx *context, fist_render_profile *out);

#endif
