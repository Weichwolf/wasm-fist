#include "render/renderer.h"
#include "assets/terrain.h"
#include "render/hud.h"
#include "render/terrain_scene.h"

#include <GL/softgl.h>
#include <stdint.h>
#include <stdlib.h>

struct fist_renderer {
    softgl_ctx *context;
};

fist_renderer *fist_renderer_create(int width, int height) {
    if (width <= 0 || height <= 0) {
        return NULL;
    }
    fist_renderer *renderer = calloc(1, sizeof(*renderer));
    if (renderer == NULL) {
        return NULL;
    }
    renderer->context = softgl_create(width, height);
    if (renderer->context == NULL) {
        free(renderer);
        return NULL;
    }
    return renderer;
}

void fist_renderer_destroy(fist_renderer *renderer) {
    if (renderer != NULL) {
        softgl_destroy(renderer->context);
        free(renderer);
    }
}

const uint8_t *fist_renderer_pixels(fist_renderer *renderer) {
    return softgl_read_rgba8(renderer->context);
}

int fist_renderer_draw_hud(fist_renderer *renderer, const fist_hud *hud) {
    if (renderer == NULL) {
        return -1;
    }
    softgl_make_current(renderer->context);
    return fist_draw_hud(hud);
}

int fist_renderer_draw_probe(fist_renderer *renderer) {
    /* Coordinates and colors are an integration fixture, not recovered game data. */
    static const GLfloat background[] = {0.0F, 0.0F, 0.0F, 1.0F};
    static const GLfloat left[] = {-0.75F, -0.75F};
    static const GLfloat right[] = {0.75F, -0.75F};
    static const GLfloat top[] = {0.0F, 0.75F};
    static const GLfloat red[] = {1.0F, 0.0F, 0.0F};
    static const GLfloat green[] = {0.0F, 1.0F, 0.0F};
    static const GLfloat blue[] = {0.0F, 0.0F, 1.0F};

    softgl_make_current(renderer->context);
    glClearColor(background[0], background[1], background[2], background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    glColor3fv(red);
    glVertex2fv(left);
    glColor3fv(green);
    glVertex2fv(right);
    glColor3fv(blue);
    glVertex2fv(top);
    glEnd();
    return glGetError() == GL_NO_ERROR ? 0 : -1;
}

int fist_renderer_draw_terrain(fist_renderer *renderer, const fist_terrain *terrain,
                               const fist_terrain_view *view) {
    if (renderer == NULL) {
        return -1;
    }
    softgl_make_current(renderer->context);
    return fist_draw_terrain_scene(renderer, terrain, view, NULL);
}

int fist_renderer_draw_vehicle(fist_renderer *renderer, const fist_terrain *terrain,
                               const fist_terrain_view *view, const fist_scene_vehicle *vehicle) {
    if (renderer == NULL || vehicle == NULL || vehicle->bitmap.indices == NULL ||
        vehicle->bitmap.width == 0 || vehicle->bitmap.height == 0 || vehicle->texel_width <= 0 ||
        vehicle->texel_height <= 0) {
        return -1;
    }
    softgl_make_current(renderer->context);
    return fist_draw_terrain_scene(renderer, terrain, view, vehicle);
}
